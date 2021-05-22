#!/usr/bin/python3

import re, subprocess

bs = 1
count = 1024 * 1024
while bs <= 1024 * 1024 * 8:
    args = ['dd', 'if=/dev/zero', 'of=/dev/null', 'bs=%d' % bs, 'count=%d' % count]
    result = subprocess.run(args, capture_output=True)
    seconds = 0
    message = str(result.stderr)
    if m := re.search('copied, (.*?) s, ', message):
        seconds = float(m.group(1))
    elif m := re.search('bytes transferred in (.*?) secs', message):
        seconds = float(m.group(1))
    else:
        print('Unable to parse dd output:\n%s' % message)
        break
    print('bs=%7d count=%7d %6.3fs %8.3fus/record %9.3fMB/s' %
            (bs, count, seconds, seconds * 1e6 / count, bs * count / 1e6 / seconds))

    bs *= 2
    if seconds > 1:
        count /= 2

result = """
Raspberry Pi 4 running FreeBSD 13-RELEASE:
freebsd% python3.9 bench_dd.py
bs=      1 count=1048576  3.307s    3.154us/record     0.317MB/s
bs=      2 count= 524288  1.682s    3.209us/record     0.623MB/s
bs=      4 count= 262144  0.824s    3.144us/record     1.272MB/s
bs=      8 count= 262144  0.855s    3.262us/record     2.453MB/s
bs=     16 count= 262144  0.831s    3.171us/record     5.046MB/s
bs=     32 count= 262144  0.813s    3.101us/record    10.321MB/s
bs=     64 count= 262144  0.848s    3.236us/record    19.779MB/s
bs=    128 count= 262144  0.848s    3.235us/record    39.569MB/s
bs=    256 count= 262144  0.863s    3.293us/record    77.746MB/s
bs=    512 count= 262144  0.844s    3.220us/record   159.029MB/s
bs=   1024 count= 262144  0.894s    3.411us/record   300.221MB/s
bs=   2048 count= 262144  0.984s    3.755us/record   545.461MB/s
bs=   4096 count= 262144  1.106s    4.219us/record   970.906MB/s
bs=   8192 count= 131072  0.675s    5.148us/record  1591.372MB/s
bs=  16384 count= 131072  0.917s    6.992us/record  2343.125MB/s
bs=  32768 count= 131072  1.385s   10.567us/record  3100.959MB/s
bs=  65536 count=  65536  1.189s   18.144us/record  3611.984MB/s
bs= 131072 count=  32768  1.130s   34.500us/record  3799.209MB/s
bs= 262144 count=  16384  1.155s   70.499us/record  3718.413MB/s
bs= 524288 count=   8192  1.264s  154.328us/record  3397.221MB/s
bs=1048576 count=   4096  1.543s  376.625us/record  2784.138MB/s
bs=2097152 count=   2048  2.041s  996.766us/record  2103.957MB/s
bs=4194304 count=   1024  2.441s 2383.790us/record  1759.511MB/s
bs=8388608 count=    512  2.690s 5253.455us/record  1596.779MB/s

Raspberry Pi 4 running Ubuntu server 21.04 arm64, kernel 5.11
$ ./bench_dd.py
bs=      1 count=1048576  5.409s    5.159us/record     0.194MB/s
bs=      2 count= 524288  2.828s    5.393us/record     0.371MB/s
bs=      4 count= 262144  1.415s    5.397us/record     0.741MB/s
bs=      8 count= 131072  0.682s    5.202us/record     1.538MB/s
bs=     16 count= 131072  0.719s    5.483us/record     2.918MB/s
bs=     32 count= 131072  0.674s    5.143us/record     6.222MB/s
bs=     64 count= 131072  0.704s    5.373us/record    11.911MB/s
bs=    128 count= 131072  0.711s    5.425us/record    23.593MB/s
bs=    256 count= 131072  0.690s    5.262us/record    48.655MB/s
bs=    512 count= 131072  0.714s    5.449us/record    93.955MB/s
bs=   1024 count= 131072  0.707s    5.392us/record   189.911MB/s
bs=   2048 count= 131072  0.751s    5.728us/record   357.517MB/s
bs=   4096 count= 131072  0.802s    6.116us/record   669.720MB/s
bs=   8192 count= 131072  1.038s    7.916us/record  1034.902MB/s
bs=  16384 count=  65536  0.833s   12.712us/record  1288.837MB/s
bs=  32768 count=  65536  1.325s   20.212us/record  1621.207MB/s
bs=  65536 count=  32768  1.282s   39.113us/record  1675.575MB/s
bs= 131072 count=  16384  1.211s   73.936us/record  1772.773MB/s
bs= 262144 count=   8192  1.185s  144.619us/record  1812.651MB/s
bs= 524288 count=   4096  1.091s  266.418us/record  1967.912MB/s
bs=1048576 count=   2048  1.372s  670.063us/record  1564.891MB/s
bs=2097152 count=   1024  1.543s 1507.129us/record  1391.488MB/s
bs=4194304 count=    512  1.650s 3223.105us/record  1301.324MB/s
bs=8388608 count=    256  1.583s 6185.391us/record  1356.197MB/s

Raspberry Pi 3 running Raspbian GNU/Linux 10, kernel 5.10
$ ./bench_dd.py
bs=      1 count=1048576  1.507s    1.437us/record     0.696MB/s
bs=      2 count= 524288  0.753s    1.437us/record     1.392MB/s
bs=      4 count= 524288  0.757s    1.444us/record     2.770MB/s
bs=      8 count= 524288  0.762s    1.454us/record     5.503MB/s
bs=     16 count= 524288  0.763s    1.456us/record    10.992MB/s
bs=     32 count= 524288  0.767s    1.463us/record    21.878MB/s
bs=     64 count= 524288  0.897s    1.711us/record    37.394MB/s
bs=    128 count= 524288  0.899s    1.715us/record    74.630MB/s
bs=    256 count= 524288  0.925s    1.764us/record   145.141MB/s
bs=    512 count= 524288  0.943s    1.799us/record   284.672MB/s
bs=   1024 count= 524288  1.013s    1.933us/record   529.725MB/s
bs=   2048 count= 262144  0.565s    2.155us/record   950.259MB/s
bs=   4096 count= 262144  0.671s    2.559us/record  1600.774MB/s
bs=   8192 count= 262144  0.996s    3.799us/record  2156.141MB/s
bs=  16384 count= 262144  1.627s    6.208us/record  2639.224MB/s
bs=  32768 count= 131072  1.456s   11.111us/record  2949.152MB/s
bs=  65536 count=  65536  1.365s   20.821us/record  3147.534MB/s
bs= 131072 count=  32768  1.324s   40.391us/record  3245.109MB/s
bs= 262144 count=  16384  1.301s   79.400us/record  3301.561MB/s
bs= 524288 count=   8192  1.369s  167.107us/record  3137.440MB/s
bs=1048576 count=   4096  1.862s  454.695us/record  2306.109MB/s
bs=2097152 count=   2048  2.197s 1072.520us/record  1955.351MB/s
bs=4194304 count=   1024  2.454s 2396.406us/record  1750.247MB/s
bs=8388608 count=    512  2.584s 5046.152us/record  1662.377MB/s

Raspberry Pi 2 running Raspbian GNU/Linux 10, kernel 5.10
$ ./bench_dd.py
bs=      1 count=1048576  2.294s    2.188us/record     0.457MB/s
bs=      2 count= 524288  1.155s    2.203us/record     0.908MB/s
bs=      4 count= 262144  0.573s    2.187us/record     1.829MB/s
bs=      8 count= 262144  0.581s    2.215us/record     3.611MB/s
bs=     16 count= 262144  0.579s    2.210us/record     7.239MB/s
bs=     32 count= 262144  0.582s    2.221us/record    14.405MB/s
bs=     64 count= 262144  0.767s    2.926us/record    21.874MB/s
bs=    128 count= 262144  0.725s    2.767us/record    46.261MB/s
bs=    256 count= 262144  0.794s    3.028us/record    84.557MB/s
bs=    512 count= 262144  0.773s    2.951us/record   173.523MB/s
bs=   1024 count= 262144  0.799s    3.050us/record   335.763MB/s
bs=   2048 count= 262144  1.093s    4.170us/record   491.168MB/s
bs=   4096 count= 131072  0.547s    4.170us/record   982.276MB/s
bs=   8192 count= 131072  1.039s    7.929us/record  1033.159MB/s
bs=  16384 count=  65536  0.771s   11.765us/record  1392.607MB/s
bs=  32768 count=  65536  1.511s   23.059us/record  1421.036MB/s
bs=  65536 count=  32768  2.009s   61.321us/record  1068.740MB/s
bs= 131072 count=  16384  1.858s  113.374us/record  1156.103MB/s
bs= 262144 count=   8192  2.055s  250.829us/record  1045.111MB/s
bs= 524288 count=   4096  2.036s  496.960us/record  1054.989MB/s
bs=1048576 count=   2048  2.070s 1010.869us/record  1037.301MB/s
bs=2097152 count=   1024  2.084s 2035.068us/record  1030.507MB/s
bs=4194304 count=    512  2.097s 4094.844us/record  1024.289MB/s
bs=8388608 count=    256  2.096s 8189.414us/record  1024.323MB/s

Overclocking https://wiki.debian.org/RaspberryPi#Overclocking_Pi_2
arm_freq=1000
core_freq=500
sdram_freq=400
over_voltage=0
over_voltage_sdram_p=0
over_voltage_sdram_i=0
over_voltage_sdram_c=0

$ ./bench_dd.py
bs=      1 count=1048576  2.071s    1.975us/record     0.506MB/s
bs=      2 count= 524288  1.038s    1.979us/record     1.011MB/s
bs=      4 count= 262144  0.520s    1.984us/record     2.016MB/s
bs=      8 count= 262144  0.520s    1.982us/record     4.036MB/s
bs=     16 count= 262144  0.524s    2.001us/record     7.997MB/s
bs=     32 count= 262144  0.524s    1.999us/record    16.006MB/s
bs=     64 count= 262144  0.692s    2.640us/record    24.246MB/s
bs=    128 count= 262144  0.654s    2.494us/record    51.329MB/s
bs=    256 count= 262144  0.653s    2.492us/record   102.746MB/s
bs=    512 count= 262144  0.672s    2.564us/record   199.718MB/s
bs=   1024 count= 262144  0.732s    2.792us/record   366.773MB/s
bs=   2048 count= 262144  0.785s    2.993us/record   684.160MB/s
bs=   4096 count= 262144  0.968s    3.694us/record  1108.962MB/s
bs=   8192 count= 262144  1.612s    6.148us/record  1332.376MB/s
bs=  16384 count= 131072  1.504s   11.471us/record  1428.238MB/s
bs=  32768 count=  65536  1.497s   22.840us/record  1434.649MB/s
bs=  65536 count=  32768  1.432s   43.706us/record  1499.482MB/s
bs= 131072 count=  16384  1.437s   87.693us/record  1494.671MB/s
bs= 262144 count=   8192  1.426s  174.119us/record  1505.548MB/s
bs= 524288 count=   4096  1.415s  345.540us/record  1517.302MB/s
bs=1048576 count=   2048  1.428s  697.305us/record  1503.756MB/s
bs=2097152 count=   1024  1.430s 1396.846us/record  1501.348MB/s
bs=4194304 count=    512  1.442s 2815.664us/record  1489.632MB/s
bs=8388608 count=    256  1.444s 5642.461us/record  1486.693MB/s
"""
