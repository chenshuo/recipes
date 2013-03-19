#!/usr/bin/python

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

if __name__ == '__main__':
	hand = sys.argv[1:]
	print score(hand)
