#!/usr/bin/python

import operator
import sys

def get_ranks(hand):
	ranks = ['--23456789TJQKA'.index(r) for r, s in hand]
	ranks.sort(reverse = True)
	if ranks == [14, 5, 4, 3, 2]:
		ranks = [5, 4, 3, 2, 1]
	return ranks

def straight(ranks):
	return len(set(ranks)) == 5 and (max(ranks) - min(ranks) == 4)

def flush(hand):
	suits = [s for r, s in hand]
	return len(set(suits)) == 1

def kind(n, ranks):
	for r in ranks:
		if ranks.count(r) == n:
			return r
	return None

def two_pair(ranks):
	pair = kind(2, ranks)
	lowpair = kind(2, list(reversed(ranks)))
	if pair and lowpair != pair:
		return (pair, lowpair)
	else:
		return None

def score(hand):
	assert len(hand) == 5
	ranks = get_ranks(hand)
	if straight(ranks) and flush(hand):
		return (8, max(ranks))
	elif kind(4, ranks):
		return (7, kind(4, ranks), kind(1, ranks))
	elif kind(3, ranks) and kind(2, ranks):
		return (6, kind(3, ranks), kind(2, ranks))
	elif flush(hand):
		return (5, ranks)
	elif straight(ranks):
		return (4, max(ranks))
	elif kind(3, ranks):
		return (3, kind(3, ranks), ranks)
	elif two_pair(ranks):
		return (2, two_pair(ranks), ranks)
	elif kind(2, ranks):
		return (1, kind(2, ranks), ranks)
	else:
		return (0, ranks)

def multiply(counts, ranks):
	cards = []
	for i in range(len(counts)):
		cards.extend((ranks[i], ) * counts[i])
	return cards

def score2(hand):
	assert len(hand) == 5
	cards = get_ranks(hand)
	assert len(cards) == len(hand)
	groups = [(cards.count(x), x) for x in set(cards)]
	groups.sort(reverse = True)
	counts, ranks = zip(*groups)
	cards = multiply(counts, ranks)
	assert sum(counts) == len(hand)
	assert len(set(ranks)) == len(ranks)
	straight = len(ranks) == 5 and max(ranks) - min(ranks) == 4
	suits = [s for r, s in hand]
	flush = len(set(suits)) == 1

	if (5, ) == counts:
		score = 9
	elif straight and flush:
		score = 8
	elif (4, 1) == counts:
		score = 7
	elif (3, 2) == counts:
		score = 6
	elif flush:
		score = 5
	elif straight:
		score = 4
	elif (3, 1, 1) == counts:
		score = 3
	elif (2, 2, 1) == counts:
		score = 2
	elif (2, 1, 1, 1) == counts:
		score = 1
	else:
		score = 0
	return score, cards

if __name__ == '__main__':
	hand = sys.argv[1:]
	print score(hand)
	print score2(hand)
