#!/bin/tcsh
# turn CFHTLS catalogue format into kk shapelets catalogue format
# filter catalogue $1 to std output

grep -v '#' $1 | awk '{print($2,$3,$20,$21,2*$17,$12,$13,$14,$1,$28)}'

