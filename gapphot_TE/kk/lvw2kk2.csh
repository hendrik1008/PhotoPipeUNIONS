#!/bin/tcsh
# turn CFHTLS catalogue format into kk shapelets catalogue format
# filter catalogue $1 to std output

grep -v '#' $1 | awk '{print($2,$3,$24,$25,2*$21,$16,$17,$18,$1,$32)}'

