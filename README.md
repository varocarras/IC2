# IC2 - P2P RTC Penetration Testing Framework

## Introduction
 - This framework provides the ability to create a unique payload that communicates to the client directly through RTC.
 - The client is a website that runs on localhost, once connected to it, it will start communicating with implants.
 - Since the communication protocol is RTC, the Client/C2 can be deployed on any public network without the need to port forward.
 - While the Client/C2 is down, the Implants will communicate with each other to exchange their data and get their assigned tasks.

## Functionality
 - Once executed, the payload will attempt to create persistence on the machine and setup a working directory, it then will then initiate the connection to the client.
- The Implants life cycle is described below:
![lifecycle](https://github.com/varocarras/IC2/blob/main/Implant/Implant%20Cycle.png)

 - The implant includes the following functionality:
    | System Info
    | Command shell
    | Steal credentials
    | Steal data
    | Drop .exe (base64 strings)
    | Display video
    | Popup message

- The Client/C2 includes the following functionality:
    | Friendly GUI 
    | Individual implant control panel
    | Host scripts in base64 string

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
 - Alvaro : Worked mostly on implant
 - Aidan : Worked mostly on C2/client

