#!/usr/bin/python

import random

word_len = 5
alphabet = 'ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-'

output = open('word_count', 'w')
words = set()
N = 1000*1000
for x in xrange(N):
    arr = [random.choice(alphabet) for i in range(word_len)]
    words.add(''.join(arr))

print len(words)
for word in words:
    output.write(word)
    output.write('\t')
    output.write(str(random.randint(1, 2*N)))
    output.write('\n')

