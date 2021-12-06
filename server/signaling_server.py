#leadpipe server
import asyncio
import random
from bleak import BleakClient
from bleak import BleakScanner
import zmq
from zmq.asyncio import Context, Poller
import subprocess as sp
import sys

async def sender(blocka, blockb, blockc, blockd):
    ctx = Context.instance()
    print("<PUSHING ...>")
    url = 'tcp://127.0.0.1:5555'
    tic = time.time()
    push = ctx.socket(zmq.PUSH)
    push.bind(url)
    combined = str(blocka)+"x"+str(blockb)+"x"+str(blockc)+"x"+str(blockd)
    await push.send_multipart([str(combined).encode('ascii')])
    await asyncio.sleep(0.1)

async def receive_client(queue):
    ctx = Context.instance()
    print("<- CLIENT RECV RUNNING ->")
    url = 'tcp://127.0.0.1:5555'
    pull = ctx.socket(zmq.PULL)
    pull.connect(url)
    poller = Poller()
    poller.register(pull, zmq.POLLIN)
    print("<- LISTENING ->")
    while(True):
        events = await poller.poll()
        if pull in dict(events):
            #print("<INBOUND RECV>")
            msg = await pull.recv_multipart()
            msg = msg[0].decode('ascii')
            msg = msg.split('x')
            for x in range(0, len(msg)):
                msg[x] = int(msg[x])
            await queue.put(msg)


async def receive_implant(queue):
    ctx = Context.instance()
    print("<- IMPLANT RECV RUNNING ->")
    url = 'tcp://127.0.0.1:5556'
    pull = ctx.socket(zmq.PULL)
    pull.connect(url)
    poller = Poller()
    poller.register(pull, zmq.POLLIN)
    print("<- LISTENING ->")
    while(True):
        events = await poller.poll()
        if pull in dict(events):
            #print("<INBOUND RECV>")
            msg = await pull.recv_multipart()
            msg = msg[0].decode('ascii')
            msg = msg.split('x')
            for x in range(0, len(msg)):
                msg[x] = int(msg[x])
            await queue.put(msg)



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






loop = asyncio.get_event_loop()
#inbound queues from client and implant (messages received)
queue_inbound_client = asyncio.Queue(loop=loop)
queue_inbound_implant = asyncio.Queue(loop=loop)

#outbound queues from client and implant (messages to be sent)
queue_outbound_client = asyncio.Queue(loop=loop)
queue_outbound_implant = asyncio.Queue(loop=loop)

#async run receive from client and implant
asyncio.ensure_future(receive_client(queue_inbound_client), loop=loop)
asyncio.ensure_future(receive_implant(queue_inbound_implant), loop=loop)

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
