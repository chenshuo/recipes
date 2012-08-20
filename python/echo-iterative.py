#!/usr/bin/python

import socket

serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serversocket.bind(("0.0.0.0", 2007))
serversocket.listen(5)

while True:
    (clientsocket, address) = serversocket.accept()
    print "got connection from", address
    while True:
        data = clientsocket.recv(4096)
        if data:
            sent = clientsocket.send(data)    # sendall?
        else:
            print "disconnect", address
            clientsocket.close()
            break
