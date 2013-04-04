#!/usr/bin/python

import itertools
import poker

def gen(num):
	cards = []
	ranks = '23456789TJQKA'
	for rank in reversed(ranks):
		for suit in 'SHDC':
			cards.append(rank + suit)
	return itertools.combinations(cards, num)

if __name__ == '__main__':
	scores = []
	for hand in gen(5):
		scores.append((poker.score(hand), " ".join(hand)))
	scores.sort(reverse=True)
	for s in scores:
		print s
