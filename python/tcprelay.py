#!/usr/bin/python

import socket, thread, time

listen_port = 3007
connect_addr = ('localhost', 2007)
sleep_per_byte = 0.0001

def forward(source, destination):
    source_addr = source.getpeername()
    while True:
        data = source.recv(4096)
        if data:
            for i in data:
                destination.sendall(i)
                time.sleep(sleep_per_byte)
        else:
            print 'disconnect', source_addr
            destination.shutdown(socket.SHUT_WR)
            break

serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
serversocket.bind(('', listen_port))
serversocket.listen(5)

while True:
    (clientsocket, address) = serversocket.accept()
    print 'accepted', address
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(connect_addr)
    print 'connected', sock.getpeername()
    thread.start_new_thread(forward, (clientsocket, sock))
    thread.start_new_thread(forward, (sock, clientsocket))
