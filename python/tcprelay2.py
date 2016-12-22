#!/usr/bin/python

import socket, thread, time

def forward(source, destination):
    source_addr = source.getpeername()
    while True:  # FIXME: error handling
        data = source.recv(4096)  # Connection reset by peer
        if data:
            destination.sendall(data)  # Broken pipe
        else:
            print 'disconnect', source_addr
            destination.shutdown(socket.SHUT_WR)
            break

listensocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
listensocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
listensocket.bind(('', 2000))
listensocket.listen(5)

while True:
    (serversocket, address) = listensocket.accept()
    print 'accepted', address
    clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    clientsocket.connect(('localhost', 3000))
    print 'connected', clientsocket.getpeername()
    thread.start_new_thread(forward, (serversocket, clientsocket))
    thread.start_new_thread(forward, (clientsocket, serversocket))
