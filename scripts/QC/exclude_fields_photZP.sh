#while read field
#do
#    ZP_u=0
#    ZP_g=0
#    ZP_r=0
#    ZP_i=0
#    ZP_z=0
#    if [ -f /arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS5000/$field/u/${field}_u_smart_full_SDSS_u_offset.asc ]
#    then
#	ZP_u=`awk '{if ($5>10 && $1>-0.1) print 1; else print 0}' /arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS5000/$field/u/${field}_u_smart_full_SDSS_u_offset.asc`
#    fi
#    if [ -f /arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS5000/$field/g/${field}_g_smart_full_SDSS_g_offset.asc ]
#    then
#	ZP_g=`awk '{if ($5>10 && $1>-0.1) print 1; else print 0}' /arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS5000/$field/g/${field}_g_smart_full_SDSS_g_offset.asc`
#    fi
#    if [ -f /arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS5000/$field/r/${field}_r_smart_full_SDSS_r_offset.asc ]
#    then
#	ZP_r=`awk '{if ($5>10 && $1>-0.1) print 1; else print 0}' /arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS5000/$field/r/${field}_r_smart_full_SDSS_r_offset.asc`
#    fi
#    if [ -f /arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS5000/$field/i/${field}_i_smart_full_SDSS_i_offset.asc ]
#    then
#	ZP_i=`awk '{if ($5>10 && $1<0.1) print 1; else print 0}' /arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS5000/$field/i/${field}_i_smart_full_SDSS_i_offset.asc`
#    fi
#    if [ -f /arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS5000/$field/z/${field}_z_smart_full_SDSS_z_offset.asc ]
#    then
#	ZP_z=`awk '{if ($5>10 && $1>-0.1) print 1; else print 0}' /arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS5000/$field/z/${field}_z_smart_full_SDSS_z_offset.asc`
#    fi
#    echo $field $ZP_u $ZP_g $ZP_r $ZP_i $ZP_z | awk '{if ($2+$3+$4+$5+$6==5) print $1}'
#done<@RUNROOT@/ugriz_only_tiles.txt > @RUNROOT@/ugriz_only_tiles_cleanZP.txt

while read field
do
    ZP_i=0
    if [ -f /arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS5000/$field/i/${field}_i_smart_full_SDSS_i_offset.asc ]
    then
	ZP_i=`awk '{if ($5>10 && $1<0.1) print 1; else print 0}' /arc/projects/unions/catalogues/unions/GAaP_photometry/UNIONS5000/$field/i/${field}_i_smart_full_SDSS_i_offset.asc`
    fi
    echo $field $ZP_i | awk '{if ($2==1) print $1}'
done<@RUNROOT@/ugriz_only_tiles.txt > @RUNROOT@/ugriz_only_tiles_cleanZPi.txt
