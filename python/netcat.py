#!/usr/bin/python

import os
import select
import socket
import sys

def relay(sock):
    poll = select.poll()
    poll.register(sock, select.POLLIN)
    poll.register(sys.stdin, select.POLLIN)

    done = False
    while not done:
        events = poll.poll(10000)  # 10 seconds
        for fileno, event in events:
            if event & select.POLLIN:
                if fileno == sock.fileno():
                    data = sock.recv(8192)
                    if data:
                        sys.stdout.write(data)
                    else:
                        done = True
                else:
                    assert fileno == sys.stdin.fileno()
                    data = os.read(fileno, 8192)
                    if data:
                        sock.sendall(data)
                    else:
                        sock.shutdown(socket.SHUT_WR)
                        poll.unregister(sys.stdin)


def main(argv):
    if len(argv) < 3:
        binary = argv[0]
        print "Usage:\n  %s -l port\n  %s host port" % (argv[0], argv[0])
        return
    port = int(argv[2])
    if argv[1] == "-l":
        # server
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(('', port))
        server_socket.listen(5)
        (client_socket, client_address) = server_socket.accept()
        server_socket.close()
        relay(client_socket)
    else:
        # client
        sock = socket.create_connection((argv[1], port))
        relay(sock)


if __name__ == "__main__":
    main(sys.argv)
