#!/usr/bin/python

import socket
import sys

OPTS = [
    (socket.SOL_SOCKET, socket.SO_KEEPALIVE, 'SO_KEEPALIVE'),
    (socket.IPPROTO_TCP, socket.TCP_NODELAY, 'TCP_NODELAY'),
]

def print_sockopt(sock):
    for opt in OPTS:
        print "   ", opt[2], sock.getsockopt(opt[0], opt[1])

listen_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
listen_socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
listen_socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
listen_socket.listen(5)
listen_addr = listen_socket.getsockname()
print 'Listen on %s:%d' % listen_addr
print_sockopt(listen_socket)

port = listen_addr[1]
client_socket = socket.create_connection(('127.0.0.1', port))
print 'Client connected ', client_socket.getsockname(), client_socket.getpeername()
print_sockopt(client_socket)

(server_socket, client_address) = listen_socket.accept()
print 'Server accepted  ', server_socket.getsockname(), server_socket.getpeername()
print_sockopt(server_socket)

