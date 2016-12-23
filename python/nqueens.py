#!/usr/bin/python

# http://code.activestate.com/recipes/576647-eight-queens-six-lines/

from itertools import permutations

N = 8
cols = range(N)
for perm in permutations(cols):
    if (N == len(set(perm[i]-i for i in cols))
          == len(set(perm[i]+i for i in cols))):
        print perm
