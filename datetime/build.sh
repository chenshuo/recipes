#!/bin/sh

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
