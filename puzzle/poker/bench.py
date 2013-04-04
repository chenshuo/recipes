#!/usr/bin/python

import time
import poker, poker2, generate

if __name__ == '__main__':

	start = time.time()
	max1 = max(generate.gen(5))
	elapsed = time.time() - start
	print ("%.4f" % (elapsed)), max1

	start = time.time()
	max2 = max(generate.gen(5), key=poker.score)
	elapsed = time.time() - start
	print ("%.4f" % (elapsed)), max2

	start = time.time()
	max3 = max(generate.gen(5), key=poker2.score2)
	elapsed = time.time() - start
	print ("%.4f" % (elapsed)), max3

