#!/usr/bin/python

import numpy

words = 1000*1000
S = 1.05
ALPHABET = '_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789'

def number2word(x):
    assert x > 0
    arr = []
    base = len(ALPHABET)
    div = x
    while div > 0:
        div, mod = divmod(div, base)
        arr.append(ALPHABET[mod])
    return ''.join(arr)

output = open('random_words', 'w')

for x in xrange(words):
    word = number2word(numpy.random.zipf(S))
    output.write(word)
    output.write('\n')

