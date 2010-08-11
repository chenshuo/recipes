#!/bin/sh

g++ -g Date.cc Date_unittest.cc -o date_unittest
./date_unittest

gpic shift_month.pic | groff | ps2eps --loose --gsbbox > shift_month.eps
eps2png --pnggray -resolution 144 shift_month.eps

mpost linear_regression.mp
ps2eps --loose --gsbbox < linear_regression-1.mps > linear_regression-1.eps
eps2png --png16m -resolution 300 linear_regression-1.eps
ps2eps --loose --gsbbox < linear_regression-2.mps > linear_regression-2.eps
eps2png --png16m -resolution 300 linear_regression-2.eps
