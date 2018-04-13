#!/usr/bin/env python3

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import socket, struct, sys, time

"""
struct SessionMessage
{
  be32 number;
  be32 length;
} __attribute__ ((__packed__));
"""

SessionMessage = struct.Struct(">ii")

"""
struct PayloadMessage
{
  be32 length;
  char data[0];
};
"""
LengthMessage = struct.Struct(">i")


def transmit(sock, length, number):
    sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    print("Length of buffer : %5d\nNumber of buffers: %5d" % (length, number))
    total_mb = length * number / 1024.0 / 1024
    print("Total MiB        : %7.1f " % total_mb)
    header = LengthMessage.pack(length)
    # bytearray works for both Python 2 and 3
    payload = bytearray((b"0123456789ABCDEF"[i % 16] for i in range(length)))
    assert len(payload) == length
    start = time.time()
    sock.sendall(SessionMessage.pack(number, length))
    for x in range(number):
        sock.sendall(header)
        sock.sendall(payload)
        ack = sock.recv(LengthMessage.size, socket.MSG_WAITALL)
        ack_length, = LengthMessage.unpack(ack)
        assert ack_length == length
    duration = time.time() - start
    print("%.3f seconds, %.3f MiB/s" % (duration, total_mb / duration)) 
    print("%.3f microseconds per message" % (duration * 1e6 / number))


def receive(sock):
    number, length = SessionMessage.unpack(sock.recv(SessionMessage.size))
    print("Length of buffer : %5d\nNumber of buffers: %5d" % (length, number))
    total_mb = length * number / 1024.0 / 1024
    print("Total MiB        : %7.1f " % total_mb)
    ack = LengthMessage.pack(length)
    start = time.time()
    for x in range(number):
        header = sock.recv(LengthMessage.size, socket.MSG_WAITALL)
        header_length, = LengthMessage.unpack(header)
        assert header_length == length
        payload = sock.recv(length, socket.MSG_WAITALL)
        assert len(payload) == length
        sock.sendall(ack)
    duration = time.time() - start
    print("%.3f seconds, %.3f MiB/s" % (duration, total_mb / duration)) 
    print("%.3f microseconds per message" % (duration * 1e6 / number))


def main(argv):
    if len(argv) < 2:
        print("Transmit: ttcp -t host\nReceive : ttcp -r")
        return
    port = 5001
    print("SessionMessage.size = %s" % SessionMessage.size)
    if argv[1] == "-t":
        # client
        host = argv[2]
        # sock = socket.create_connection((host, port))
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((host, port))
        transmit(sock, length=65536, number=8192)
    else:
        # server
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(('', port))
        server_socket.listen(5)
        (client_socket, client_address) = server_socket.accept()
        print("Got client from %s:%s" % client_address)
        server_socket.close()
        receive(client_socket)


if __name__ == '__main__':
    main(sys.argv)
