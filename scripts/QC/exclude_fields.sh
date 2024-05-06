while read field
do
    star_frac_u=`grep $field @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_u_starfrac.asc |awk '{if ($4>0.8) print 1; else print 0}'`
    star_frac_g=`grep $field @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_g_starfrac.asc |awk '{if ($4>0.8) print 1; else print 0}'`
    star_frac_r=`grep $field @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_r_starfrac.asc |awk '{if ($4>0.8) print 1; else print 0}'`
    star_frac_i=`grep $field @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_i_starfrac.asc |awk '{if ($4>0.8) print 1; else print 0}'`
    xy_range_u=`grep $field @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_u_xy_range.asc |awk '{if ($2-$4<1000 && $3-$5<1000) print 1; else print 0}'`
    xy_range_g=`grep $field @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_g_xy_range.asc |awk '{if ($2-$4<1000 && $3-$5<1000) print 1; else print 0}'`
    xy_range_r=`grep $field @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_r_xy_range.asc |awk '{if ($2-$4<1000 && $3-$5<1000) print 1; else print 0}'`
    xy_range_i=`grep $field @RUNROOT@/@WORKINGDIR@/QC/UNIONS2000_i_xy_range.asc |awk '{if ($2-$4<1000 && $3-$5<1000) print 1; else print 0}'`
    echo $field $star_frac_u $star_frac_g $star_frac_r $star_frac_i $xy_range_u $xy_range_g $xy_range_r $xy_range_i | awk '{if (NF==9 && $2+$3+$4+$5+$6+$7+$8+$9==8) print $1}'
done<@RUNROOT@/@POINTINGLIST@ > @RUNROOT@/@POINTINGLIST@_clean.txt
