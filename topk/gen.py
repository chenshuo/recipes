#!/usr/bin/python3

import numpy

words = 100*1000*1000
S = 1.0001

output = open('random_words', 'w')

for x in range(words):
    output.write("%x\n" % numpy.random.zipf(S))
