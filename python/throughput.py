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
Python:
Receiver transferred 1073.742MB in 4.497s, throughput 238.789MB/s 1910.311Mbits/s
Transferred 2529.821MB in 10.006s, throughput 252.818MB/s 2022.543Mbits/s, 38602 syscalls 65536.0 Bytes/syscall

tcpperf:
Transferred 3171.746MB 3024.812MiB in 10.006s, 48397 syscalls, 65536.0 Bytes/syscall
 7.000  317.09MB/s  2536.8Mbits/s  sndbuf=135.9K snd_cwnd=481.9K ssthresh=1048560.0K snd_wnd=3.2K rtt=281/437

iperf3:
[  5]   0.00-10.00  sec  2.32 GBytes  2.00 Gbits/sec                  receiver


Raspberry Pi 4 running Raspbian GNU/Linux 10, kernel 5.10
Receiver transferred 1073.742MB in 1.783s, throughput 602.052MB/s 4816.417Mbits/s

Raspberry Pi 4 running Ubuntu server 21.04 arm64, kernel 5.11
Receiver transferred 1073.742MB in 1.540s, throughput 697.363MB/s 5578.907Mbits/s

Raspberry Pi 4 running Debian 11 arm64, kernel 5.10
Python:
Receiver transferred 1073.742MB in 1.654s, throughput 649.204MB/s 5193.631Mbits/s
Transferred 8186.823MB in 10.000s, throughput 818.647MB/s 6549.173Mbits/s, 125013 syscalls 65487.8 Bytes/syscall

tcpperf:
Transferred 10737.418MB 10240.000MiB in 7.804s, 163840 syscalls, 65536.0 Bytes/syscall
 7.804  1375.92MB/s  11007.4Mbits/s  sndbuf=2565.0K snd_cwnd=639.5K ssthresh=2097152.0K snd_wnd=3039.5K rtt=60/8

iperf3:
[  5]   0.00-10.00  sec  13.4 GBytes  11.5 Gbits/sec    0             sender


Raspberry Pi 3 running Raspbian GNU/Linux 10, kernel 5.10
Receiver transferred 1073.742MB in 1.938s, throughput 554.156MB/s 4433.249Mbits/s

Raspberry Pi 2 running Raspbian GNU/Linux 10, kernel 5.10
Receiver transferred 1073.742MB in 3.696s, throughput 290.500MB/s 2323.996Mbits/s

==============================================================================
HP e8300, CPU i7-3770

linux$ iperf3 -c localhost -V
iperf 3.9
Linux flute 5.10.0-6-amd64 #1 SMP Debian 5.10.28-1 (2021-04-09) x86_64
Control connection MSS 32768
Time: Sun, 23 May 2021 02:58:38 GMT
Connecting to host localhost, port 5201
      TCP MSS: 32768 (default)
[  5] local ::1 port 37764 connected to ::1 port 5201
Starting Test: protocol: TCP, 1 streams, 131072 byte blocks, omitting 0 seconds, 10 second test, tos 0
[ ID] Interval           Transfer     Bitrate         Retr  Cwnd
[  5]   0.00-1.00   sec  5.17 GBytes  44.4 Gbits/sec    0    959 KBytes
[  5]   1.00-2.00   sec  5.22 GBytes  44.9 Gbits/sec    0   1.25 MBytes
[  5]   2.00-3.00   sec  5.21 GBytes  44.8 Gbits/sec    0   1.25 MBytes
[  5]   3.00-4.00   sec  5.25 GBytes  45.1 Gbits/sec    0   1.25 MBytes
[  5]   4.00-5.00   sec  5.25 GBytes  45.1 Gbits/sec    0   1.25 MBytes
[  5]   5.00-6.00   sec  5.25 GBytes  45.1 Gbits/sec    0   1.25 MBytes
[  5]   6.00-7.00   sec  5.25 GBytes  45.1 Gbits/sec    0   1.25 MBytes
[  5]   7.00-8.00   sec  5.26 GBytes  45.2 Gbits/sec    0   1.25 MBytes
[  5]   8.00-9.00   sec  5.25 GBytes  45.1 Gbits/sec    0   1.25 MBytes
[  5]   9.00-10.00  sec  5.25 GBytes  45.1 Gbits/sec    0   1.25 MBytes
- - - - - - - - - - - - - - - - - - - - - - - - -
Test Complete. Summary Results:
[ ID] Interval           Transfer     Bitrate         Retr
[  5]   0.00-10.00  sec  52.4 GBytes  45.0 Gbits/sec    0             sender
[  5]   0.00-10.00  sec  52.4 GBytes  45.0 Gbits/sec                  receiver
CPU Utilization: local/sender 100.0% (1.5%u/98.5%s), remote/receiver 69.0% (5.0%u/64.0%s)
snd_tcp_congestion cubic
rcv_tcp_congestion cubic

iperf Done.

throughput-bidi.py:
Transferred 47299.953MB in 10.000s, throughput 4729.937MB/s 37839.492Mbits/s, 721740 syscalls 65536.0 Bytes/syscall

tpc/tcpperf:
Transferred 53370.749MB 50898.312MiB in 10.000s, 814373 syscalls, 65536.0 Bytes/syscall
10.000  5337.01MB/s  42696.1Mbits/s  sndbuf=2565.0K snd_cwnd=639.5K ssthresh=2097152.0K snd_wnd=3039.5K rtt=19/8


freebsd% % iperf3 -c localhost -V
iperf 3.9
FreeBSD freebsd 13.0-RELEASE FreeBSD 13.0-RELEASE #0 releng/13.0-n244733-ea31abc261f: Fri Apr  9 04:24:09 UTC 2021     root@releng1.nyi.freebsd.org:/usr/obj/usr/src/amd64.amd64/sys/GENERIC amd64
Control connection MSS 16344
Time: Sun, 23 May 2021 03:39:04 UTC
Connecting to host 172.16.0.77, port 5201
      TCP MSS: 16344 (default)
[  5] local 172.16.0.77 port 47020 connected to 172.16.0.77 port 5201
Starting Test: protocol: TCP, 1 streams, 131072 byte blocks, omitting 0 seconds, 10 second test, tos 0
[ ID] Interval           Transfer     Bitrate         Retr  Cwnd
[  5]   0.00-1.00   sec  7.99 GBytes  68.7 Gbits/sec    0   1021 KBytes
[  5]   1.00-2.00   sec  8.30 GBytes  71.3 Gbits/sec    0   1021 KBytes
[  5]   2.00-3.00   sec  8.29 GBytes  71.2 Gbits/sec    0   1021 KBytes
[  5]   3.00-4.00   sec  8.36 GBytes  71.8 Gbits/sec    0   1.21 MBytes
[  5]   4.00-5.00   sec  8.37 GBytes  71.9 Gbits/sec    0   1.21 MBytes
[  5]   5.00-6.00   sec  8.41 GBytes  72.2 Gbits/sec    0   1.21 MBytes
[  5]   6.00-7.00   sec  8.38 GBytes  72.0 Gbits/sec    0   1.21 MBytes
[  5]   7.00-8.00   sec  8.39 GBytes  72.0 Gbits/sec    0   1.21 MBytes
[  5]   8.00-9.00   sec  8.33 GBytes  71.5 Gbits/sec    0   1.25 MBytes
[  5]   9.00-10.00  sec  8.33 GBytes  71.6 Gbits/sec    0   1.25 MBytes
- - - - - - - - - - - - - - - - - - - - - - - - -
Test Complete. Summary Results:
[ ID] Interval           Transfer     Bitrate         Retr
[  5]   0.00-10.00  sec  83.2 GBytes  71.4 Gbits/sec    0             sender
[  5]   0.00-10.00  sec  83.2 GBytes  71.4 Gbits/sec                  receiver
CPU Utilization: local/sender 98.3% (2.7%u/95.7%s), remote/receiver 78.2% (1.8%u/76.3%s)
snd_tcp_congestion newreno
rcv_tcp_congestion newreno

iperf Done.

throughput-bidi.py:
Transferred 65850.835MB in 10.000s, throughput 6584.994MB/s 52679.952Mbits/s, 1004804 syscalls 65536.0 Bytes/syscall

tpc/tcpperf:
Transferred 83659.260MB 79783.688MiB in 10.000s, 1276539 syscalls, 65536.0 Bytes/syscall
10.000  8456.80MB/s  67654.4Mbits/s  sndbuf=647.9K snd_cwnd=1387.7K ssthresh=1048560.0K snd_wnd=667.6K rtt=218/187

"""
