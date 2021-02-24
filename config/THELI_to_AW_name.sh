#!/bin/sh

patch=S  #or N

sed '+s/KIDS_/KiDS_DR4.0_/' < K1000_${patch}_THELI.txt | sed '+s/p/./' | sed '+s/m/-/' | sed '+s/p/./' > K1000_${patch}_AWname.txt
sed '+s/KIDS_//' < K1000_${patch}_THELI.txt | sed '+s/p/./' | sed '+s/m/-/' | sed '+s/p/./' | sed '+s/_/   /' > K1000_${patch}_racent_deccent.txt
paste K1000_${patch}_AWname.txt K1000_${patch}_THELI.txt K1000_${patch}_racent_deccent.txt > K1000_$patch.txt