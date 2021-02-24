#!/bin/tcsh

# read in a star shapelet catalogue $1, generate a new cat in which the
# centers have been adjusted with the shifts applied in shapelet
# fitting. Cap the shift to 0.5 x beta.

~/data/shapelets/kk/shummary < $1 | grep -v "#" | \
  awk '{dx=$9;dy=$10;beta=$13;if (dx*dx+dy*dy>beta*beta/4) {fac=beta/2/sqrt(dx*dx+dy*dy);dx=dx*fac;dy=dy*fac}; print($1-dx,$2-dy,$3,$4,$5,2,2,0,$6,$7)}'


