#!/usr/bin/python

import re, sys

# 0x4460e2 malloc (17408) returns 0xd96fe0
M = re.compile('.* malloc \((\\d+)\) returns (.*)')
F = re.compile('freed (.*)')
S = re.compile('========+ (.*)')

alloc = {}
section = ''

def dump():
    sections = {}
    for addr in alloc:
        sec = alloc[addr][1]
        if sec in sections:
            sections[sec] += alloc[addr][0]
        else:
            sections[sec] = alloc[addr][0]

    print section
    for key in sorted(sections):
        print "   ", key, sections[key]

hook = 'hook'
if len(sys.argv) > 1:
    hook = sys.argv[1]

with open(hook) as f:
    for line in f:
        m = M.search(line)
        if m:
            # print 'alloc', m.group(1)
            alloc[m.group(2)] = (int(m.group(1)), section)
            continue
        m = F.match(line)
        if m:
            if m.group(1) in alloc:
                del alloc[m.group(1)]
            else:
                # print 'free', m.group(1)
                pass
            continue
        m = S.match(line)
        if m:
            dump()
            section = m.group(1)


print alloc

# after handshaking 0
# {'SSL_handshake client': 7518, 'BIO_new_bio_pair 0': 35136, 'SSL_new client': 2128, 'SSL_handshake server': 146, 'SSL_new server': 3092, 'SSL_connect': 33680, 'SSL_accept': 36755}
# client 7518 + 2128 + 33680 = 43326
# server 146 + 3092 + 36755 = 39993


# after handshaking 1
# {'SSL_connect 0': 35432, 'SSL_accept 0': 40400, 'SSL_new server 0': 112, 'SSL_new server 1': 2964, 'BIO_new_bio_pair 1': 35152, 'SSL_connect 1': 405, 'SSL_accept 1': 760, 'SSL_new client 1': 2360, 'SSL_handshake server 1': 1792, 'SSL_handshake client 1': 8924, 'SSL_handshake client 0': 112}
# client 2360 + 405 + 8924 = 11689
# server 2964 + 760 + 1792 = 5516
