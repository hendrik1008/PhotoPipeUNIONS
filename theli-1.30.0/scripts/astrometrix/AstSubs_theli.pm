# Last revised: May 8, 2004

# HISTORY COMMENTS for changements within the
# GaBoDS pipeline context. The original file
# is from v1.2 of Mario Radovich's ASTROMETRIX
#
# 18.07.2005:
# I adapted calls to 'read_ldac' and 'wheadasc'
# to the new syntax of these functions (see comments
# in file 'nldacsub.pm' for details).
#
# 14.02.2007:
# - A new flag 'BADCCD' is introduced. If an image has set
#   this header keyword to '1' it will not ne considered for
#   astrometric solutions. The BADCCD flag also appears as
#   header keyword in the ASTROMETRIX headers.
# - We included the possibility to use USNOB1 and SDSS3
#   as astrometric standardstar catalogues (thanks to P. Hudelot).
# - I refined the initial checking whether there are standard
#   sources for a certain chips at all. We mark an input chip
#   as bad if, within its boundaries, there are less than 10
#   reference sources. The old procedure did not catch bad chips
#   as it asked for sources within the boundaries plus some
#   margin of 10 arcmin. Hence, bad chips slipped through
#   if there happend to be sources within these
#
# 12.06.2007:
# I added code to use the SDSS4 and SDSS5 catalogues for
# astrometric calibration (thanks to P. Hudelot)
#
# 04.07.2007:
# - I deleted the superfulous read_GSC2 subroutine. The GSC
#   catalogue is handled by read_CDS.
# - The read_CDS function now obtains the Ra and Dec arguments
#   in decimal degrees. If conversions to sexagesimal notation
#   are necessary they are handled within this function. The
#   change was necessary because the 2MASS catalogue needs to
#   be queries with decimal degrees.
# - Only astrometric standards with a magnitude brighter than 20
#   are considered for calibration.

# TODO:
# Make the magnitude limit up to which objects from astrometric
# standards are used for astrometric calibration a configuration
# parameter.

sub mkwcs
  {

    my ($crpix1, $crpix2, $cd11, $cd12, $cd21, $cd22, $offx, $offy, $angle, $scale) = @_;

    return ($crpix1, $crpix2, $cd11, $cd12, $cd21, $cd22) if ($offx==0 && $offy==0 && $angle==0 && $scale==1);

    my $xpix=float[1,1,1,500,500,500,2000,2000,2000];
    my $ypix=float[1,2000,4000,1,2000,4000,1,2000,4000];


    #get PROJECTION PLANE COORDINATES  from PIXEL coordinates
    # using the old linear metric

    my $px = $xpix-$crpix1;
    my $py = $ypix-$crpix2;
    my $ox = $cd11 * $px + $cd12*$py;
    my $oy =  $cd21 * $px + $cd22*$py;

    my ($x, $y);

    #Now apply the new transformation


    if ($angle != 0) {
      my $rang=$angle*3.14159265/180.;
      my $cosx=cos($rang);
      my $sinx=sin($rang);

      #Ok, now rotate and backsubstitute in the original pdl

      #      my $midx=(max($ox)+min($ox))/2.;
      #      my $midy=(max($oy)+min($oy))/2.;

      #$ox -= $midx;
      #$oy -= $midy;

      #$x = double($scale*($ox * $cosx - $oy * $sinx) + $midx);
      #$y = double($scale*($ox * $sinx + $oy * $cosx) + $midy);

      $x = double($scale*($ox * $cosx - $oy * $sinx));
      $y = double($scale*($ox * $sinx + $oy * $cosx));

    }
    else {
      $x= double($scale*$ox);
      $y = double($scale*$oy);
    }

    $x+=$offx; $y+=$offy;

    # get PROJECTION PLANE COORDINATES
    #   from CELESTIAL SPHERICAL coordinates and CRVALj
    #my ($x, $y) =  Mylib::Astrom::celfwd($a, $d, $ap, $dp);

    # Make the  fit of new PROJECTION PLANE COORDINATES
    #   to PIXEL COORDINATES

    my $ord=1;
    my $sig=ones(float, $x->nelem);
    #print "!!!!!!!!! $xpix $x\n $ypix $y\n";

    my ($aa, $ax, $ad, $ay);

    $aa = Mylib::Pfit::pfitxy($xpix, $ypix, $x, $sig, $ord);
    $ad = Mylib::Pfit::pfitxy($xpix, $ypix, $y, $sig, $ord);


    #compute the linear part of the transformation

    $ax=$aa->at(0);
    $cd11=$aa->at(1);
    $cd12=$aa->at(2);

    #  $ad = Mylib::Pfit::pfitxy($xpix, $ypix, $y, $sig, $ord);
    $ay=$ad->at(0);
    $cd21=$ad->at(1);
    $cd22=$ad->at(2);

#    $crpix2 = ($ay-$cd21/$cd11*$ax)/($cd21/$cd11*$cd12-$cd22);
#    $crpix1 = ($ax+$cd12*$crpix2)/(-$cd11);


     $crpix2 = ($cd11*$ay-$cd21*$ax)/($cd21*$cd12-$cd22*$cd11);
     $crpix1 = ($cd22*$ax-$cd12*$ay)/($cd12*$cd21-$cd11*$cd22);


    return ($crpix1, $crpix2, $cd11, $cd12, $cd21, $cd22);

  }

sub chgwcs  {

  my ($file, $opt)=@_;

  if (ref($opt) ne 'HASH') {
           print "Error in CHGWCS !!!!\nERROR\n";
           die "\n";
     }

  my ($offx, $offy, $angle, $scale, $KEY, $KVAL);

  if (exists $opt->{offx} ) {$offx=$$opt{offx}} else {$offx=0}
  if (exists $opt->{offy} ) {$offy=$$opt{offy}}  else {$offy=0}
  if (exists $opt->{angle} ) {$angle=$$opt{angle}} else {$angle=0}
  if (exists $opt->{scale} ) {$scale=$$opt{scale}} else {$scale=1}
  if (exists $opt->{key} ) {$KEY=$$opt{key}} else {$KEY=''}
  if (exists $opt->{kvalue} ) {$KVAL=$$opt{kvalue}} else {$KVAL='T'}

  return if ($offx==0 && $offy==0 && $angle==0 && $scale==1);


  my @s=split(/\./,$file);	#find extension
  my $ext=$s[-1];

  $offx=$offx/3600.;
  $offy=$offy/3600.;


  my ($crval1, $crval2, $crpix1, $crpix2, $cd11, $cd12, $cd21, $cd22);

  my %fh;
  tie (%fh, 'Tie::IxHash');	#this header will be written later, so we need to preserve order

  if ($ext eq 'ldac') {

    #This should be removed  (simply to read initial header without the fits file)
    # !!!!!!!!!!!!!!

    my @fields=('header','CRPIX1','CRPIX2','CRVAL1','CRVAL2','CD1_1','CD1_2','CD2_1','CD2_2') ;
    ($crpix1, $crpix2, $crval1, $crval2, $cd11, $cd12, $cd21, $cd22) = read_ldac($file, 0, @fields);

    print "Read header $file: $crpix1, $crpix2, $crval1, $crval2, $cd11, $cd12, $cd21, $cd22 \n";
  }
  elsif ($ext eq 'fits') {
    %fh=rhead($file);

    $crval1=$fh{CRVAL1};
    $crval2=$fh{CRVAL2};
    $crpix1=$fh{CRPIX1};
    $crpix2=$fh{CRPIX2};
    $cd11  =$fh{CD1_1};
    $cd21  =$fh{CD2_1};
    $cd12  =$fh{CD1_2};
    $cd22 =  $fh{CD2_2};
  }
  elsif ($ext eq 'hdr' || $ext eq 'head' ) {
    %fh=rheadasc($file);

    $crval1=$fh{CRVAL1};
    $crval2=$fh{CRVAL2};
    $crpix1=$fh{CRPIX1};
    $crpix2=$fh{CRPIX2};
    $cd11  =$fh{CD1_1};
    $cd21  =$fh{CD2_1};
    $cd12  =$fh{CD1_2};
    $cd22 =  $fh{CD2_2};
  }

  if ($KEY ne '' && exists $fh{$KEY}) {my $s="CHGWCS: keyword $KEY found in $file!"; return $s}

  #  print "Calling mkwcs with\n $crpix1, $crpix2, $crval1, $crval2, $cd11, $cd12, $cd21, $cd22, $offx, $offy, $angle, $scale\n";

  ($crpix1, $crpix2, $cd11, $cd12, $cd21, $cd22)=mkwcs($crpix1, $crpix2, $cd11, $cd12, $cd21, $cd22, $offx, $offy, $angle, $scale);


  $fh{CRVAL1}=$crval1;
  $fh{CRVAL2}=$crval2;
  $fh{CRPIX1}=$crpix1;
  $fh{CRPIX2}=$crpix2;
  $fh{CD1_1}=$cd11;
  $fh{CD2_1}=$cd21;
  $fh{CD1_2}=$cd12;
  $fh{CD2_2}=$cd22;

  #write to header
  my $hfile=$file;
  $hfile =~ s/\.$ext/$HDRSUF/;

  #if (-e $hfile) {system ("rm $hfile")}
  if ($KEY ne '') {$fh{$KEY}="\'$KVAL\'"};
  my $ret=wheadasc($hfile, \%fh, undef, "ASTROM", "CHGWCS");

   return;

 # return ($cd11, $cd12, $cd21, $cd22, $crpix1, $crpix2);

}				#end sub chgwcs


sub dofcorr {

  my ($a, $b, $domag, $lab, $itype, $dowhat, $start, @invals)= @_;

  my ($angle, $cscale, $xfmax, $yfmax);
  my ($r1, $r2, $r3);

  my ($amap, $bmap);

  #The Fourier image will be convolved with this kernel
  my $kern=ones(short,3,3);
  #my $kern=short[[0,0,0],[0,1,0],[0,0,0]] ;
  #my $kern=short[[1,2,1],[2,4,2],[1,2,1]] ;
  #my $kern=-ones(3,3);
  #set $kern ,1,1,8;
  #my $kern=-1*short[[0,1,0],[1,-4,1],[0,1,0]] ;
  #------------------------------------------------
  #my $i13=1./3.;
  #my $i23=2./3.;
  #my $i43=4./3.;
  #my $kern=-1*float[[$i23,-$i13,$i23],[-$i13,-$i43,-$i13],[$i23,-$i13,$i23]] ;
  #------------------------------------------------


  my ($fac, $mdig);
  if ($itype eq 'ast') {
    $fac=1.;
    $mdig=10;
  }  else  {
    $fac=1.;
    $mdig=9;
  }

  my $prec=.5/$fac;

  my $xa=$a->slice(':,0');
  my $ya=$a->slice(':,1');

  if ($start) {
    #apply a given starting solution

    my $cscale=1;

    if ($start =~ 'off') {
      #Apply the offset given by the user
      $xa += $invals[0];
      $ya += $invals[1];
    }

    if ($start =~ 'rot') {
      #Apply the rotation given by the user
      my $txa=$a->slice(':,0')->copy ;
      my $tya=$a->slice(':,1')->copy ;

      #my $offx=(max($txa)+min($txa))/2.;
      #my $offy=(max($tya)+min($tya))/2.;

      #$offx=0; $offy=0;
      # $txa -= $offx;
      # $tya -= $offy;

      my $rang=$invals[$#invals]*3.14159265/180.;
      my $cosx=cos($rang);
      my $sinx=sin($rang);

      #Ok, now rotate and backsubstitute in the original pdl
      $xa .= $cscale * ($txa * $cosx - $tya * $sinx);
      $ya .= $cscale * ($txa * $sinx + $tya * $cosx);

      #      $xa .= $cscale * ($txa * $cosx - $tya * $sinx) + $offx;
      #      $ya .= $cscale * ($txa * $sinx + $tya * $cosx) + $offy;

    }


  }

  if ($dowhat eq 'r') {

    # 1) Determine scaling and rotation

    $mdig=11;

    my $bang=0;
    my $bscal=0;

    my $min = float[0.,0.] ;
    my $max= float[0.,0.] ;

    my ($amap, $amin, $amax) = Mylib::nFtrans::map_angdist($a, $min, $max, $bang, $bscal,0);
    my ($bmap, $bmin, $bmax) = Mylib::nFtrans::map_angdist($b, $min, $max, $bang, $bscal,0);


    #    if ($amap->isempty || $bmap->isempty) {print "ERROR: something wrong in fcorr \n"; die}

    $min=float[0,, min(float[$amin->at(1),$bmin->at(1)])];
    $max=float[180, max(float[$amax->at(1),$bmax->at(1)])];


    my $res_ang=( ($max->at(0) - $min->at(0)) )*2. ;
    my $res_scal= ( ($max->at(1) - $min->at(1)) )*2.;

    my $prec_ang=0.1;
    my $prec_scal=0.01;

    my $sbang= int(log($res_ang/$prec_ang)/log(2.)+.5);
    my $sbscal= int(log($res_scal/$prec_scal)/log(2.)+.5);

    $bang=2**min(float[$mdig,$sbang]);
    $bscal=2**min(float[$mdig,$sbscal]);


    $res_ang /= $bang;
    $res_scal /=$bscal;

    my ($tmin, $tmax);

    ($amap, $tmin, $tmax) = Mylib::nFtrans::map_angdist($a, $min, $max, $bang, $bscal,1);
    ($bmap, $tmin, $tmax) = Mylib::nFtrans::map_angdist($b, $min, $max, $bang, $bscal,1);

    if ($amap->isempty || $bmap->isempty) {print "ERROR: something wrong in fcorr \n"; return}

    my ($ix, $iy, $nsig)=Mylib::nFtrans::fcorr($amap,$bmap,$kern, $bang, $bscal);



    my $xfmax = $ix*$res_ang;
    my  $yfmax = $iy*$res_scal;
    my $cscale=exp($yfmax);

    my $tlog=sprintf " >>> angle = %.2f (%.2f), scale = $cscale (-> 1)\n", $xfmax, $res_ang, $cscale;


    #if ($cscale != 1) {plog ("  WARNING: scale = $cscale, set to 1 !!!!\n");$cscale=1 }


    $r1=$xfmax; $r2=$cscale; $r3=$nsig;



  }


  #now compute the shift
  #compute the extremes in each array.....

  if ($dowhat =~ 's') {
    if ($dowhat eq 's+') {$mdig=11;}
    elsif  ($dowhat eq 's-') {$mdig=8}
    elsif  ($dowhat eq 's++') {$mdig=12}
    else {$mdig=10}

    #$mdig-=1;   #TEST ONLY

    my $amin=minimum($a);
    my $amax=maximum($a);
    my $bmin=minimum($b);
    my $bmax=maximum($b);

    #... and the minimum/maximum for both arrays

    my $min=float[min(float[$amin->at(0),$bmin->at(0)]), min(float[$amin->at(1),$bmin->at(1)])];
    my $max=float[max(float[$amax->at(0),$bmax->at(0)]), max(float[$amax->at(1),$bmax->at(1)])];

    #compute the array dimension to get an accuracy of x=>0.5, y=>0.5
    # the factor 2 is required because of binning


    my $res_x = ( ($max->at(0) - $min->at(0)) )*2. ;
    my $res_y = ( ($max->at(1) - $min->at(1)) )*2. ;

    #print "$res_x $res_y \n";

    my $sbx= int(log($res_x/$prec)/log(2.)+.5);
    my $sby= int(log($res_y/$prec)/log(2.)+.5);

    #the maximum size allowed is 1024x1024
    my $bx=2**min(float[$mdig,$sbx]);
    my $by=2**min(float[$mdig,$sby]);

    #The true resolution
    $res_x /= $bx;
    $res_y /= $by;



    #build the image

    $amap = Mylib::nFtrans::map_dist($a,$min,$max, $bx, $by);
    $bmap = Mylib::nFtrans::map_dist($b,$min,$max, $bx, $by);
    #$amap=conv2d $amap, $kern;
    #$bmap=conv2d $amap, $kern;

    #print "$amin - $amax - $bmin - $bmax => $min - $max - $bx -$by\n";

    #make correlation + smooth (inplace, result will be in $amap)
    #print "$amap $bmap\n";

   my ($ix, $iy, $nsig)=Mylib::nFtrans::fcorr($amap,$bmap, $kern, $bx, $by);

    #print  "maximum at: $ix $iy ($nsig sigma)\n";

    #rescale values
    my $xfmax = $ix*$res_x;
    my $yfmax = $iy*$res_y;


    #Applying the shift !

    $xa += $xfmax;
    $ya += $yfmax;

    $r1=$xfmax; $r2=$yfmax; $r3=$nsig;

  }


  return ($r1, $r2, $r3);

}				#end dofcorr



sub getOffset
  {

    my ($rxproj, $ryproj, $xproj, $yproj, $nccd) = @_;

    my $tgcat=cat $xproj, $yproj;
    my $rcat=cat $rxproj, $ryproj;


    my ($xshift, $yshift, $nsig);

    if ($nccd !=1) {
       ($xshift, $yshift, $nsig)=dofcorr ($tgcat, $rcat, 'n', '', '', 's+');
    $xproj+=$xshift;
       $yproj+=$yshift;
       print "Fourier finds $xshift $yshift ($nsig)\n" if ($DEBUG);
     } else {$xshift=0; $yshift=0}



    my ($xi, $m)=Mylib::Astrom::matchCats($rxproj, $ryproj, $xproj, $yproj, 5);
    my $offx=median($rxproj->index($xi)-$xproj->index($m));
    my $offy=median($ryproj->index($xi)-$yproj->index($m));


    $xproj+=$offx;
    $yproj+=$offy;

    $xshift+=$offx;
    $yshift+=$offy;


    return ($xshift, $yshift);

  }				#end sub getOffset

sub four_get_offset
  {
    # Will compute offsets/angles in a list of images
    my $opt = pop @_ if ref($_[$#_]) eq 'HASH';
    my $ref=$opt->{RFRAME};
    my $nccd=$opt->{NCCD};
    my $refcoor=$opt->{ASTCAT};
    my @fcoord=@{$opt->{CATS}};
    my $angle0=$opt->{ANGLE};
    my $REJANG=$opt->{REJANGLE};
    my $DELTANG=$opt->{DELTANGLE};

    my $labplot="kcorr.ps";
    $labplot=$opt->{LABEL} if (exists $opt->{LABEL});


print ">>>>>>>>>>>>>>>> $labplot\n";

    my @fits=@{$opt->{FITS}};


    my $STEP="KCALC";

    $nccd--;

    my $i;
    my ($xshift, $yshift);

    my $nchip=zeroes(short, $nccd+1);
    my $noffx=zeroes(float, $nccd+1);
    my $noffy=zeroes(float, $nccd+1);
    my $pangle=zeroes(float, $nccd+1);
    my $pscale=ones(float, $nccd+1);

    my $cd=zeroes(float,6, $nccd+1);


    print "====  Computing k-table from the reference image ....\n";

    pgbegin(0,"$labplot/PS",1,1);

    my $domag=0;

    #my ($orefa, $orefd, $omag)=read_asc($refcoor,'');
    my ($orefa, $orefd)=read_asc($refcoor,'');

    #print $refcoor, "   ",min($orefa),"   ", max($orefd),"\n";


    #Convert to arcsec
    my $tc;

    my $iter;

    my ($n, $p, $s) = fileparse($ref, '.fits');
    my $nref=$n;

    my @rcoos=grep(/\Q$nref\E/,@fcoord);
    my @rfits=grep(/\Q$nref\E/,@fits);	

    my @bad=();

  ITER: foreach $iter (1..2) {

    my ($crval1, $crval2);

    CCD: foreach $i (0..$nccd) {
	
        $bad[$i]=0;

	my $j=$i+1;
	my $im=$ref;
	
	
	my $obscoor= $rcoos[$i];
        my $rfits=$rfits[$i];
	
      	#Find the shift, do the plot
	my $lab='';
	
	#read the files
			
        #my @fields=('X_IMAGE','Y_IMAGE','MAG_AUTO');	
	#my ($px, $py, $mag) = read_ldac($obscoor, 0, @fields);

#warn "reading files.....";

        my @fields=('X_IMAGE','Y_IMAGE');
        my ($px, $py) = read_ldac($obscoor, 0, @fields);


#print $obscoor, "  ",ref($px),"\n";

	my %fh=rheadasc($rfits);
	my $hccd=$i;
	$hccd=$fh{IMAGEID} if (exists $fh{IMAGEID});

	if (!exists $fh{BADCCD}) {
          $fh{BADCCD}=0
        }

	my $crpix1=$fh{CRPIX1};
	my $crpix2=$fh{CRPIX2};
	#my $crval1=$fh{CRVAL1};
	#my $crval2=$fh{CRVAL2};

        if ($i==0) {
	    $crval1=$fh{CRVAL1};
	    $crval2=$fh{CRVAL2};
	  }


	my $cd11=0;
	$cd11=$fh{CD1_1} if (exists $fh{CD1_1});
	my $cd12=0;
	$cd12=$fh{CD1_2} if (exists $fh{CD1_2});
	my $cd21=0;
	$cd21=$fh{CD2_1} if (exists $fh{CD2_1});
	my $cd22=0;
	$cd22=$fh{CD2_2} if (exists $fh{CD2_2});
	my $equinox=2000;
	$equinox=$fh{EQUINOX} if (exists $fh{EQUINOX});
	
	
	#print "$equinox $crval1 $crval2 $crpix1 $crpix2 $cd11 $cd12 $cd21 $cd22\n";
	
	
	if ($cd11==0 && $cd22==0 && $cd12==0 && $cd21==0) {
	  plog ("W", $STEP, "CDij not found in header, looking for CDELTi");
	
	  my @tf=('header', 'CDELT1', 'CDELT2');
	  ($cd11, $cd22)=read_ldac($obscoor, 0, @tf);
         $cd11=$cd11->at(0);
	 $cd22=$cd22->at(0);
         if  (exists $fh{CROTA1}) {$angle0 = $fh{CROTA1}}
	};
	
	if ($DEBUG) {print "Warning: Read LDAC header $obscoor: $crpix1, $crpix2, $crval1, $crval2, $cd11, $cd12, $cd21, $cd22, $equinox\n"}
	
	 my $tcd=$cd->slice(":,$i");
         $tcd .= float($crpix1, $crpix2, $cd11, $cd12, $cd21, $cd22);	

	if ($angle0 ne 'off' && $angle0 ne 0) {
	  ($crpix1, $crpix2, $cd11, $cd12, $cd21, $cd22)=mkwcs($crpix1, $crpix2, $cd11, $cd12, $cd21, $cd22, 0, 0, $angle0, 1);
	}
		
	
	# Should precess coordinates ?
	if ($equinox !=2000) {
	  ($crval1, $crval2)=precess($crval1/15.,$crval2, $equinox,2000.);
	  $crval1*=15.;
	}


#warn "==================> $rfits $hccd\n";

	if ((ref($px) ne 'PDL') || ($fh{BADCCD} == 1)) {
	  $bad[$i]=1;
               if ($iter==1) {
                   plog ("W", $STEP, "$obscoor: no sources found or BADCCD!!");
                   set $pangle, $i, 0;
		   set $pscale, $i, 1;
                 }
	       elsif ($iter==2) {
		 set ($nchip, $i ,$hccd);
		 set ($noffx, $i ,0);
		 set ($noffy, $i ,0);
		 next CCD
	       }

           }
      else {
           if ($iter==1) {
	         my $s=sprintf "$obscoor: found %d sources", $px->nelem;
                 plog ("I", $STEP, $s);
	       }
        }	

     my ($a, $lcat);


     if ($bad[$i]!=1) {
	
	#Transforming to projected coordinates....
	
	$px-=$crpix1;
	$py-=$crpix2;
	
	my $xproj = $cd11*$px + $cd12*$py;
	my $yproj = $cd21*$px + $cd22*$py;
	
	$px = ($xproj*3600);
	$py = ($yproj*3600);

	#my $a=cat $px, $py, $mag;		
	#my $lcat=cat $orefa, $orefd, $omag;
	
 	$a=cat $px, $py;
        $lcat=cat $orefa, $orefd;
	
	my $refa=$lcat->slice(':,(0)');
	my $refd=$lcat->slice(':,(1)');
	
	my ($prefx, $prefy) = Mylib::Astrom::celfwd($refa, $refd, $crval1, $crval2);

	$refa.=($prefx*3600.);  #backval to $lcat
	$refd.=($prefy*3600.);

       }	
	
      # Start the first iteration
	
	if ($iter==1) {
	
	      if ($bad[$i]!=1) {
	
	  my $tlog;
	
#	  if ($i ==0) {plog ("=======================\n", "FIRST RUN\n", "=======================\n")}
	  plog ("I", $STEP, "Registration of $obscoor vs. $refcoor",'First Run');
	
	  #first call, compute only a VERY first approximation of shift
	  #in order to approximately match the two catalogs
	
	  my $ptol=0.;	##########
	
	
	  $b = xsubcat($a, $lcat, $ptol); #"cut" a subcatalog from the reference catalog

	
	  if ($b->nelem < 10) {
	      plog ("I", $STEP, "No overlap!!", 'First Run');
	      $bad[$i]=1;
          } else {
	
	    my $ptol=600.;	##########
	
	
	    $b = xsubcat($a, $lcat, $ptol); #"cut" a subcatalog from the reference catalog
  	    #magmatch($a,$b);	#match the magnitudes
  	
  	
  	    my ($xshift, $yshift, $nsig)=dofcorr ($a, $b, $domag, '', '', 's');
  	
  	
  	    $tlog=sprintf " >>> xshift = %.2f, yshift = %.2f (sigma = %.0f)", $xshift, $yshift, $nsig;
  	    plog ("I", $STEP,$tlog);
  	
  	    #now get rotation and scaling factor (set to 1 for now....)
  	    #my $lab="CCD $j";
  	    $ptol=60.;		##########
  	    $b = xsubcat($a, $b, $ptol);
  	
  	
  	    my ($angle, $scale, $nsig)=dofcorr ($a, $b, $domag, '', '', 'r');
  	
  	    $tlog=sprintf " >>> angle = %.2f, scale = %.3f  (sigma = %.0f)",
  	    $angle, $scale, $nsig;
  	    plog ("I", $STEP,$tlog);
  	
  	
  	    set $pangle, $i, $angle;
  	
  	    $scale=1;		#     <-------------- To BE CHECKED !!!
  	
  	    set $pscale, $i, $scale;
          }

        } # end if ccd not bad

	
	  if ($i == $nccd) {my ($idx, $m, $tmp);
	
			    $m=median($pangle);
			
			    my $adev=abs($pangle-$m);
			    my $asig=sqrt(sum($adev*$adev)/($adev->nelem));
			    $idx=which($adev > $REJANG*$asig | $adev > $DELTANG);

#print "$pangle $m $adev $asig  $REJANG $DELTANG $idx\n" ;			
			    if (! $idx->isempty) {
			         ($tmp=$pangle->index($idx)) .= $m;
			     }
			    $m=median($pscale);
			    ($tmp=$pscale) .= $m;
			  }	
	
	}
	
	if ($iter==2) {
	
	  #goto DOFIT;
	
	  my $tlog;
	  my $lab="CCD $j";
	
#	  if ($i ==0) {plog ("=======================\n", "SECOND RUN\n", "=======================\n")}
	  plog ("I", $STEP,"Registration of $obscoor vs. $refcoor",'Second Run');
	
	  # recompute the shift AFTER the angle is applied
	  my $ptol=600.;	##########
	
	  $b = xsubcat($a, $lcat, $ptol); #"cut" a subcatalog from the reference catalog
	  #magmatch($a,$b);	#match the magnitudes
	
	  #Apply the rotation and recompute the offset
	  my $lab0="CCD $j a";
	  my ($xshift, $yshift, $nsig)=dofcorr ($a, $b, $domag, $lab0, '', 's','rot', $pangle->at($i));
	
	  $tlog=sprintf " >>> xshift = %.2f, yshift = %.2f (sigma = %.0f)", $xshift, $yshift, $nsig ;
	  plog ("I", $STEP,$tlog);
	
	
	  #now compute the offset with higher accuracy, matching catalogs
	  my ($refa, $refd) = dog $b;
	  my ($xproj, $yproj) = dog $a;
	  my ($xi, $m)=Mylib::Astrom::matchCats($refa, $refd, $xproj, $yproj, 5);
          my $offx=($refa->index($xi)-$xproj->index($m));
	  my $offy=($refd->index($xi)-$yproj->index($m));

	  my $moffx=median($offx);
	  my $moffy=median($offy);
	
	  my $dox=$offx-$moffx; my $doy=$offy-$moffy;

	  my $sigx=sqrt(sum($dox*$dox)/$dox->nelem);
	  my $sigy=sqrt(sum($doy*$doy)/$doy->nelem);


           my $s=sprintf "$obscoor: %d sources matched (%.2f+/-%.2f, %.2f+/-%.2f)",
$xi->nelem, $moffx, $sigx, $moffy, $sigy;
                 plog ("I", $STEP, $s);
	
	  $xshift+=$moffx;
	  $yshift+=$moffy;
	
	  $tlog=sprintf " >>>F.R. xshift = %.2f, yshift = %.2f", $xshift, $yshift ;
	  plog ("I", $STEP,$tlog);
	
	  $xproj+=$moffx; $yproj+=$moffy;
	
	  $xproj=float($xproj); $yproj=float($yproj);
	  $refa=float($refa); $refd=float($refd);
	
	  $lab=sprintf "CCD %d:  \\gD\\ga = %.4f \\gD\\gd= %.4f",$hccd, $xshift, $yshift;
	  pgenv (min($xproj), max($xproj), min($yproj), max($yproj),0,1);
	  pglab('\ga','\gd',$lab);
	  pgpoint(nelem($xproj),$xproj->get_dataref, $yproj->get_dataref,4);
	  pgpoint(nelem($refa),$refa->get_dataref, $refd->get_dataref,2);
	
	DOFIT:
	
	  #Store the values
	  set ($nchip, $i ,$hccd);
	  set ($noffx, $i ,$xshift);
	  set ($noffy, $i ,$yshift);


	}			#end if iter==2
	

	
      }				#end loop on CCDs



    }				#end loop on iter





    pgend();


    if ($angle0 ne 'off' && $angle0 ne 0) {$pangle+=$angle0}


     my $bad=short(@bad);
     my $good=which($bad==0);

     if ($good->isempty) {plog ("E", $STEP,'All images are bad ?????????');}

#print " =======> $nccd/", $good->dims,"\n";

       my $ibad=which($bad==1);
       if (! $ibad->isempty) {
	 foreach my $j (0..$ibad->nelem-1) {
           my $s= sprintf "CCD %d flagged as BAD !", $ibad->at($j);
              plog ("W", $STEP,$s);
         }
          my $t;
          $t=$noffx->index($ibad) .= avg($noffx->index($good));
          $t=$noffy->index($ibad) .= avg($noffy->index($good));
          $t=$pangle->index($ibad) .= avg($pangle->index($good));
          $t=$pscale->index($ibad) .= avg($pscale->index($good));
        }


     foreach $i (0..$nccd) {
           #next if ($bad[$i]==1);
           my $tcd=$cd->slice(":,$i");
           my @cd=list $tcd;
	   my @ncd=mkwcs(@cd, $noffx->at($i)/3600, $noffy->at($i)/3600, $pangle->at($i), $pscale->at($i));
           $tcd.=float(@ncd);
     }

    system ("gzip -f $labplot") if (-e $labplot);

    return ($nchip, $noffx, $noffy, $pangle, $pscale, $cd)


  }				#end sub four_get_offset

sub read_CDS
{
    #-------------------------------------------------------------------
    # 1) Read  catalogs from CDS

    my ($mode, $findpmm, $refcoor, $RA, $DEC, $radius, $xmax) = @_;
    my $ret  = -1;
    my @s    = split(/\//, $refcoor);
    my $wcat = $s[-1];

    unlink $refcoor if ($mode eq "REDO" && -e $refcoor);

    if (!-e $refcoor || -z $refcoor)
    {
        if ($radius eq '') {$radius=40}
        if ($xmax eq '')   {$xmax=99999}; # maximun number of objects to be queried
        # Search radius (arcmin)
        my $ch_cat;

        print "=== Reading the $wcat catalogue from CDS ... ra=$RA dec=$DEC radius=$radius max=$xmax\n ";

        plog ("I", $wcat, "Field coordinates: $RA - $DEC");

	# convert Ra and Dec to sexagesimal if necessary.
	if($wcat !~ /2mass/i)
        {
	    $RA=Mylib::Astrom::sexag($RA / 15.);
	    $DEC=Mylib::Astrom::sexag($DEC, '\sig');
        }

        my $opt = $RA." ".$DEC;
        $opt=$opt.' -r '.$radius;
        $opt=$opt.' -m '.$xmax;

        if ($wcat =~ /usnob1/i) { $opt=$opt.' -smr '; }
        if ($wcat =~ /^ucac2/i) { $opt.=' -eis'; }

        my $tmp=$refcoor.'tmp';

        system ($findpmm.$opt." > $tmp");
        print $findpmm.$opt." > $tmp";

        open (FILE, "<$tmp");
        open (OUT, ">$refcoor");

        my @cat_ent=<FILE>;

        close(FILE);

        foreach my $cat_ent (@cat_ent)
        {
	    my ($cid, $cra, $cdec, $magb, $magr, $craerr, $cdecerr, $ceu);

	    if (($cat_ent !~ /^#|=/) && ($cat_ent !~ /^$/))
            {
	        if ($wcat =~ /^usnob1/i)
                {
		    my ($t1, $t2, $t3, $t4) = unpack 'A26 A21 A81 A4', $cat_ent;
		
		    $magr = $t4;
		
		    $t2 =~ /(\S+)([\+\-])(\S+)/;
		    my $bcra = $1;
		    my $bcdec = $2.$3;

		    if ($magr !~ /\d/) { $magr=99.9; }

		    $cra = Mylib::Astrom::sexag($bcra/15.);
		    $cdec = Mylib::Astrom::sexag($bcdec,'\sig');

		    $cra = " ".$cra;
		    $magr = " ".$magr;
	        }
                # USNO means the USNO-A2 catalogue. For the moment we do not change
                # this parameter name to be compatible with old pipeline scripts that
                # still use USNO. Probably we should change it to USNOA2 anyway!
		elsif ($wcat =~ /^usno/i)
                {
		    ($cid, $cra, $cdec, $magb, $magr)=unpack 'A13 A13 A13 A6 A6', $cat_ent;
		}

                if ($wcat =~ /^ucac2/i)
                {		
	            ($cid, $cra, $cdec, $craerr, $cdecerr, $ceu, $magr)=unpack 'A9 A13 A13 A4 A4 A5 A5', $cat_ent;
 	        }

	        if ($wcat =~ /^gsc2/i)
                {
	            my @t=split(/\s+/, $cat_ent);
	            $cra=Mylib::Astrom::sexag($t[1]/15.);
	            $cdec=Mylib::Astrom::sexag($t[2],'\sig');
	            ($cid, $magr)=@t[0,6];

                    if ($magr !~ /\d/) { $magr=99.9; }
	        }

                if ($wcat =~ /^sdss/i)
                {
                    $cat_ent =~ /\A\s*(\S.*)/;
                    my @list = split(/\s+/,$1);
                    my $t2, $t3;

	            if ($wcat =~ /^sdss3/i)
                    {
                        $t2 = $list[3];
                        $t3 = $list[8];
                    }
	            elsif ($wcat =~ /^sdss4/i || $wcat =~ /^sdss5/i)
                    {
                        $t2 = $list[4];
                        $t3 = $list[12];
                    }

                    # Format magnitude
                    $t3 =~ /(.*)\`/;
                    $magr = $1;
                    if ($magr !~ /\d/) { $magr=99.9; }

                    # Format RA/DEC
                    $t2 =~ /(\S+)([\+\-])(\S+)/;
                    my $bcra = $1;
                    my $bcdec = $2.$3;

                    $cra = Mylib::Astrom::sexag($bcra/15.);
                    $cdec = Mylib::Astrom::sexag($bcdec,'\sig');

                    $cra = " ".$cra;
                    $magr = " ".$magr;
                }

		if ($wcat =~ /^2mass/i)
                {
                    $cat_ent =~ /\A\s*(\S.*)/;
                    my @list = split(/\s+/,$1);

                    my $bcra = $list[0];
                    my $bcdec = $list[1];
		
		    $cra = Mylib::Astrom::sexag($bcra/15.);
                    $cdec = Mylib::Astrom::sexag($bcdec,'\sig');
		
                    # Format magnitude
		    my $t3 = $list[6];
                    $magr = $t3;
                    if ($magr !~ /\d/) { $magr=99.9; }

		    $cra = " ".$cra;
                    $magr = " ".$magr;
                }

                if($magr < 20.0)
                {
	            print OUT "$cra $cdec $magr\n";
                }
            }
        }

        close(OUT);

        $ret=1;

    }
    else
    {
        $ret=0; print "  $refcoor catalogue already extracted - skipping !\n"
    }


    if (-z "$refcoor") {print "ERROR zero size file returned for $refcoor....\nERROR\n";die "\n"}

    return $ret;

}				#end sub read_CDS


1;
