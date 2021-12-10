#fresh.py


#hueprocess2.py
import asyncio
import random
from bleak import BleakClient
from bleak import BleakScanner
import zmq
from zmq.asyncio import Context, Poller
import subprocess as sp
from sh import bluetoothctl
import sys
def convert_rgb(rgb):
    scale = 0xFF
    adjusted = [max(1, chan) for chan in rgb]
    total = sum(adjusted)
    adjusted = [int(round(chan / total * scale)) for chan in adjusted]

    # Unknown, Red, Blue, Green
    return bytearray([0x1, adjusted[0], adjusted[2], adjusted[1]])

async def produce(queue, n):
    ctx = Context.instance()
    print("<PRODUCTION RUNNING>")
    url = 'tcp://127.0.0.1:5555'
    pull = ctx.socket(zmq.PULL)
    pull.connect(url)
    poller = Poller()
    poller.register(pull, zmq.POLLIN)
    print("<LISTENING>")
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
    stdoutdata = sp.getoutput("hcitool con")
    if address in stdoutdata.split():
        return(True)
    else:
        return(False)

async def consume(queue, address):
    print("<---LINKING--->")
    commands = []
    address = address
    LIGHT_CHARACTERISTIC = "932c32bd-0002-47a2-835a-a8d455b859dd"
    BRIGHTNESS_CHARACTERISTIC = "932c32bd-0003-47a2-835a-a8d455b859dd"
    TEMPERATURE_CHARACTERISTIC = "932c32bd-0004-47a2-835a-a8d455b859dd"
    COLOR_CHARACTERISTIC = "932c32bd-0005-47a2-835a-a8d455b859dd"
    async with BleakClient(address) as client:
        print(f"Connected: {client.is_connected}")
        paired = await client.pair(protection_level=2)
        print("Paired")
        inv = True
        while(inv):
            if(connected_boolean(address)):
                #print(item)
                item = await queue.get()
                if item is None:
                    # the producer emits None to indicate that it is done
                    pass
                else:
                    if(item[0] == 0):
                        await client.write_gatt_char(LIGHT_CHARACTERISTIC, b"\x00")
                        await asyncio.sleep(1.0)
                    elif(item[0] == 1):
                        await client.write_gatt_char(LIGHT_CHARACTERISTIC, b"\x01")
                        await asyncio.sleep(1.0)
                    elif(item[0] == 2):
                        await client.write_gatt_char(
                                    BRIGHTNESS_CHARACTERISTIC,
                                    bytearray(
                                        [
                                            item[1],
                                        ]
                                    ),
                                )
                    elif(item[0] == 3):
                        color = convert_rgb([item[1],item[2],item[3]])
                        await client.write_gatt_char(COLOR_CHARACTERISTIC, color)

                    #print('consuming item {}...'.format(item))

            else:
                inv = False
                print("<-LINK DISCONNECT->")



loop = asyncio.get_event_loop()
queue = asyncio.Queue(loop=loop)
asyncio.ensure_future(produce(queue,10), loop=loop)

invariant = False
while(invariant!=True):
    try:
        loop.run_until_complete(consume(queue, "D8:BD:5B:28:25:A6"))
        invariant = True
    except Exception as e:
        print("<-LINK FAILED->")
        print(e)
        if(e == "[org.bluez.Error.NotReady] Resource Not Ready"):
            bluetoothctl("power", "on")




loop.close()
