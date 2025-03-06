# IC2 - P2P RTC Penetration Testing Framework

⚠️ Disclaimer: For Research & Educational Use Only ⚠️

IC2 is a powerful penetration testing framework designed for security research and adversary simulation. This tool includes functionality capable of bypassing Windows Defender mechanisms, executing payloads, and exfiltrating sensitive data. Due to its offensive capabilities, IC2 should only be deployed in controlled, isolated, and virtualized environments. Unauthorized or malicious use of this framework may violate legal and ethical guidelines. Users are responsible for ensuring compliance with applicable laws and obtaining proper authorization before executing tests. The developers assume no liability for misuse.

## Introduction
 - This penetration testing framework was developed to help launch an attack, both anonymously and efficiently, without the need of deploying a permanent C2.
 - This framework provides the ability to create a unique payload that communicates to the client directly through RTC.
 - When the client is out-of-reach, the P2P Network allows the Implant to communicate with others to self-update and share un-transferred useful data.
 - The client is a website that runs on localhost, once connected to it, it will start communicating with implants.
 - Since the communication protocol is RTC, the Client can be deployed on any public network without the need to port forward, making it ideal to run the Client on public networks.

## Functionality
 - Once executed, the payload will attempt to create persistence on the machine and setup a working directory, it then will then attempt to initiate the connection to the client.
- The Implants life cycle is described below:
![lifecycle](https://github.com/varocarras/IC2/blob/main/Implant/Implant%20Cycle.png)

 - The implant includes the following functionality:
    - System Info
    - Command shell
    - Steal credentials
    - Steal data
    - Drop .exe (base64 strings)
    - Display video
    - Popup message

- The Client/C2 includes the following functionality:
    - Friendly GUI 
    - Individual implant control panel
    - Host scripts in base64 string

## User Interface
### Current
![current](https://github.com/varocarras/IC2/blob/main/C2-Client/Current%20GUI.png)
### Upcoming
![upcoming](https://github.com/varocarras/IC2/blob/main/C2-Client/New%20GUI.png)
## Sources
 - Chromium-Stealer - [SaulBerrenson/BrowserStealer](https://github.com/SaulBerrenson/BrowserStealer?ref=bestofcpp.com)

## Presentation
 - You can find the presentation [here](https://docs.google.com/presentation/d/172e0TQYKG9e2lxGtp5YXmFnBki4YkVWDc0HZuaKSB7s/edit#slide=id.p)
## Demo video
 - Demo Malware.mp4
## Diagrams
![first](https://user-images.githubusercontent.com/44831139/145478465-a4a7976a-c445-465e-b2aa-34952987cb03.jpg)
![second](https://user-images.githubusercontent.com/44831139/145478485-77f89ad6-b8a0-4456-93b4-dd6f7525e5ff.jpg)
## Collaboration
 - Alvaro: Led the development of the implant, core functionality, and system integrations.
 - Aidan: Contributed to theoretical research, documentation, and overall project structuring.

