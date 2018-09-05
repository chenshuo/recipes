
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import sys


total_mul = 0
total_add = 0

def calc(N, c, level):
    global total_mul, total_add
    if N not in c:
        half = N // 2;
        if N % 2 == 0:
            print('%s"%d" -> "%d"' % (" "*level*2, half-1, N))
        print('%s"%d" -> "%d"' % (" "*level*2, half, N))
        print('%s"%d" -> "%d"' % (" "*level*2, half+1, N))
        c.add(N)
        if N % 2 == 0:
            calc(half-1, c, level+1)
            calc(half, c, level+1)
            c.add(half+1)
            total_add += 2
            total_mul += 1
            print('%s"%d" -> "%d" [color=green,constraint=false]' % (" "*level*2, half-1, half+1))
            print('%s"%d" -> "%d" [color=green,constraint=false]' % (" "*level*2, half, half+1))
        else:
            calc(half, c, level+1)
            calc(half+1, c, level+1)
            total_add += 1
            total_mul += 2


def main(argv):
    if len(argv) > 1:
        N = int(argv[1])
        c = set([1, 2, 3])
        print('digraph G {')
        print('node [shape=box]')
        for k in sorted(c):
            print('"%d" [style=filled]' % k)
        calc(N, c, 0)
        print('}\n// total_add = %d, total_mul = %d' % (total_add, total_mul))

    else:
        print("Usage: %s N" % argv[0])


if __name__ == '__main__':
    main(sys.argv)

