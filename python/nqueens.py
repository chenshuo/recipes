#!/usr/bin/python

# http://code.activestate.com/recipes/576647-eight-queens-six-lines/

from itertools import permutations

N = 8
for rows in permutations(range(N)):
    if (N == len(set(rows[i]-i for i in range(N)))    # Diagonal
          == len(set(rows[i]+i for i in range(N)))):  # Anti-diagonal
        print rows
