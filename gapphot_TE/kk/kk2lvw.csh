#!/bin/tcsh

# merge CFHTLS catalogue $1 and KK catalogue $2 into a catalogue with
# output cols id,x y ra,dec,g1,g2,rh,mag,wt,flg

# required both catalogues to be sorted on ID.

grep -v '#' $1 | sort -n > _lvw.in
grep -v '#' $2 | sort -n -k 14 > _kk.in

awk '{id=$14;g1=$9;g2=$10;wt=0.2**2/(0.2**2+$11**2+$12**2);getline < "_lvw.in"; while($1!=id) getline < "_lvw.in"; print(id,$2,$3,$4,$5,g1,g2,$17,$18,wt)}' _kk.in 

\rm -f _lvw.in _kk.in


