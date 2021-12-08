
clients = {}

async def handle_websocket(websocket, path):
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
                #print('Client {} >> {}'.format(destination_id, data))
            else:
                #print('Client {} not found'.format(destination_id))
                print("<- REQUESTED CLIENT NOT FOUND ->")
    except Exception as e:
        print(e)
    finally:
        if client_id:
            del clients[client_id]
            print('<- CLIENT {} DISCONNECTED ->'.format(client_id))

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
    start_server = websockets.serve(handle_websocket, host, int(port), ssl=ssl_context)
    asyncio.get_event_loop().run_until_complete(start_server)
    asyncio.get_event_loop().run_forever()
