#!/usr/bin/python3

# A simple program to test TCP throughput, by sending 1GiB data.

import socket, sys, time

def report(total_bytes : int, elapsed_seconds : float):
    mbps = total_bytes / 1e6 / elapsed_seconds
    print('Transferred %.3fMB in %.3fs, throughput %.3fMB/s %.3fMbits/s' %
            (total_bytes / 1e6, elapsed_seconds, mbps, mbps * 8))


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
    report(total, time.time() - start)


# This client has flaws.
def run_client(server : str, port : int):
    sock = socket.create_connection((server, port))
    buf = b'X' * 65536
    n = 16384
    start = time.time()
    for i in range(n):
        sock.sendall(buf)
    total = len(buf) * n
    report(total, time.time() - start)


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: {0} -s or {0} server'.format(sys.argv[0]))
    elif sys.argv[1] == '-s':
        run_server(port=2009)
    else:
        run_client(sys.argv[1], port=2009)
