#!/usr/bin/python

import socket
import select

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.bind(('', 2007))
server_socket.listen(5)
# serversocket.setblocking(0)

poll = select.poll() # epoll() should work the same
connections = {}
handlers = {}

def handle_input(socket, data):
    for (fd, other_socket) in connections.iteritems():
        if other_socket != socket:
            other_socket.send(data) # sendall() partial?

def handle_request(fileno, event):
    if event & select.POLLIN:
        client_socket = connections[fileno]
        data = client_socket.recv(4096)
        if data:
            handle_input(client_socket, data)
        else:
            poll.unregister(fileno)
            client_socket.close()
            del connections[fileno]
            del handlers[fileno]

def handle_accept(fileno, event):
    (client_socket, client_address) = server_socket.accept()
    print "got connection from", client_address
    # client_socket.setblocking(0)
    poll.register(client_socket.fileno(), select.POLLIN)
    connections[client_socket.fileno()] = client_socket
    handlers[client_socket.fileno()] = handle_request

poll.register(server_socket.fileno(), select.POLLIN)
handlers[server_socket.fileno()] = handle_accept

while True:
    events = poll.poll(10000)  # 10 seconds
    for fileno, event in events:
        handler = handlers[fileno]
        handler(fileno, event)
