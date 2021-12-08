#leadpipe server

import random
import subprocess as sp

import sys
import json
import asyncio
import logging
import websockets


async def sender(blocka, blockb, blockc, blockd):
    await asyncio.sleep(0.1)

async def receive_ws_implant(queue, websocket, path):
    try:
        splitted = path.split('/')
        splitted.pop(0)
        client_id = splitted.pop(0)
        print('Client {} connected'.format(client_id))
        clients[client_id] = websocket
        while True:
            data = await websocket.recv()
            print('Client {} << {}'.format(client_id, data))
            message = json.loads(data)
            destination_id = message['id']
            destination_websocket = clients.get(destination_id)
            if destination_websocket:
                message['id'] = client_id
                data = json.dumps(message)
                await queue.put(data)

                #print('Client {} >> {}'.format(destination_id, data))
                #await destination_websocket.send(data)
            else:
                print('Client {} not found'.format(destination_id))
    except Exception as e:
        print(e)

def connected_boolean(address):
    return(True)

async def consumption(queue_inbound_client, queue_inbound_implant):
            print("<- CONSUMING ->")
            if(connected_boolean(None)):
                #wait for a new inbound message
                item_client = await queue_inbound_client.get()
                item_implant = await queue_inbound_implant.get()
                if item_client is None:
                    pass
                else:
                    print('consuming item {}...'.format(item_client))
                    #here is where the logic goes (what action does the item need)
                    #obvisouly in the future
                    #await send_implant(msg)
                    #await asyncio.sleep(1.0)
                if item_implant is None:
                    pass
                else:
                    print('consuming item {}...'.format(item_implant))
            else:
                inv = False
                print("<-BRIDGE DISCONNECT->")






endpoint_or_port = sys.argv[1] if len(sys.argv) > 1 else "8000"
ssl_cert = sys.argv[2] if len(sys.argv) > 2 else None

endpoint = endpoint_or_port if ':' in endpoint_or_port else "10.0.0.143:" + endpoint_or_port
if ssl_cert:
    ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    ssl_context.load_cert_chain(ssl_cert)
else:
    ssl_context = None

loop = asyncio.get_event_loop()
#inbound queues from client and implant (messages received)
queue_inbound_client = asyncio.Queue(loop=loop)
queue_inbound_implant = asyncio.Queue(loop=loop)

#outbound queues from client and implant (messages to be sent)
queue_outbound_client = asyncio.Queue(loop=loop)
queue_outbound_implant = asyncio.Queue(loop=loop)

host, port = endpoint.rsplit(':', 1)
receive_implant = websockets.serve(receive_ws_implant(queue_inbound_implant,host, int(port)), host, int(port), ssl=ssl_context)

#async run receive from client and implant
#asyncio.ensure_future(receive_client(queue_inbound_client), loop=loop)
#start_server = websockets.serve(receive_implant, host, int(port), ssl=ssl_context)

asyncio.ensure_future(receive_implant, loop=loop)

invariant = False

while(invariant!=True):
    try:
        #run consumption
        loop.run_until_complete(consumption(queue_inbound_client, queue_inbound_implant))
        invariant = True
    except Exception as e:
        print("<- FAILED->")
        print(e)





loop.close()
