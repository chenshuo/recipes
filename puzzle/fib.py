
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import sys


def calc(N, c, level):
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
        calc(half+1, c, level+1)


def main(argv):
    if len(argv) > 1:
        N = int(argv[1])
        c = set([0, 1, 2])
        print('digraph G {')
        print('node [shape=box]')
        print('"1" [style=filled]')
        print('"2" [style=filled]')
        calc(N, c, 0)
        print('}')
    else:
        print("Usage: %s N" % argv[0])


if __name__ == '__main__':
    main(sys.argv)

