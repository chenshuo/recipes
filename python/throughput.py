#!/usr/bin/python3

# A simple program to test TCP throughput, by sending 1GiB data.

import socket, sys, time

def report(name : str, total_bytes : int, elapsed_seconds : float):
    mbps = total_bytes / 1e6 / elapsed_seconds
    print('%s transferred %.3fMB in %.3fs, throughput %.3fMB/s %.3fMbits/s' %
            (name, total_bytes / 1e6, elapsed_seconds, mbps, mbps * 8))


def run_server(port : int):
    server_socket = socket.create_server(('', port))  # Requires Python 3.8
    sock, client_addr = server_socket.accept()
    print('Got client from %s:%d' % client_addr)
    start = time.time()
    total = 0
    while True:
        data = sock.recv(65536)
        if not data:
            break
        total += len(data)
    report('Receiver', total, time.time() - start)


# This client has flaws.
def run_client(server : str, port : int):
    sock = socket.create_connection((server, port))
    buf = b'X' * 65536
    n = 16384
    start = time.time()
    for i in range(n):
        sock.sendall(buf)
    total = len(buf) * n
    report('Sender', total, time.time() - start)


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: {0} -s or {0} server'.format(sys.argv[0]))
    elif sys.argv[1] == '-s':
        run_server(port=2009)
    else:
        run_client(sys.argv[1], port=2009)

self_result="""
Raspberry Pi 4 running FreeBSD 13-RELEASE:
Receiver transferred 1073.742MB in 4.497s, throughput 238.789MB/s 1910.311Mbits/s

Raspberry Pi 4 running Raspbian GNU/Linux 10, kernel 5.10
Receiver transferred 1073.742MB in 1.783s, throughput 602.052MB/s 4816.417Mbits/s

Raspberry Pi 4 running Ubuntu server 21.04 arm64, kernel 5.11
Receiver transferred 1073.742MB in 1.540s, throughput 697.363MB/s 5578.907Mbits/s

Raspberry Pi 3 running Raspbian GNU/Linux 10, kernel 5.10
Receiver transferred 1073.742MB in 1.938s, throughput 554.156MB/s 4433.249Mbits/s

Raspberry Pi 2 running Raspbian GNU/Linux 10, kernel 5.10
Receiver transferred 1073.742MB in 3.696s, throughput 290.500MB/s 2323.996Mbits/s
"""
