#!/bin/sh

g++ -g Date.cc Date_unittest.cc -o date_unittest
./date_unittest
g++ -g Date.cc Date_test.cc -o date_test

gpic shift_month.pic | groff | ps2eps --loose --gsbbox > shift_month.eps
eps2png --pnggray -resolution 144 shift_month.eps

gpic shift_month_cumsum.pic | groff | ps2eps --loose --gsbbox > shift_month_cumsum.eps
eps2png --png16m -resolution 144 shift_month_cumsum.eps

mpost linear_regression.mp
ps2eps --loose --gsbbox < linear_regression-1.mps > linear_regression-1.eps
eps2png --png16m -resolution 300 linear_regression-1.eps
ps2eps --loose --gsbbox < linear_regression-2.mps > linear_regression-2.eps
eps2png --png16m -resolution 300 linear_regression-2.eps

latex formula_shift.tex
dvips formula_shift
ps2eps --loose --gsbbox < formula_shift.ps > formula_shift.eps
eps2png --pnggray -resolution 300 formula_shift.eps

latex formula_cumdays.tex
dvips formula_cumdays
ps2eps --loose --gsbbox < formula_cumdays.ps > formula_cumdays.eps
eps2png --pnggray -resolution 300 formula_cumdays.eps

latex formula_floors.tex
dvips formula_floors
ps2eps --loose --gsbbox < formula_floors.ps > formula_floors.eps
eps2png --pnggray -resolution 300 formula_floors.eps

latex formula_linear_reg.tex
dvips formula_linear_reg
ps2eps --loose --gsbbox < formula_linear_reg.ps > formula_linear_reg.eps
eps2png --pnggray -resolution 300 formula_linear_reg.eps
