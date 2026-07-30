#!/usr/bin/env perl

# this script creates region files for the masking of bright stars it
# is a modified version of
# /home/schrabback/skript_develop/shaggles_lensing/auto_create_star_region.pl

# In the current version the header information is not used, but x,y
# as externally computed from xy2sky. Otherwise things go wrong
# (projection due to large field??)

# Version history:
#
# 12.05.2008:
# - I removed all WCS oriented stuff. To decide whether
#   a mask should be created we use pixel cooridnates. We accept
#   all masks falling within the image area including a 100 pixel margin
#   around the image boundaries.
# - I substituted the header command line argument by two quantities
#   giving NAXIS1 and NAXIS2 of the image under consideration. The command line
#   argument 'influence radius' was deleted. 

use constant PI    => 4 * atan2(1, 1);


$outBuf='# Region file format: DS9 version 3.0
# Filename: masked.fits
# Auto created by mask_create_star_regions.pl
global color=green font="helvetica 10 normal" select=1 edit=1 move=1 delete=1 include=1 fixed=0 source'."\n";

$starcat=$ARGV[0];
$xwidth=$ARGV[1]; # width of image in pixels
$ywidth=$ARGV[2]; # width of image in pixels
$region_conf=$ARGV[3];
$mask_radial_offset_factor=$ARGV[4]; # radially displace mask by this
                                     # offset (from 1)
$dir=$ARGV[5];

$centerx = $xwidth / 2.;
$centery = $ywidth / 2.;


print "mask_create_star_region.pl $ARGV[0] $ARGV[1] $ARGV[2] $ARGV[3] $ARGV[4]\n";


if (-r ($starcat)) {
    open (DATAFILE,($starcat));  
    $starBuf  = join("", <DATAFILE>);      
    close (DATAFILE);
} else {
    die "Could not find file $starcat\n";
}

if (-r ($region_conf)) {
    open (DATAFILE,($region_conf));  
    $region_confBuf  = join("", <DATAFILE>);      
    close (DATAFILE);
} else {
    die "Could not find file $region_conf\n";
}

$region_confBuf =~ s/^\s*//is;
$region_confBuf =~ s/\s*$//is;

# browse through star list
#print "\nStars within $influence_radius arcmin from the pointing:\n";
$useBuf='';
$i=0;
foreach (split /\n/, $starBuf) {
    $line=$_; 
    $line=~s/^\s*//igs;
    ($xext, $yext, $m, @rest)=split /\s\s*/, $line;

    if ($xext > -100 && $yext > -100 && 
        $xext < ($xwidth + 100) && $yext < ($ywidth + 100)) {
	# stretch to account for scaling of halo position
	$x=$xext+$mask_radial_offset_factor*($xext-$centerx);
	$y=$yext+$mask_radial_offset_factor*($yext-$centery);
	
	$useBuf.="$i $line $x $y\n";
	$i++;
    }
}

# Filter double entries from useBuf with some AI:
# No, taken out in this version:
$goodBuf=$useBuf;

#print "\nStars which will be masked:\n";
$starcount=0;
foreach (split /\n/, $goodBuf) {
    $line=$_; 
    ($i, $xext, $yext, $m, $x, $y, @rest)=split /\s\s*/, $line;
        # parse the config file defining the region sizes to compute
        # region size for this star
	$mag_last=100;
	foreach (split /\n/, $region_confBuf) {
	    $defline=$_; 
	    ($mag_this, $rhalo_this, $rspike_this, $hspike_this, @rest) = split /\s\s*/, $defline;
	    if (( $m<$mag_last ) && ( $m>=$mag_this )) {
       	        # print "$m between $mag_last and $mag_this\n"; We found the
		# right intervall
		$relative=($m-$mag_last)/($mag_this-$mag_last);
		$rhalo=$rhalo_last+($rhalo_this-$rhalo_last)*$relative;
		$rspike=$rspike_last+($rspike_this-$rspike_last)*$relative;
		$hspike=$hspike_last+($hspike_this-$hspike_last)*$relative;
	    }
	    $mag_last=$mag_this;
	    $rhalo_last=$rhalo_this;
	    $rspike_last=$rspike_this;	
	    $hspike_last=$hspike_this;
	}

	# create the polygon
	$outBuf.="physical;polygon(";
	#
	$xn=$x+$rhalo;
	$yn=$y-$hspike/2;
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x+$rspike;
	$yn=$y-$hspike/2;
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x+$rspike;
	$yn=$y+$hspike/2;
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x+$rhalo;
	$yn=$y+$hspike/2;
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x+$rhalo/sqrt(2);
	$yn=$y+$rhalo/sqrt(2);
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x+$hspike/2;
	$yn=$y+$rhalo;
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x+$hspike/2;
	$yn=$y+$rspike;
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x-$hspike/2;
	$yn=$y+$rspike;
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x-$hspike/2;
	$yn=$y+$rhalo;
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x-$rhalo/sqrt(2);
	$yn=$y+$rhalo/sqrt(2);
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x-$rhalo;
	$yn=$y+$hspike/2;
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x-$rspike;
	$yn=$y+$hspike/2;
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x-$rspike;
	$yn=$y-$hspike/2;
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x-$rhalo;
	$yn=$y-$hspike/2;
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x-$rhalo/sqrt(2);
	$yn=$y-$rhalo/sqrt(2);
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x-$hspike/2;
	$yn=$y-$rhalo;
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x-$hspike/2;
	$yn=$y-$rspike;
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x+$hspike/2;
	$yn=$y-$rspike;
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x+$hspike/2;
	$yn=$y-$rhalo;
	$outBuf.=$xn.",".$yn.",";
	#
	$xn=$x+$rhalo/sqrt(2);
	$yn=$y-$rhalo/sqrt(2);
	$outBuf.=$xn.",".$yn.")\n";
	
    
    $starcount++;

}

print "Created masks for $starcount stars\n";


    $cor_file=">${dir}/auto_brightstars.reg";
    open CORFILE, "$cor_file";
    print CORFILE "$outBuf";
    close CORFILE;
    print "Auto regions written to $cor_file\n\n";
