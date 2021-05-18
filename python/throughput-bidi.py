#!/usr/bin/python3

# A simple program to test TCP throughput, by sending data for 10 seconds.

import socket, sys, time

def report(total_bytes : int, elapsed_seconds : float, syscalls : int):
    mbps = total_bytes / 1e6 / elapsed_seconds
    print('Transferred %.3fMB in %.3fs, throughput %.3fMB/s %.3fMbits/s, %d syscalls %.1f Bytes/syscall' %
            (total_bytes / 1e6, elapsed_seconds, mbps, mbps * 8, syscalls, total_bytes / syscalls))


def print_buf(sock):
    rcvbuf = sock.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF)
    sndbuf = sock.getsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF)
    print('rcvbuf=%.1fK sndbuf=%.1fK' % (rcvbuf / 1024.0, sndbuf / 1024.0))


def format_address(sock):
    def format(addr):
        return '%s:%d' % addr

    return (format(sock.getsockname()), format(sock.getpeername()))


def run_sender(sock):
    buf = b'X' * 65536
    print('Sending... %s -> %s' % format_address(sock))
    print_buf(sock)
    start = time.time()
    total = 0
    count = 0
    while True:
        total += sock.send(buf)
        count += 1
        if time.time() - start > 10:
            break
    print_buf(sock)
    sent = time.time()
    sock.shutdown(socket.SHUT_WR)
    sock.recv(4096)
    print('waited %.1fms' % ((time.time() - sent) * 1e3))
    report(total, time.time() - start, count)


def run_receiver(sock):
    print('Receiving... %s <- %s' % format_address(sock))
    print_buf(sock)
    start = time.time()
    total = 0
    count = 0
    while True:
        data = sock.recv(65536)
        if not data:
            break
        total += len(data)
        count += 1
    print_buf(sock)
    sock.close()
    report(total, time.time() - start, count)


if __name__ == '__main__':
    port = 2009
    if len(sys.argv) < 2:
        print('Usage: {0} -s or {0} server'.format(sys.argv[0]))
        print('Append -b to send traffic backwards.')
    elif sys.argv[1] == '-s':
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(('', port))
        server_socket.listen(5)
        while True:
            sock, client_addr = server_socket.accept()
            if len(sys.argv) > 2 and sys.argv[2] == '-b':
                run_sender(sock)
            else:
                run_receiver(sock)
    else:
        sock = socket.create_connection((sys.argv[1], port))
        if len(sys.argv) > 2 and sys.argv[2] == '-b':
            run_receiver(sock)
        else:
            run_sender(sock)
