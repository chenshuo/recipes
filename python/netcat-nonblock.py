#!/usr/bin/python

import errno
import fcntl
import os
import select
import socket
import sys

def setNonBlocking(fd):
    flags = fcntl.fcntl(fd, fcntl.F_GETFL)
    fcntl.fcntl(fd, fcntl.F_SETFL, flags | os.O_NONBLOCK)


def nonBlockingWrite(fd, data):
    try:
        nw = os.write(fd, data)
        return nw
    except OSError as e:
        if e.errno == errno.EWOULDBLOCK:
            return -1


def relay(sock):
    socketEvents = select.POLLIN
    poll = select.poll()
    poll.register(sock, socketEvents)
    poll.register(sys.stdin, select.POLLIN)

    setNonBlocking(sock)
    # setNonBlocking(sys.stdin)
    # setNonBlocking(sys.stdout)

    done = False
    socketOutputBuffer = ''
    while not done:
        events = poll.poll(10000)  # 10 seconds
        for fileno, event in events:
            if event & select.POLLIN:
                if fileno == sock.fileno():
                    data = sock.recv(8192)
                    if data:
                        nw = sys.stdout.write(data)  # stdout does support non-blocking write, though
                    else:
                        done = True
                else:
                    assert fileno == sys.stdin.fileno()
                    data = os.read(fileno, 8192)
                    if data:
                        assert len(socketOutputBuffer) == 0
                        nw = nonBlockingWrite(sock.fileno(), data)
                        if nw < len(data):
                            if nw < 0:
                                nw = 0
                            socketOutputBuffer = data[nw:]
                            socketEvents |= select.POLLOUT
                            poll.register(sock, socketEvents)
                            poll.unregister(sys.stdin)
                    else:
                        sock.shutdown(socket.SHUT_WR)
                        poll.unregister(sys.stdin)
            if event & select.POLLOUT:
                if fileno == sock.fileno():
                    assert len(socketOutputBuffer) > 0
                    nw = nonBlockingWrite(sock.fileno(), socketOutputBuffer)
                    if nw < len(socketOutputBuffer):
                        assert nw > 0
                        socketOutputBuffer = socketOutputBuffer[nw:]
                    else:
                        socketOutputBuffer = ''
                        socketEvents &= ~select.POLLOUT
                        poll.register(sock, socketEvents)
                        poll.register(sys.stdin, select.POLLIN)



def main(argv):
    if len(argv) < 3:
        binary = argv[0]
        print "Usage:\n  %s -l port\n  %s host port" % (argv[0], argv[0])
        print (sys.stdout.write)
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
