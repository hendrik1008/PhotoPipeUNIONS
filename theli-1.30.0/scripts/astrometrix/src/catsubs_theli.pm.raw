# HISTORY COMMENTS for changements within the
# GaBoDS pipeline context. The original file
# is from v1.2 of Mario Radovich's ASTROMETRIX
#
# 18.07.2005:
#
# changes to account for the new syntax of
# the function read_ldac (see file nldacsub.pm).

use PDL;
use SEXconf;
use strict;
use Mylib::Astrom;

#my $DEBUG=1;
my $DEBUG=0;

sub mkMaster {

# The input must be:
#   Reference to an array of ldac files
#   Reference to root names (without CCD)
#   The number of CCDs

my ($sfiles, $hdrs, $stack, $nccd, $outdir, $nomake)=@_;


my @stack=@$stack;
my @sfiles=@$sfiles;
my @hdrs=@$hdrs;

my @mcats=();


# ------------- FOR EACH SET, COPY ALL THE CCD CATALOGS INTO ONE CATALOG
#                    (do not remove saturated stars for now)


# ====== We also write the index for each set
my $index=$outdir.'/index.db';

open (IDX, ">$index");
print IDX "# Set    Name\n";

 for (my $i=0; $i<=$#stack; $i++) {

   print IDX sprintf "$i  $stack[$i]\n";

    my ($nnr,$nxpx, $nypx, $nxpj, $nypj, $nra, $ndec, $ccd);

    my @xfiles=grep(/$stack[$i]/,@sfiles);
    my @xhdrs=grep(/$stack[$i]/,@hdrs);


    my @s=split(/\//, $xfiles[0]);
    pop @s;
    my $ddir;
    if ($outdir ne '') {$ddir=$outdir} else {$ddir=join '/', @s}

    my $mname=$ddir.'/'.'M'.$stack[$i].'.ldac';
    push @mcats, $mname;

   next if ($nomake==1);

 CCD: for (my $j=0; $j<$nccd; $j++) {


    if ($j>$#xfiles) {last}

   my $ncat=$xfiles[$j];


print "MKMASTER: Adding catalog: $ncat to $mname \n" if ($DEBUG);
#read positions in pixels
     my @fields=('NUMBER','X_IMAGE','Y_IMAGE');	
      my ($number, $xpix, $ypix) = read_ldac($ncat, 0, @fields);


  if ($j==0 ) {
   my $nrows;
    if (ref($number) ne 'PDL') {$nrows=1}
     else { $nrows=$number->nelem}

    $nxpx=zeroes(float,$nrows);
    $nypx =zeroes(float,$nrows);
    $nxpj=zeroes(float,$nrows);
    $nypj =zeroes(float,$nrows);
    $nra=zeroes(float,$nrows);
    $ndec=zeroes(float,$nrows);
    $nnr=zeroes(long, $nrows);
    $ccd= zeroes(byte,$nrows);
  }



#convert pixels into projected coordinates  (arcsec)
    my $head = $xhdrs[$j];

print "  reading $head\n" if ($DEBUG);

    if (! -e $head) {
        print "MKMASTER: header $head not found !";
	next;
     }

    my %fh=rheadasc($head);

    my $hccd=$fh{IMAGEID};

    my $crval1=$fh{CRVAL1};
    my $crval2=$fh{CRVAL2};
    my $crpix1=$fh{CRPIX1};
    my $crpix2=$fh{CRPIX2};


 if (ref($number) ne 'PDL') {
    my $tmp;
  if ($j==0) {
    my $tmp;
   ($tmp = $nxpx) .= 0;
   ($tmp = $nypx) .= 0;
   ($tmp = $nxpj) .= 0;
   ($tmp = $nypj) .= 0;
   ($tmp = $nra) .= 0;
   ($tmp = $ndec) .= 0;
    ($tmp = $nnr) .= -1;
   ($tmp = $ccd) .= $hccd;
   } else {
     my $to= $nxpj->nelem-1;
     my $xto=$to+1;
      my $dim=$nxpj->nelem+1;

      my $tmp;
      my $cp;

      $cp = $nxpx;
      $nxpx=zeroes(float,$dim);
      ($tmp = $nxpx->slice("0:$to")) .= $cp ;
      ($tmp = $nxpx->slice("$xto:$xto")) .= 0;

        $cp = $nypx;
        $nypx=zeroes(float,$dim);
         ($tmp = $nypx->slice("0:$to")) .= $cp ;
         ($tmp = $nypx->slice("$xto:$xto")) .= 0;

        $cp = $nxpj;
        $nxpj=zeroes(float,$dim);
        ($tmp = $nxpj->slice("0:$to")) .= $cp ;
        ($tmp = $nxpj->slice("$xto:$xto")) .= 0;

        $cp = $nypj;
        $nypj=zeroes(float,$dim);
         ($tmp = $nypj->slice("0:$to")) .= $cp ;
         ($tmp = $nypj->slice("$xto:$xto")) .= 0;

        $cp = $nra;
        $nra=zeroes(float,$dim);
         ($tmp = $nra->slice("0:$to")) .= $cp ;
         ($tmp = $nra->slice("$xto:$xto")) .= 0;

        $cp = $ndec;
        $ndec=zeroes(float,$dim);
         ($tmp = $ndec->slice("0:$to")) .= $cp ;
         ($tmp = $ndec->slice("$xto:$xto")) .= 0;

        $cp = $nnr;
        $nnr=zeroes(long,$dim);
         ($tmp = $nnr->slice("0:$to")) .= $cp ;
         ($tmp = $nnr->slice("$xto:$xto")) .= -1;

        $cp = $ccd;
        $ccd=zeroes(byte,$dim);
        ($tmp = $ccd->slice("0:$to")) .= $cp ;
        ($tmp = $ccd->slice("$xto:$xto")) .= $hccd;

   }
   next CCD;
    }



    my ($cd11, $cd12, $cd21, $cd22);

	    if (exists $fh{CD1_1}) {$cd11 = $fh{CD1_1}}
	    elsif (exists $fh{CDELT1}) {$cd11 = $fh{CDELT1}}
	    else {die "MKMASTER: no CD1_1/CDELT1 found in $head !!\n"}

	    if (exists $fh{CD2_2}) {$cd22 = $fh{CD2_2}}
	    elsif (exists $fh{CDELT2}) {$cd22 = $fh{CDELT2}}
	    else {die "MKMASTER: no CD2_2/CDELT2 found in $head !!\n"}

	    if (exists $fh{CD2_1}) {$cd21 = $fh{CD2_1}}
	    else {$cd21=0}

	    if (exists $fh{CD1_2}) {$cd12 = $fh{CD1_2}}
	    else {$cd12=0}


#print "### $crval1 $crval2  $crpix1 $crpix2 $cd11 $cd21 $cd12  $cd22\n";

   my $zxpix = $xpix - $crpix1;
   my $zypix = $ypix - $crpix2;
   my $xproj = ($cd11 * $zxpix + $cd12*$zypix);
   my $yproj =  ($cd21 * $zxpix + $cd22*$zypix);

# NEW #######################################
# apply a plate solution if found

    if (exists $fh{PSORD}) {
          my $h_distord=$fh{PSORD};
	  my $h_ncoeffs=&Mylib::Astrom::calcNcoeffs($h_distord);
	
	  my $pv1=zeroes(double, $h_ncoeffs);
	  my $pv2=zeroes(double, $h_ncoeffs);

	      if (exists $fh{PV1_0}) {
                &Mylib::Astrom::ReadPV($pv1, \%fh, ,'PV1', $h_ncoeffs );
                &Mylib::Astrom::ReadPV($pv2, \%fh, ,'PV2', $h_ncoeffs );
	
		  if (! $pv1->isempty) {
		  my $tx=&Mylib::Astrom::pveval($pv1, $xproj, $yproj, $h_distord);
		  my $ty=&Mylib::Astrom::pveval($pv2, $yproj, $xproj, $h_distord);
		  $xproj=$tx->copy;
		  $yproj=$ty->copy;
                  }
	      }

    }

#############################################


   my ($ra, $dec) =  Mylib::Astrom::celrev($xproj, $yproj, $crval1, $crval2);



   $xproj*=3600; $yproj*=3600;
   $ra*=3600; $dec*=3600;

#store into the "master" catalog for the current file

    if ($j==0) {
   my $tmp;
   ($tmp = $nxpx) .= $xpix;
   ($tmp = $nypx) .= $ypix;
   ($tmp = $nxpj) .= $xproj;
   ($tmp = $nypj) .= $yproj;
   ($tmp = $nra) .= $ra;
   ($tmp = $ndec) .= $dec;
    ($tmp = $nnr) .= $number;
   ($tmp = $ccd) .= $hccd;

    }
  else {
      my $to= $nxpj->nelem-1;
      my $from=$to+1;
      my $xto= $from+($xproj->nelem)-1;
      my $dim=$nxpj->nelem+$xproj->nelem;


      my $tmp;
      my $cp;

        $cp = $nxpx;
        $nxpx=zeroes(float,$dim);
        ($tmp = $nxpx->slice("0:$to")) .= $cp ;
        ($tmp = $nxpx->slice("$from:$xto")) .= $xpix;

        $cp = $nypx;
        $nypx=zeroes(float,$dim);
         ($tmp = $nypx->slice("0:$to")) .= $cp ;
         ($tmp = $nypx->slice("$from:$xto")) .= $ypix;

        $cp = $nxpj;
        $nxpj=zeroes(float,$dim);
        ($tmp = $nxpj->slice("0:$to")) .= $cp ;
        ($tmp = $nxpj->slice("$from:$xto")) .= $xproj;

        $cp = $nypj;
        $nypj=zeroes(float,$dim);
         ($tmp = $nypj->slice("0:$to")) .= $cp ;
         ($tmp = $nypj->slice("$from:$xto")) .= $yproj;

        $cp = $nra;
        $nra=zeroes(float,$dim);
         ($tmp = $nra->slice("0:$to")) .= $cp ;
         ($tmp = $nra->slice("$from:$xto")) .= $ra;

        $cp = $ndec;
        $ndec=zeroes(float,$dim);
         ($tmp = $ndec->slice("0:$to")) .= $cp ;
         ($tmp = $ndec->slice("$from:$xto")) .= $dec;

        $cp = $nnr;
        $nnr=zeroes(long,$dim);
         ($tmp = $nnr->slice("0:$to")) .= $cp ;
         ($tmp = $nnr->slice("$from:$xto")) .= $number;

        $cp = $ccd;
        $ccd=zeroes(byte,$dim);
        ($tmp = $ccd->slice("0:$to")) .= $cp ;
        ($tmp = $ccd->slice("$from:$xto")) .= $hccd;

  }

 }

my $n=long[1..$nxpj->getdim(0)];
my $tform = [ qw( 1J 1E 1E 1E 1E 1E 1E 1J 1J) ];
my $ttype = [ qw( NUMBER X_IMAGE Y_IMAGE X_PROJ Y_PROJ ALPHA_J2000 DELTA_J2000 CCD NFOUND) ];
my $tunit = [ ( '', 'pixel', 'pixel', 'arcsec', 'arcsec', 'arcsec', 'arcsec', '','') ];

my $nf=zeroes(byte, $nxpj->nelem);

wldac($mname,$nnr, $nxpx, $nypx, $nxpj, $nypj, $nra, $ndec, $ccd, $nf, $ttype, $tform, $tunit);

} # The end of the big loop.....

close (IDX);


return @mcats;

} #end sub mkMaster



sub CrossCats {
# ------------- CROSS-IDENTIFY COMMON SOURCES IN ALL THE CATALOGS

my $tol=5;
$tol=pop if ($#_>=1);

my $stack=shift;
my @stack=@$stack;



my $scat='db';

my $wclock0=time;
my $cpu0=(times)[0];


my $tform = [ qw( 1J 1E 1E 1E 1E 1E 1E 1J 1J 0J 0J) ];
$tform->[9]= ($#stack+1).'J';    #this will store the position in the set
$tform->[10]= ($#stack+1).'J';    #this will store the index number

my $ttype = [ qw( NUMBER X_IMAGE Y_IMAGE X_PROJ Y_PROJ ALPHA_J2000 DELTA_J2000 CCD NFOUND SCCD SIDX) ];
my $tunit = [ ( '', 'pixel', 'pixel', 'arcsec', 'arcsec', 'arcsec', 'arcsec', '','', '','') ];


  my @fields=('NUMBER','X_IMAGE','Y_IMAGE','X_PROJ','Y_PROJ','ALPHA_J2000','DELTA_J2000',
          'CCD','NFOUND','SCCD','SIDX');	

  my @cfields=('CCD','SET');
  my $cform=[ qw( 1J 0J) ];
  $cform->[1]  =  ($#stack+1).'J';
  my $ctype=[qw(CCD SET)];
  my $cunit=[('','')];

my $init=zeroes(byte,$#stack+1);


 for (my $i=0; $i<=$#stack; $i++) {

    my $rname=$stack[$i];


print "------> Reading $rname\n" if ($DEBUG);

    if (!-e $rname) {print "CROSSCATS: $rname not found !!\n"; next}


      my ($rn, $rxp, $ryp, $rxpr, $rypr, $ra, $rd, $rccd, $rfound, $rsccd, $rsidx)=read_ldac($rname, 0, @fields);
          if ($rsccd->isempty) {$rsccd=-1*ones(short,$rn->nelem,$#stack+1);}
	  if ($rfound->isempty || $init->at($i)==0) {
	
	        $rfound=zeroes(short,$rn->nelem);
		$rsccd=-1*ones(short,$rn->nelem,$#stack+1);
		$rsidx=	zeroes(short,$rn->nelem,$#stack+1);	
              }

   #read the "archive" tables
    my @s=split(/\./, $rname);
    $s[-1]=$scat;
    my $rcatname = join '.', @s;

print "   Reading db: $rcatname\n" if ($DEBUG);

     my ($arccd, $rflag);
     if ($init->at($i)==0) {
          $arccd=short[min($rccd)..max($rccd)];
          $rflag=zeroes(short, $arccd->nelem, $#stack+1);
	   wldac($rcatname, $arccd, $rflag ,$ctype, $cform, $cunit);
       } else {($arccd, $rflag) = read_ldac($rcatname, 0, @cfields)}

   if ($i<$#stack) {
       	
    for (my $j=$i+1; $j<=$#stack; $j++) {

      my $mname=$stack[$j];

print "      Reading cat: $mname\n" if ($DEBUG);
       if (!-e $mname) {print "CROSSCATS: $rname not found !!\n"; next}

        my ($tn,$xp, $yp, $xpr, $ypr, $ca, $cd, $tccd, $tfound, $tsccd, $tsidx) = read_ldac($mname, 0, @fields);
          if ($tsccd->isempty) {$tsccd=-1*ones(short,$tn->nelem, $#stack+1);}
	  if ($tfound->isempty || $init->at($j)==0) {

	          $tfound=zeroes(short,$tn->nelem);
		  $tsccd=-1*ones(short,$tn->nelem, $#stack+1);
                  $tsidx=zeroes(short,$tn->nelem,$#stack+1);	
             }
	

           my @s=split(/\./, $mname);
              $s[-1]=$scat;
           my $catname = join '.', @s;

         my ($atccd, $tflag);
         if ( $init->at($j)==0) {
             $atccd=short[min($tccd)..max($tccd)];
              $tflag=zeroes(short, $atccd->nelem, $#stack+1);
	      wldac($catname, $atccd, $tflag ,$ctype, $cform, $cunit);

          } else {($atccd, $tflag)=read_ldac($catname, 0, @cfields)}

    #find matching points with the given tolerance (here: 5 arcsec)

print "         Cross correlating catalogs\n" if $DEBUG;


# print sprintf "%d %d %f\n", $ra->nelem, $ca->nelem, $tol if $DEBUG;
        my ($xi, $m)=Mylib::Astrom::matchCats($ra, $rd, $ca, $cd, $tol);
#my ($xi, $m, $n)=_matchCats($ra, $rd, $ca, $cd, null, null, $tol, 0);
#print sprintf "%d %d %d\n", $xi->nelem, $m->nelem, $n;
#if ($n>0) {
#   $n--;
#  $xi=$xi->slice("0:$n"); $m=$m->slice("0:$n");
#} else {$xi=null; $m=null}

	next if ($xi->isempty);
	
print "         Found: ", $xi->nelem,"\n" if $DEBUG;


# ------  'REFERENCE' set of images

# a) Increase the counter of overlapping points,

   my $tmp=$rfound->index($xi);
   $tmp++;                 #increase by 1 all the selected elements in $rfound

# .... store the number of the CCD for the "reference" images

    $tmp=$rsccd->slice(":,($i)");
    my $tt=$tmp->index($xi);

    $tt.=$rccd->index($xi);


# .... and the indexes

    $tmp=$rsidx->slice(":,($i)");
    $tt=$tmp->index($xi);
    $tt.=$rn->index($xi);

# .... and for the "target" images
   $tmp=$rsccd->slice(":,($j)");  #extract the columns for the $jth set
   $tt=$tmp->index($xi);      #link to the matched entries
   $tt .= $tccd->index($m);      #backsubstitute the (target) CCD number


   $tmp=$rsidx->slice(":,($j)");
   $tt=$tmp->index($xi);
   $tt.=$tn->index($m);



# write from  the CCDs in target sets containing overlapping points
  for (my $id=0; $id<$arccd->nelem; $id++) {
     my $kccd=which ($tccd->index($m) == $arccd->at($id) );
     if (! $kccd->isempty) {
        set $rflag, $id,$j, 1;	
     }
  }



  for (my $id=0; $id<$arccd->nelem; $id++) {
     my $kccd=which ($rccd->index($xi) == $arccd->at($id) );
     if (! $kccd->isempty) {
	set $tflag, $id,$i ,1;
     }
  }

 # ------  'TARGET' set of images

#b)  Increase the counter of overlapping points,

  $tmp=$tfound->index($m);
  $tmp++;                 #increase by 1 all the selected elements in $tfound

# .... store the number of the CCD for the "reference" images

   $tmp=$tsccd->slice(":,($i)");  #extract the columns for the $ith set
   $tt=$tmp->index($m);      #link to the matched entries
   my $frccd=$rccd->index($xi);
   $tt .= $frccd;      #backsubstitute the (reference) CCD number

   $tmp=$tsidx->slice(":,($i)");
   $tt=$tmp->index($m);
   $tt.=$rn->index($xi);


# Write the 'target' catalog, set the flag
print "      Writing  $mname\n" if ($DEBUG);

 wldac($mname,$tn, $xp, $yp, $xpr, $ypr, $ca, $cd, $tccd,
              $tfound, $tsccd, $tsidx, $ttype, $tform, $tunit);



print "      Writing  $catname\n" if ($DEBUG);	

 wldac($catname, $atccd, $tflag, $ctype, $cform, $cunit);	
	
 set $init, $j, 1;

    }

  }


#write the archive name

print "  Writing $rcatname\n" if ($DEBUG);

  wldac($rcatname, $arccd, $rflag ,$ctype, $cform, $cunit);

# Update the catalog

print "------> Writing  $rname\n" if ($DEBUG);	

  wldac($rname,$rn, $rxp, $ryp, $rxpr, $rypr, $ra, $rd, $rccd,
        $rfound, $rsccd, $rsidx,
          $ttype, $tform, $tunit);

  set $init, $i, 1;
}


# pgend();



my $cpu1=(times)[0];
my $wclock1 = time;

printf "#### The XCORR of cats took: %.2fs (user), %.2fs (CPU)\n", $wclock1-$wclock0,$cpu1-$cpu0;



} #end sub CrossCats

sub xlimCats {

my $limit=pop;
my @stack=@_;

my $tform = [ qw( 1J 1E 1E 1E 1E 1J 1J 0J 0J) ];
$tform->[7]= ($#stack+1).'J';    #this will store the position in the set
$tform->[8]= ($#stack+1).'J';    #this will store the index number

my $ttype = [ qw( NUMBER X_IMAGE Y_IMAGE X_PROJ Y_PROJ CCD NFOUND SCCD SIDX) ];
my $tunit = [ ( '', 'pixel', 'pixel', 'arcsec', 'arcsec', '','','','') ];
my @fields=('NUMBER','X_IMAGE','Y_IMAGE','X_PROJ','Y_PROJ','CCD',
        'NFOUND','SCCD','SIDX');

for (my $i=0; $i<=$#stack; $i++) {
   my $mname=$stack[$i];
   my ($number, $xpix, $ypix, $xproj, $yproj,$ccd, $found,$sccd, $sidx) = read_ldac($mname, 0, @fields);
   my $idx=which($found >= $limit);
   my $nsccd=zeroes(short,$idx->nelem, $#stack+1);
   my $nsidx=zeroes(short,$idx->nelem, $#stack+1);
   for (my $n=0; $n<$idx->nelem; $n++) {
        my $tmp;
	my $i=$idx->at($n);
	
	($tmp=$nsccd->slice("($n),:")).=$sccd->slice("($i),:");
	($tmp=$nsidx->slice("($n),:")).=$sidx->slice("($i),:");
	#print "$n: ",$nsccd->slice("$n,:"), " ", $nsidx->slice("$n,:"),"\n";
   }	
   my @s=split(/\//, $mname);
   $s[-1]='n'.$s[-1];
   my $xmname=join '/', @s;
   wldac($xmname,$number->index($idx), $xpix->index($idx), $ypix->index($idx), $xproj->index($idx), $yproj->index($idx),
                                             $ccd->index($idx), $found->index($idx), $nsccd, $nsidx,					
					     $ttype, $tform, $tunit);

 }
} #end sub xlimCats

#sub  matchCats
##match two catalogs
##INPUT: reference (m,2) and input catalog (n,2), coordinates in arcsecs
##OUTPUT: a (p,2) array giving in the first column the indexes of the reference catalog
##        and in the second column the matching indexes of the second catalog
#{

#my ($msigma, $rm, $tm);
#$msigma=0;
#  if ($#_==7) {
#    ($rm, $tm, $msigma)=splice (@_, -3);
#    $msigma=0 if ($rm->isempty || $tm->isempty);
#   }

#my ($ra, $rd, $ta, $td, $sigma)=@_;

#my @lra=minmax($ra);
#my @lrd=minmax($rd);
#my @lta=minmax($ta);
#my @ltd=minmax($td);


#my $xi=null;
#my $m=null;

#if ( ($lta[0] >= $lra[1] || $lra[0]>= $lta[1]) ||
#     ($ltd[0] >= $ltd[1] || $lrd[0]>= $ltd[1])) {
#               return ($xi, $m);
#}



#     my $idx1=qsorti $ra;
#        $ra=$ra->index($idx1);
#        $rd=$rd->index($idx1);

#     my $idx2=qsorti $ta;
#        $ta=$ta->index($idx2);
#        $td=$td->index($idx2);

#if ($msigma>0) {
#  $rm=$rm->index($idx1);
#  $tm=$tm->index($idx2);
#}

#my $k=0;

#my $dim1=$ra->nelem;
#my $dim2=$ta->nelem;

#my $m=-1*ones(long, $dim1);

#my ($i1, $i2);

#for ($i1=0; $i1<$dim1; $i1++) {
#   my $atol=$sigma;
#   my $dtol=$sigma;
#   my $mtol=$msigma;
#  #print ">>> Comparing ", $ra->at($i1), " ", $rd->at($i1),"\n";
#   for ($i2=$k; $i2<$dim2; $i2++) {
#     #print "with ", $ta->at($i2), " ", $td->at($i2),"\n";
#    #print "$i2 $k $dim1 $dim2\n";

#        my $asig = ( $ta->at($i2) - $ra->at($i1) );
#	#print "--- asig $asig -sigma -$sigma atol $atol\n";
#        #if ( $asig <0 && $asig > -$sigma) {$k=$i2}
#	#if ($asig > $sigma) {last}    # the points are too far...

#     if (abs($asig) > $sigma) {
#        if ( $asig <0 ) {$k=$i2; next} # We'll start from here at the next loop
#	elsif ($asig>0) {last}    # the points are too far...
#      }

#	$asig=abs($asig);
#	my $dsig=abs( $td->at($i2) - $rd->at($i1) ) ;

#	my $msig=0;
#        if ($msigma>0) {$msig=abs( $tm->at($i2) - $rm->at($i1) )} ;

##Find the points closest in ra & dec and optionally in mag
#	if ($asig <=$atol && $dsig<=$dtol && $msig<=$mtol) {

#               $atol=$asig; $dtol=$dsig; $mtol=$msig;
#              #fill the array with UNSORTED indexes
#	      set ($m,$idx1->at($i1),$idx2->at($i2));
#              #set ($m,$i1,$i2);
	
#	  }
#   }
# }  	

#   #return matching indexes only

#   my $xi=which($m >-1 );

#   if (! $xi->isempty) {$m=$m->index($xi)}


#   return ($xi, $m);

#} #end sub MatchCats



##======================================================
#sub  matchCatsV2
##match two catalogs
##INPUT: reference (m,2) and input catalog (n,2), coordinates in arcsecs
##OUTPUT: a (p,2) array giving in the first column the indexes of the reference catalog
##        and in the second column the matching indexes of the second catalog
#{

#my ($msigma, $rm, $tm);
#$msigma=0;
#  if ($#_==7) {
#    ($rm, $tm, $msigma)=splice (@_, -3);
#    $msigma=0 if ($rm->isempty || $tm->isempty);
#   }

#my ($ra, $rd, $ta, $td, $sigma)=@_;

#my @lra=minmax($ra);
#my @lrd=minmax($rd);
#my @lta=minmax($ta);
#my @ltd=minmax($td);


#my $xi=null;
#my $m=null;
#my $nf=null;

#if ( ($lta[0] >= $lra[1] || $lra[0]>= $lta[1]) ||
#     ($ltd[0] >= $ltd[1] || $lrd[0]>= $ltd[1])) {
#               return ($xi, $m, $nf);
#}



#     my $idx1=qsorti $ra;
#        $ra=$ra->index($idx1);
#        $rd=$rd->index($idx1);

#     my $idx2=qsorti $ta;
#        $ta=$ta->index($idx2);
#        $td=$td->index($idx2);

#if ($msigma>0) {
#  $rm=$rm->index($idx1);
#  $tm=$tm->index($idx2);
#}

#my $k=0;

#my $dim1=$ra->nelem;
#my $dim2=$ta->nelem;

# $m=-1*ones(long, $dim1);
# $nf=zeroes(byte, $dim1);

#my ($i1, $i2);

#for ($i1=0; $i1<$dim1; $i1++) {
#   my $atol=$sigma;
#   my $dtol=$sigma;
#   my $mtol=$msigma;
#   my $nfound=0;
#  #print ">>> Comparing ", $ra->at($i1), " ", $rd->at($i1),"\n";
#   for ($i2=$k; $i2<$dim2; $i2++) {
#     #print "with ", $ta->at($i2), " ", $td->at($i2),"\n";
#    #print "$i2 $k $dim1 $dim2\n";

#        my $asig = ( $ta->at($i2) - $ra->at($i1) );
#	#print "--- asig $asig -sigma -$sigma atol $atol\n";

#     if (abs($asig) > $sigma) {
#        if ( $asig <0 ) {$k=$i2; next} # We'll start from here at the next loop
#	elsif ($asig>0) {last}    # the points are too far...
#      }

##        if ( $asig <0 && $asig > -$sigma) {$k=$i2}
##	if ($asig > $sigma) {last}    # the points are too far...

#	$asig=abs($asig);
#	my $dsig=abs( $td->at($i2) - $rd->at($i1) ) ;

#	my $msig=0;
#        if ($msigma>0) {$msig=abs( $tm->at($i2) - $rm->at($i1) )} ;

#	$nfound++ if ($asig <=$sigma && $dsig<=$sigma);

##Find the points closest in ra & dec and optionally in mag
#	if ($asig <=$atol && $dsig<=$dtol && $msig<=$mtol) {	
#               $atol=$asig; $dtol=$dsig; $mtol=$msig;
#              #fill the array with UNSORTED indexes
#	      set ($m,$idx1->at($i1),$idx2->at($i2));
#               set ($nf,$idx1->at($i1),$nfound);
#              #set ($m,$i1,$i2);
	
#	  }
#   }
# }  	

#   #return matching indexes only

#   $xi=which($m >-1 );

#   if (! $xi->isempty) {$m=$m->index($xi); $nf=$nf->index($xi)}


#   return ($xi, $m, $nf);

#} #end sub MatchCats
#============================================================

sub cleancat
{

my $opt = pop @_ if ref($_[$#_]) eq 'HASH';

my $ret;
return if ($#_ == -1);

my $TEST=0;
$TEST=1 if (defined $$opt{TEST});
my $KEEP=0;
$KEEP=1 if (defined $$opt{KEEP});

my @icat=@_;
my @ocat=();

foreach my $cat (@icat) {
   my @s=split(/\//, $cat);
   $s[-1]='x'.$s[-1];
   my $xcat=join ('/', @s);
    push (@ocat, $xcat);
    # next if (-e $xcat);
    next if ($TEST==1 || (-e $xcat && $KEEP==1) );
    my $xret=xldac($cat, $xcat, $opt);   #remove entries with too small area and saturated stars
 #   return if ($xret==-1);
}

return @ocat;

}




sub xsubcat
{

my ($a, $b, $ptol)= @_;

     my $amin=minimum($a);
     my $amax=maximum($a);

     my $pamin = $amin-$ptol;
     my $pamax = $amax+$ptol;

     my @ndims=$a->dims;

     my $ta = $b->slice(':,(0)');
     my $td = $b->slice(':,(1)');
     my $tm;
     if ($ndims[1] >=3) {$tm=$b->slice(':,(2)')}

      my $m = which ($ta>=$pamin->at(0) & $ta<= $pamax->at(0) & $td>=$pamin->at(1) & $td<= $pamax->at(1));
      if ($m->isempty) {$b=null}
      else {
        if ($ndims[1] ==2) {$b=cat $ta->index($m), $td->index($m)} else {
            $b=cat $ta->index($m), $td->index($m), $tm->index($m)
          }
       }
      return ($b);

}


sub sexcat
  # INPUT:  a list of fits files
  # OUTPUT: a list of coo files
  # optional arguments: output dir., threshold
  {

#optional parameters for SExtractor
my $opt; $opt = pop @_ if ref($_[$#_]) eq 'HASH';

#    my $usew=0;
#    $usew=pop if ($#_==3);
#    my $wtype="NONE";
#    if ($usew==1 || $usew eq 'y') {
#        $wtype="MAP_WEIGHT" ;
#      }

    my $thresh=10;
    my $backsize=32;

    my $spars='';


 #   $thresh=pop if ($#_==2);

    my $odir='.';
    $odir=pop if ($#_>0);

    my $sfiles=shift;
    my @sfiles=@$sfiles;



my $sdir='./';
$sdir=$opt->{CONFDIR} if (exists  $opt->{CONFDIR});

my $run=1;
$run=0  if (exists $opt->{RUN} && $opt->{RUN}==0);


  if ($run !=0) {

   if (exists $opt->{BACK_SIZE}) {$backsize=$opt->{BACK_SIZE}};
   if (exists $opt->{THRESH}) {$thresh=$opt->{THRESH}};
   if (exists $opt->{PARS}) {$spars=$opt->{PARS}};


    #Make the configuration files
    &InitSEx($sdir);  #default values

    foreach my $entry (@SExConf) {
      if (exists $entry->{KEY}) {
	my $key=$entry->{KEY};
	if ($key eq 'WEIGHT_TYPE' &&
            exists $opt->{WEIGHT_TYPE} &&
            $opt->{WEIGHT_TYPE} eq 'BACKGROUND') {	
	       $entry->{VAL}='BACKGROUND'
	}
	elsif ($key eq 'BACK_SIZE') {
	  $entry->{VAL}=$backsize;
	}
	elsif ($key eq 'DETECT_THRESH') {
	        $entry->{VAL}=$thresh;
	      }
      elsif (exists  $opt->{CONF}) {
            foreach  my $econf (%{$opt->{CONF}}) {
	      if ($key eq $econf) {
                  $entry->{VAL}=%{$opt->{CONF}}->{$econf};
		  delete  %{$opt->{CONF}}->{$econf};
		  last;
                }		
            }
        }
      }
  }


      &mkSExPars();
      &mkSExConf();      #write the configuration and parameter files
  }


    #-------------------------------------------------------------------

    # 0a) Extract a catalogue of stars from all frames, to be used for zero shift calculation


    my @scoord=();

    foreach my $i (0..$#sfiles) {

      #   if ($mode eq "REDO" && -e $scoord[$i]) {system ("rm $scoord[$i]")}
      $_=$sfiles[$i];
      my @s=split(/\//,$_);
      my @ns=split(/\./,$s[-1]); #find extension
      $ns[-1]='ldac';
      my $o=$odir.'/'.join ('.',@ns);
      #my ($n, $p, $s) = fileparse($_, ".$s[$#s]");
      #my ($n, $p, $s) = fileparse($_, '\..*');
      #my $o=$n.'.ldac';
      push (@scoord, $o);


       next if ($run !=1);

      if (-e $scoord[$i] && exists $opt->{REWRITE}
      && ($opt->{REWRITE}==1 || uc($opt->{REWRITE}) eq 'Y') ) {
             system "rm $scoord[$i]"
	 }

      if (! -e "$scoord[$i]") {
	if (! -e $sfiles[$i] ) {die "What ? No $sfiles[$i] found !!!! \n"}

         my $sopt;

	if (exists $opt->{WEIGHT_TYPE} ) {
         if ($opt->{WEIGHT_TYPE} eq 'MAP_WEIGHT') {
	    my @s=split (/\./, $sfiles[$i]);
	    $s[-1]='weight.fits';
	    $sopt->{WEIGHT_TYPE} = 'MAP_WEIGHT';
	    $sopt->{WEIGHT_IMAGE}=join ('.', @s);
	  }
	 elsif ( $opt->{WEIGHT_TYPE} =~ /\.fits/)
           { $sopt->{WEIGHT_IMAGE}=$opt->{WEIGHT_TYPE};
                   $sopt->{WEIGHT_TYPE}='MAP_WEIGHT';
		}
	}
	
        my $sexec='sex';
	$sexec= $opt->{EXEC} if exists $opt->{EXEC};

	&RunSEx($sexec, $sfiles[$i], $scoord[$i], $sopt);

      }
    }

    return @scoord;

    #-------------------------------------------------------------------

  }				# end sub sexcat



1;
