#!/usr/bin/python

import unittest
import poker, poker2

class TestPoker(unittest.TestCase):

	def setUp(self):
		self.sf6 = "2H 3H 4H 5H 6H".split()  # straight flush
		self.sf5 = "2H 3H 4H 5H AH".split()
		self.sfA = "TH JH QH KH AH".split()
		self.fk9 = "9D 9H 9S 9C 7D".split()  # four of a kind
		self.fhT = "TD TC TH 7C 7D".split()  # full house
		self.fh7 = "TD TC 7H 7C 7D".split()  # full house
		self.fl = "2H 3H 4H 5H 7H".split()   # flush
		self.st = "2H 3H 4H 5H 6D".split()   # straight
		self.tk = "9D 9H 9S 8C 7D".split()   # three of a kind
		self.tp = "5S 5D 9H 9C 6S".split()   # two pairs
		self.op = "5S 5D 9H 8C 6S".split()   # one pair
		self.hc = "5S 4D 9H 8C 6S".split()   # high card

	def test_get_ranks(self):
		self.assertEqual([6, 5, 4, 3, 2], poker.get_ranks(self.sf6))
		self.assertEqual([5, 4, 3, 2, 1], poker.get_ranks(self.sf5))
		self.assertEqual([14, 13, 12, 11, 10], poker.get_ranks(self.sfA))

	def test_flush(self):
		self.assertTrue(poker.flush(self.sf6))
		self.assertFalse(poker.flush(self.fk9))

	def test_straigh(self):
		self.assertTrue(poker.flush(self.sf6))
		self.assertTrue(poker.flush(self.sf5))
		self.assertTrue(poker.flush(self.sfA))
		self.assertFalse(poker.flush(self.fk9))

	def test_kind(self):
		self.assertEqual(9, poker.kind(4, poker.get_ranks(self.fk9)))
		self.assertEqual(10, poker.kind(3, poker.get_ranks(self.fhT)))
		self.assertEqual(7, poker.kind(2, poker.get_ranks(self.fhT)))
		self.assertEqual(9, poker.kind(2, poker.get_ranks(self.tp)))
		self.assertEqual(5, poker.kind(2, list(reversed(poker.get_ranks(self.tp)))))

	def test_score2(self):
		print poker2.score2(self.sf6)
		print poker2.score2(self.fk9)
		print poker2.score2(self.fhT)
		print poker2.score2(self.fh7)
		print poker2.score2(self.fl)
		print poker2.score2(self.st)
		print poker2.score2(self.tk)
		print poker2.score2(self.tp)
		print poker2.score2(self.op)
		print poker2.score2(self.hc)

if __name__ == '__main__':
	unittest.main()
