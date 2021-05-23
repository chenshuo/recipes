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

Raspberry Pi 4 running Raspbian GNU/Linux 10 armv7, kernel 5.10
$ python3 bench_dd.py
bs=      1 count=1048576  1.067s    1.018us/record     0.982MB/s
bs=      2 count= 524288  0.529s    1.009us/record     1.982MB/s
bs=      4 count= 524288  0.540s    1.030us/record     3.885MB/s
bs=      8 count= 524288  0.537s    1.025us/record     7.805MB/s
bs=     16 count= 524288  0.533s    1.016us/record    15.741MB/s
bs=     32 count= 524288  0.537s    1.023us/record    31.265MB/s
bs=     64 count= 524288  1.527s    2.913us/record    21.972MB/s
bs=    128 count= 262144  0.758s    2.892us/record    44.258MB/s
bs=    256 count= 262144  0.760s    2.899us/record    88.300MB/s
bs=    512 count= 262144  0.768s    2.930us/record   174.728MB/s
bs=   1024 count= 262144  0.795s    3.034us/record   337.543MB/s
bs=   2048 count= 262144  0.817s    3.117us/record   657.138MB/s
bs=   4096 count= 262144  0.886s    3.378us/record  1212.454MB/s
bs=   8192 count= 262144  1.406s    5.365us/record  1527.034MB/s
bs=  16384 count= 131072  1.294s    9.875us/record  1659.057MB/s
bs=  32768 count=  65536  1.245s   19.003us/record  1724.402MB/s
bs=  65536 count=  32768  1.227s   37.450us/record  1749.962MB/s
bs= 131072 count=  16384  1.264s   77.148us/record  1698.972MB/s
bs= 262144 count=   8192  1.257s  153.500us/record  1707.781MB/s
bs= 524288 count=   4096  1.303s  318.062us/record  1648.385MB/s
bs=1048576 count=   2048  1.503s  733.804us/record  1428.960MB/s
bs=2097152 count=   1024  1.839s 1796.094us/record  1167.618MB/s
bs=4194304 count=    512  1.833s 3580.527us/record  1171.421MB/s
bs=8388608 count=    256  1.860s 7266.406us/record  1154.437MB/s

Raspberry Pi 4 running Debian 11 arm64, kernel 5.10
$ ./bench_dd.py
bs=      1 count=1048576  1.464s    1.396us/record     0.716MB/s
bs=      2 count= 524288  0.729s    1.390us/record     1.439MB/s
bs=      4 count= 524288  0.735s    1.402us/record     2.852MB/s
bs=      8 count= 524288  0.740s    1.411us/record     5.670MB/s
bs=     16 count= 524288  0.746s    1.423us/record    11.246MB/s
bs=     32 count= 524288  0.737s    1.407us/record    22.750MB/s
bs=     64 count= 524288  0.738s    1.408us/record    45.465MB/s
bs=    128 count= 524288  0.745s    1.421us/record    90.060MB/s
bs=    256 count= 524288  0.752s    1.434us/record   178.504MB/s
bs=    512 count= 524288  0.780s    1.488us/record   344.122MB/s
bs=   1024 count= 524288  0.831s    1.585us/record   645.859MB/s
bs=   2048 count= 524288  0.914s    1.742us/record  1175.405MB/s
bs=   4096 count= 524288  1.096s    2.090us/record  1960.027MB/s
bs=   8192 count= 262144  0.750s    2.861us/record  2863.609MB/s
bs=  16384 count= 262144  1.125s    4.290us/record  3819.446MB/s
bs=  32768 count= 131072  1.001s    7.638us/record  4289.905MB/s
bs=  65536 count=  65536  0.975s   14.882us/record  4403.740MB/s
bs= 131072 count=  65536  1.834s   27.978us/record  4684.865MB/s
bs= 262144 count=  32768  2.088s   63.717us/record  4114.190MB/s
bs= 524288 count=  16384  2.347s  143.225us/record  3660.587MB/s
bs=1048576 count=   8192  3.553s  433.748us/record  2417.480MB/s
bs=2097152 count=   4096  5.754s 1404.768us/record  1492.881MB/s
bs=4194304 count=   2048  6.109s 2982.832us/record  1406.148MB/s
bs=8388608 count=   1024  6.307s 6159.189us/record  1361.966MB/s

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

================================================================

Raspberry Pi 3 running Raspbian GNU/Linux 10 armv7, kernel 5.10
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

Raspberry Pi 3 running Ubuntu server 21.04 arm64, kernel 5.11
$ ./bench_dd.py
bs=      1 count=1048576 10.017s    9.553us/record     0.105MB/s
bs=      2 count= 524288  5.021s    9.577us/record     0.209MB/s
bs=      4 count= 262144  2.505s    9.554us/record     0.419MB/s
bs=      8 count= 131072  1.251s    9.546us/record     0.838MB/s
bs=     16 count=  65536  0.631s    9.623us/record     1.663MB/s
bs=     32 count=  65536  0.629s    9.605us/record     3.332MB/s
bs=     64 count=  65536  0.630s    9.606us/record     6.663MB/s
bs=    128 count=  65536  0.636s    9.700us/record    13.195MB/s
bs=    256 count=  65536  0.634s    9.667us/record    26.481MB/s
bs=    512 count=  65536  0.635s    9.687us/record    52.854MB/s
bs=   1024 count=  65536  0.645s    9.840us/record   104.064MB/s
bs=   2048 count=  65536  0.655s   10.002us/record   204.760MB/s
bs=   4096 count=  65536  0.688s   10.498us/record   390.177MB/s
bs=   8192 count=  65536  0.903s   13.782us/record   594.390MB/s
bs=  16384 count=  65536  1.343s   20.487us/record   799.712MB/s
bs=  32768 count=  32768  1.105s   33.717us/record   971.844MB/s
bs=  65536 count=  16384  0.987s   60.240us/record  1087.909MB/s
bs= 131072 count=  16384  1.854s  113.177us/record  1158.110MB/s
bs= 262144 count=   8192  1.801s  219.850us/record  1192.377MB/s
bs= 524288 count=   4096  1.796s  438.547us/record  1195.511MB/s
bs=1048576 count=   2048  1.972s  963.125us/record  1088.723MB/s
bs=2097152 count=   1024  2.151s 2100.605us/record   998.356MB/s
bs=4194304 count=    512  2.253s 4400.293us/record   953.187MB/s
bs=8388608 count=    256  2.306s 9005.898us/record   931.457MB/s

Raspberry Pi 3 running Debian 11 arm64, kernel 5.10
$ ./bench_dd.py
bs=      1 count=1048576  2.171s    2.070us/record     0.483MB/s
bs=      2 count= 524288  1.069s    2.039us/record     0.981MB/s
bs=      4 count= 262144  0.543s    2.071us/record     1.931MB/s
bs=      8 count= 262144  0.539s    2.058us/record     3.888MB/s
bs=     16 count= 262144  0.543s    2.070us/record     7.730MB/s
bs=     32 count= 262144  0.543s    2.072us/record    15.443MB/s
bs=     64 count= 262144  0.544s    2.077us/record    30.817MB/s
bs=    128 count= 262144  0.552s    2.105us/record    60.802MB/s
bs=    256 count= 262144  0.557s    2.126us/record   120.423MB/s
bs=    512 count= 262144  0.572s    2.184us/record   234.471MB/s
bs=   1024 count= 262144  0.599s    2.286us/record   447.998MB/s
bs=   2048 count= 262144  0.656s    2.501us/record   818.834MB/s
bs=   4096 count= 262144  0.767s    2.926us/record  1399.933MB/s
bs=   8192 count= 262144  1.018s    3.883us/record  2109.512MB/s
bs=  16384 count= 131072  0.757s    5.776us/record  2836.329MB/s
bs=  32768 count= 131072  1.252s    9.549us/record  3431.527MB/s
bs=  65536 count=  65536  1.116s   17.026us/record  3849.261MB/s
bs= 131072 count=  32768  1.052s   32.093us/record  4084.183MB/s
bs= 262144 count=  16384  1.045s   63.790us/record  4109.505MB/s
bs= 524288 count=   8192  1.092s  133.292us/record  3933.372MB/s
bs=1048576 count=   4096  2.321s  566.655us/record  1850.465MB/s
bs=2097152 count=   2048  2.984s 1457.168us/record  1439.197MB/s
bs=4194304 count=   1024  3.431s 3350.625us/record  1251.798MB/s
bs=8388608 count=    512  3.456s 6750.234us/record  1242.714MB/s

================================================================

Raspberry Pi 2 running Raspbian GNU/Linux 10 armv7, kernel 5.10
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

================================================================

HP e8300, CPU i7-3770

freebsd13% ./bench_dd.py
bs=      1 count=1048576  0.728s    0.694us/record     1.440MB/s
bs=      2 count=1048576  0.573s    0.547us/record     3.658MB/s
bs=      4 count=1048576  0.565s    0.539us/record     7.418MB/s
bs=      8 count=1048576  0.575s    0.548us/record    14.595MB/s
bs=     16 count=1048576  0.572s    0.546us/record    29.329MB/s
bs=     32 count=1048576  0.574s    0.548us/record    58.435MB/s
bs=     64 count=1048576  0.573s    0.546us/record   117.174MB/s
bs=    128 count=1048576  0.568s    0.542us/record   236.122MB/s
bs=    256 count=1048576  0.577s    0.550us/record   465.528MB/s
bs=    512 count=1048576  0.585s    0.558us/record   917.797MB/s
bs=   1024 count=1048576  0.591s    0.564us/record  1815.495MB/s
bs=   2048 count=1048576  0.610s    0.582us/record  3517.599MB/s
bs=   4096 count=1048576  0.648s    0.618us/record  6624.642MB/s
bs=   8192 count=1048576  0.716s    0.683us/record 12000.920MB/s
bs=  16384 count=1048576  0.886s    0.845us/record 19391.838MB/s
bs=  32768 count=1048576  1.414s    1.349us/record 24291.204MB/s
bs=  65536 count= 524288  1.167s    2.226us/record 29446.678MB/s
bs= 131072 count= 262144  1.049s    4.001us/record 32757.097MB/s
bs= 262144 count= 131072  0.996s    7.597us/record 34507.742MB/s
bs= 524288 count= 131072  1.938s   14.784us/record 35462.791MB/s
bs=1048576 count=  65536  1.954s   29.814us/record 35170.740MB/s
bs=2097152 count=  32768  1.978s   60.353us/record 34748.329MB/s
bs=4194304 count=  16384  2.007s  122.520us/record 34233.639MB/s
bs=8388608 count=   8192  2.103s  256.698us/record 32678.930MB/s

debian11$ ./bench_dd.py
bs=      1 count=1048576  0.558s    0.532us/record     1.880MB/s
bs=      2 count=1048576  0.550s    0.524us/record     3.814MB/s
bs=      4 count=1048576  0.551s    0.526us/record     7.611MB/s
bs=      8 count=1048576  0.550s    0.525us/record    15.252MB/s
bs=     16 count=1048576  0.550s    0.524us/record    30.509MB/s
bs=     32 count=1048576  0.550s    0.524us/record    61.048MB/s
bs=     64 count=1048576  0.553s    0.527us/record   121.398MB/s
bs=    128 count=1048576  0.556s    0.530us/record   241.471MB/s
bs=    256 count=1048576  0.565s    0.538us/record   475.482MB/s
bs=    512 count=1048576  0.583s    0.556us/record   921.523MB/s
bs=   1024 count=1048576  0.608s    0.580us/record  1764.989MB/s
bs=   2048 count=1048576  0.640s    0.611us/record  3353.923MB/s
bs=   4096 count=1048576  0.701s    0.669us/record  6126.015MB/s
bs=   8192 count=1048576  0.870s    0.830us/record  9870.674MB/s
bs=  16384 count=1048576  1.191s    1.136us/record 14427.529MB/s
bs=  32768 count= 524288  1.004s    1.915us/record 17109.038MB/s
bs=  65536 count= 262144  0.822s    3.135us/record 20902.551MB/s
bs= 131072 count= 262144  1.496s    5.705us/record 22973.575MB/s
bs= 262144 count= 131072  1.468s   11.200us/record 23406.614MB/s
bs= 524288 count=  65536  1.519s   23.171us/record 22626.825MB/s
bs=1048576 count=  32768  1.495s   45.614us/record 22988.023MB/s
bs=2097152 count=  16384  1.487s   90.750us/record 23109.237MB/s
bs=4194304 count=   8192  1.474s  179.918us/record 23312.281MB/s
bs=8388608 count=   4096  1.588s  387.625us/record 21641.067MB/s
"""
