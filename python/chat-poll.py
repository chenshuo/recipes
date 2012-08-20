#!/usr/bin/python

import socket
import select

serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
serversocket.bind(('', 2007))
serversocket.listen(5)
# serversocket.setblocking(0)
poll = select.poll() # epoll() should work the same
poll.register(serversocket.fileno(), select.POLLIN)

connections = {}
while True:
    events = poll.poll(10000)  # 10 seconds
    for fileno, event in events:
        if fileno == serversocket.fileno():
            (clientsocket, address) = serversocket.accept()
            # clientsocket.setblocking(0)
            poll.register(clientsocket.fileno(), select.POLLIN)
            connections[clientsocket.fileno()] = clientsocket
        elif event & select.POLLIN:
            clientsocket = connections[fileno]
            data = clientsocket.recv(4096)
            if data:
                for (fd, othersocket) in connections.iteritems():
                    if othersocket != clientsocket:
                        othersocket.send(data) # sendall() partial?
            else:
                poll.unregister(fileno)
                clientsocket.close()
                del connections[fileno]
