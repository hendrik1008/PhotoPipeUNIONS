grep -v 'm2' < K1000_eff_area.txt | grep -v 'm3' > K1000_N_eff_area.txt 
grep -E 'm2|m3' < K1000_eff_area.txt > K1000_S_eff_area.txt
