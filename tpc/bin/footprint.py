#!/usr/bin/python

from collections import OrderedDict
import re, sys

slabs = {}

sections = []

section = None

for line in sys.stdin:
    m = re.match('===== (.*) =====', line)
    if m:
        section_name = m.group(1)
        # print section_name
        if (section):
            sections.append(section)
        section = (section_name, OrderedDict(), OrderedDict())
        meminfo = True
        continue
    if re.match('slabinfo -', line):
        meminfo = False
        continue
    if meminfo:
        m = re.match('(.*): *(\\d+) kB', line)
        if m:
            section[1][m.group(1)] = int(m.group(2))
    else:
        if line[0] == '#':
            continue
        (slab, active, total, objsize) = line.split()[:4]
        slabs[slab] = int(objsize)
        section[2][slab] = int(active)


sections.append(section)

for i in range(1, len(sections)):
    print '=====', sections[i][0]
    meminfo = sections[i][1]
    old = sections[i-1][1]
    for key in meminfo:
        diff = meminfo[key]-old[key]
        if diff:
            print key, diff

    print '-----'
    slab = sections[i][2]
    old = sections[i-1][2]
    for key in slab:
        diff = slab[key]-old[key]
        if diff:
            print key, slabs[key], diff

