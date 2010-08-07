#!/bin/sh
gpic shift_month.pic | groff | ps2eps | eps2png
mpost linear_regression.mp
#eps2png
