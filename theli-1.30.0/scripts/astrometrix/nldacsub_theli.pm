#Last revision: August 21, 2002

# HISTORY COMMENTS for changements within the
# GaBoDS pipeline context. The original file
# is from v1.2 of Mario Radovich's ASTROMETRIX
#
# 18.07.2005:
#
# function wheadasc:
# HISTORY comments are written at the end of the
# created headers. The function has two new arguments,
# the name of the calling program and an additional
# comment. The name of the calling program must
# be given and the additional comment is optional.
# There is no check whether the length
# of the comment is within the FITS standard
#
# function read_ldac:
# new second argument 'columnfatal'. If it is set
# to '1' and a requested column in the LDAC_OBJECTS
# table is NOT present, the program stops with a
# fatal error.
#
# changes to include a global GaBoDS version number
#
# 06.11.2006:
#
# function wheadasc:
# changes to include the new header keyword BADCCD
# into the ASCII ASTROMETRIX headers (see the file
# astrom_theli.raw for more details).

use strict;

use Tie::IxHash;
use Astro::FITS::CFITSIO qw( :constants );
use Carp;
use PDL;

# global GaBoDS pipeline version
my $GaBoDSVersion="Pipeline Version: 1.30.0";

sub read_ldac
{

my ($file, $columnfatal, @colnames)= @_;

Astro::FITS::CFITSIO::PerlyUnpacking(0);
my ($status,@ycols,@ydim,@ytyp, $naxes,$anynul,$nkeys,$keypos, $card, $nfound, $nrows );

my $fptr = Astro::FITS::CFITSIO::open_file($file,READONLY,$status);
	check_status($status);


if ($colnames[0]=~ /head/i) {


#Move to the second HDU
$fptr->movnam_hdu(ANY_HDU,'LDAC_IMHEAD',0,$status);
	check_status($status);


    my $array;
$fptr->read_col_str(1,1,1,1,0,$array,$anynul,$status);
   check_status($status);

$array=$array->[0];

my $l=length($array);

my $nf=$#colnames;
my $pdl=zeroes(double,$nf);

while ($l>80) {
 my $row=substr($array,0,79);
   foreach my $i (1..$nf) {
     if ($row=~/$colnames[$i]/) {
        my @s=split('=',$row,2);
        @s=split('/',$s[1],2);
#print "********* $s[0]\n";
       set $pdl, $i-1, $s[0];
       last;
    }
  }
  $l-=80;
  $array=substr($array,80,$l);

}

 $status=0;
 $fptr->close_file($status);

return (dog $pdl);
}


#Move to the third HDU
$fptr->movnam_hdu(ANY_HDU,'LDAC_OBJECTS',0,$status);
	check_status($status);

#Check the number of axes


Astro::FITS::CFITSIO::PerlyUnpacking(1);
 $fptr->read_keys_lng("NAXIS", 1, 2, $naxes, $nfound, $status);
    check_status($status);

Astro::FITS::CFITSIO::PerlyUnpacking(0);

if ($naxes->[1] <= 0) {
   $status=0;
  $fptr->close_file($status);
  return -1;
 }

my $colname;

#First

my @xcolnames=@colnames;

foreach $colname (@colnames) {
  my $ycol;
 $status=0.;
 $fptr->get_colnum(0,$colname,$ycol,$status);
   my ($ttype, $tunit, $tchar, $rep, $scale, $zero, $tdisp, $null);
 #print "read $colname\n";
    if ($status == COL_NOT_FOUND) {
	if($columnfatal == 1)
        {
           print "Fatal Error: could not find column $colname in binary table of file $file. Exiting !!\n" ;
	   die ;
	}
        $ycol=-1;
	#@xcolnames=grep(!/$colname/, @xcolnames);
	#next;
    }
  push (@ycols, $ycol);
  if ($ycol < 0) {
     $rep=0; $tchar='';
  } else {
     $status=0.;
     $fptr->get_bcolparms($ycol, $ttype, $tunit, $tchar, $rep, $scale, $zero, $null, $tdisp, $status);
   }

    push (@ydim, $rep);     #the number of columns (usually 1)
 push (@ytyp, $tchar);   #the variable type
}

my %funcs = (
        'B' => { 'cfitsio' => \&Astro::FITS::CFITSIO::fits_read_col_byt, 'pdl' =>byte },
        'I' => { 'cfitsio' => \&Astro::FITS::CFITSIO::fits_read_col_sht , 'pdl' => short},
        'J' => { 'cfitsio' => \&Astro::FITS::CFITSIO::fits_read_col_int, 'pdl' => long},
        'E' => {'cfitsio' => \&Astro::FITS::CFITSIO::fits_read_col_flt, 'pdl' => float },
        'D' =>  { 'cfitsio' => \&Astro::FITS::CFITSIO::fits_read_col_dbl, 'pdl' => double },
);



my @pdl=();
my $j=0;

foreach $colname (@xcolnames) {

 my $array;

 if ($ycols[$j] < 0) {$array=null}
 else {
#warn $j, "  ",$naxes->[1], " ", $ydim[$j], "  ",$ytyp[$j],"\n";
    if ($ydim[$j] > 1) {
        $array= zeroes($funcs{$ytyp[$j]}{'pdl'}, $naxes->[1],$ydim[$j])
     }
    else {$array=zeroes($funcs{$ytyp[$j]}{'pdl'}, $naxes->[1]) }
     $status=0;
    &{$funcs{$ytyp[$j]}{'cfitsio'}}($fptr, $ycols[$j],1,1,
           $ydim[$j]*$naxes->[1],0,${$array->get_dataref},$anynul,$status);
    check_status($status);
#print $array->info;
#print " ********* $colname: $ydim[$j] $ytyp[$j] $ycols[$j]","\n";
   }

  push (@pdl, $array);
 $j++;

}
   $status=0;
  $fptr->close_file($status);


  return (@pdl);

} #ned sub read_ldac


sub xldac
{

my $opt = pop @_ if ref($_[$#_]) eq 'HASH';

return "xldac FAILED\n" if ($#_ < 1);

my ($file, $nfile)= @_;

my $xnum=null;
$xnum=$opt->{NUMBER} if exists $opt->{NUMBER};


my ($xsatur, $xarea, $xthresh, $xfwhm);
$xsatur=0; $xarea=0; $xthresh=0; $xfwhm=0;

if (exists $opt->{MODE} ) {
 my @xopt=();
  my $r=$$opt{MODE};
  if (ref($r) eq 'ARRAY') {
#     $xopt=join '|', @{$$opt{MODE}};
   @xopt=@{$$opt{MODE}}
  }
  else {push @xopt, $r}

foreach my $opt (@xopt) {
  if ($opt =~ /area/) {$xarea=1; next}
  elsif ($opt =~ /fwhm/) {$xfwhm=1; next}
  elsif ($opt =~ /satur/) {$xsatur=1; next}
  elsif ($opt =~ /thresh/) {
    my @s=split(/=/, $opt, 2);
    next if ($s[1]<=0);
    $xthresh=$s[1];
    next}
}

}



#$xnum is a pdl telling the NUMBER (keyword) of the entries that should be extracted

#print "#DEBUG XCLEAN: Cleaning $file to $nfile with option $xopt\n";
#print "#DEBUG            extracting NUMBER $xnum   \n" if ($xnum);

# OK, let's go !

Astro::FITS::CFITSIO::PerlyUnpacking(0);

my ($status,$ycol,$naxes,$anynul,$nkeys,$keypos, $card, $nfound, $nrows );

unlink $nfile if (-e $nfile);

$status=0;
my $fptr = Astro::FITS::CFITSIO::open_file($file,READONLY,$status);

	check_status($status);

$status=0;
my $nfptr = Astro::FITS::CFITSIO::create_file($nfile,$status);
	check_status($status);

#Move to and copy primary HDU
  my   $morekeys = 0;
    $fptr->copy_hdu($nfptr, $morekeys, $status);
         check_status($status);

#Move to and copy the second HDU
$fptr->movnam_hdu(ANY_HDU,'LDAC_IMHEAD',0,$status);
	check_status($status);
$fptr->copy_hdu($nfptr, $morekeys, $status);
       check_status($status);

#Move to the third HDU
$fptr->movnam_hdu(ANY_HDU,'LDAC_OBJECTS',0,$status);
	check_status($status);

#Create space for the HDU on the output file
$nfptr->create_hdu($status);
  check_status($status);


#Copy the records
  $fptr->get_hdrpos($nkeys, $keypos, $status) ;
         check_status($status);

   my $ii;


   foreach $ii (1..$nkeys) {
        $status=0;
        $fptr->read_record ($ii, $card, $status);
        $nfptr->write_record($card, $status);
    }

#Check the number of axes


Astro::FITS::CFITSIO::PerlyUnpacking(1);
 $fptr->read_keys_lng("NAXIS", 1, 2, $naxes, $nfound, $status);
    check_status($status);
Astro::FITS::CFITSIO::PerlyUnpacking(0);

if ($naxes->[1]==0) {
  $status=0;
  $fptr->close_file($status);
  $status=0;
  $nfptr->close_file($status);
  return -1
}

#Read the columns to be extracted

my $colname;

my ($number, $imatch);
my ($flags, $ell, $area, $thresh, $mag, $fwhm);

if (! $xnum->isempty) {
  $colname='NUMBER';  $status="";
 $fptr->get_colnum(0,$colname,$ycol,$status);
($status == COL_NOT_FOUND) and
    die "$0: could not find TTYPE $colname in binary table";
  $number = zeroes(float, $naxes->[1]);
$fptr->read_col_flt($ycol,1,1,$naxes->[1],0,${$number->get_dataref},$anynul,$status);
check_status($status);
$imatch=vsearch($xnum,$number);    #vsearch finds the index of matching elements in two pdls
#print "******* ! $number \n ********** $xnum\n";
}

 $colname='FLAGS';  $status="";
 $fptr->get_colnum(0,$colname,$ycol,$status);
($status == COL_NOT_FOUND) and
    die "$0: could not find TTYPE $colname in binary table";
  $flags = zeroes(short, $naxes->[1]);
$fptr->read_col_sht($ycol,1,1,$naxes->[1],0,${$flags->get_dataref},$anynul,$status);
check_status($status);

 $colname='ELLIPTICITY';  $status="";
 $fptr->get_colnum(0,$colname,$ycol,$status);
($status == COL_NOT_FOUND) and
    die "$0: could not find TTYPE $colname in binary table";
  $ell = zeroes(float, $naxes->[1]);
$fptr->read_col_flt($ycol,1,1,$naxes->[1],0,${$ell->get_dataref},$anynul,$status);
 check_status($status);


 $colname='MAG_APER';
 $status="";
 $fptr->get_colnum(0,$colname,$ycol,$status);
if ($status == COL_NOT_FOUND){
  warn "xldac: MAG_APER nout found, looking for MAG_AUTO ";
    $colname='MAG_AUTO';
    $status="";
      $fptr->get_colnum(0,$colname,$ycol,$status);
    ($status == COL_NOT_FOUND) and
       die "$0: could not find TTYPE $colname in binary table";
  }
  $mag = zeroes($naxes->[1])->float;
$fptr->read_col_flt($ycol,1,1,$naxes->[1],0,${$mag->get_dataref},$anynul,$status);
check_status($status);

#the are the optional values

 if ($xarea) {
 $colname='ISOAREA_IMAGE';  $status="";
 $fptr->get_colnum(0,$colname,$ycol,$status);
($status == COL_NOT_FOUND) and
    die "$0: could not find TTYPE $colname in binary table";
  $area = zeroes(float, $naxes->[1]);
$fptr->read_col_flt($ycol,1,1,$naxes->[1],0,${$area->get_dataref},$anynul,$status);
check_status($status);
}


 if ($xfwhm) {
 $colname='FLUX_RADIUS';  $status="";
 $fptr->get_colnum(0,$colname,$ycol,$status);
($status == COL_NOT_FOUND) and
    die "$0: could not find TTYPE $colname in binary table";
  $fwhm = zeroes(float, $naxes->[1]);
$fptr->read_col_flt($ycol,1,1,$naxes->[1],0,${$fwhm->get_dataref},$anynul,$status);
check_status($status);
}



if ($xthresh) {
 $colname='THRESHOLD';  $status="";
 $fptr->get_colnum(0,$colname,$ycol,$status);
($status == COL_NOT_FOUND) and
    die "$0: could not find TTYPE $colname in binary table";
 $thresh = zeroes(float, $naxes->[1]);
$fptr->read_col_flt($ycol,1,1,$naxes->[1],0,${$thresh->get_dataref},$anynul,$status);
check_status($status);
}


my $mdar=0;


if ($xarea==1) {
#  my $iell=which($ell<=0.5 & $flags<4);
  $mdar=median($area)/1.2;
  #$mdar=median($area->index($iell)/1.2); #take a lower limit for the area
}


my $mdfwhm=0;
if ($xfwhm==1) {
  $mdfwhm=median($fwhm);
warn "=== $mdfwhm";
}


my $noutrows=0;

my $buffer = zeroes($naxes->[0])->byte;

# ***********************************************************
# THIS IS WHERE THE SELECTION IS DONE:
# 1) ELLIPTICITY <= 0.5
# 2) ISOAREA_IMAGE >= median(ISOAREA_IMAGE) / 1.2 (= take a 20% tolerance)

#warn "$nfile ---> $xopt";

my $irow;
    foreach $irow (1..$naxes->[1])  {

    if (! $xnum->isempty) {
        my $f=which($imatch == $irow-1);
     # print $irow-1," ///", $f,"\n";
        next if $f->isempty;
    }

    next if ($ell->at($irow-1) >  0.5 || $mag->at($irow-1) == 99);

#    next if ( $mag->at($irow-1) == 99);


   if ($xthresh>0) {
       next if ($thresh->at($irow-1) < $xthresh) ;
   }

   if ($xarea==1 && $mdar !=0) {
         next if ($area->at($irow-1) < $mdar) ;
      }

   if ($xfwhm==1 && $mdfwhm !=0) {
# reject points below the given fwhm
         next if ($fwhm->at($irow-1) < $mdfwhm*0.2) ;
      }


     if ($xsatur==1) {
        #  if ($ell->at($irow-1) > 0.5) {next}
          if ($flags->at($irow-1) >=4) {next}
      }
       else {
           next  if ($flags->at($irow-1) >=8);
    	  }


        $noutrows++;

        $fptr->read_tblbytes($irow, 1, $naxes->[0], ${$buffer->get_dataref}, $status);
            check_status($status);

        $nfptr->write_tblbytes($noutrows, 1,  $naxes->[0], $buffer->get_dataref, $status);
           check_status($status);
     }


# ***********************************************************

if ($noutrows <=1) {print "???????? in $file: xldac gives noutrows = $noutrows\n"};

#update the number of rows
   $nfptr->update_key(TLONG, "NAXIS2", $noutrows, 0, $status);
    check_status($status);

 $status=0;
 $nfptr->close_file($status);

 $status=0;
 $fptr->close_file($status);



}



sub rhead {

#store the header of a FITS file into an ORDERED hash
#a sequential number is appended to the HISTORY and COMMENT rows in order to store them
#in the hash as well

my $file=shift;
my $ch=1;
$ch=shift if ($#_>=0);

my ($n, $left);

my $status = 0;
my $ret;
my $fitsfile = Astro::FITS::CFITSIO::open_file($file, READONLY, $status);
if ($status ne 0) {Astro::FITS::CFITSIO::fits_get_errstatus($status,$ret); return}

#Preserve the order in the hash !!!
my %header;
tie ( %header, 'Tie::IxHash');
$fitsfile->get_hdrspace($n, $left, $status);

my ($key, $value, $comment);

my $nhist=1;
my $ncomm=1;



for my $i (1..$n) {

   last unless $status == 0;
   $fitsfile->read_keyn($i, $key, $value, $comment, $status);
   next if  ($key =~ /BSCALE/i || $key =~ /BZERO/i);
   if ($key =~ /HISTORY/ && $ch==1) {
      $key=$key.$nhist;
      $nhist++;
     }
   elsif ($key =~ /COMMENT/ && $ch==1)  {
       $key=$key.$ncomm;
       $ncomm++
    }
       $header{$key} = $value;
       $header{COMMENTS}{$key} = $comment;
   }

 $status=0;
$fitsfile->close_file($status);

return %header;
} #end sub rhead


sub apphead
{
 my ($file, $header)=@_;
 my $status = 0;
 my $ret;
 my $fptr = Astro::FITS::CFITSIO::open_file($file, READWRITE, $status);

if ($status ne 0) {Astro::FITS::CFITSIO::fits_get_errstatus($status,$ret);   die $ret}
   my @keys=keys %$header ;



 foreach my $i (0..$#keys){
         my $h=$keys[$i];
	 next if $h eq 'SIMPLE';
	 my $val=$$header{$h};
	 my $comm= $$header{COMMENTS}{$h};
	 my $type=getheadtype(\$val);
	 if ($h =~ /COMMENT/) {Astro::FITS::CFITSIO::fits_write_comment($fptr,$comm,$status)}
	 elsif  ($h =~ /HISTORY/) {Astro::FITS::CFITSIO::fits_write_history($fptr,$comm,$status)}
	 else {$fptr->update_key($type,$h,$val,$comm, $status)}
         if ($status ne 0) {Astro::FITS::CFITSIO::fits_get_errstatus($status,$ret); $fptr->close_file($status);return $ret}
   }
   $status = 0;
   $fptr->close_file($status);


   return $ret;
}


sub whead
{
 my ($file, $header)=@_;
 my ($status, $ret);

  $ret="";
  $status=0;
  my $fptr = Astro::FITS::CFITSIO::create_file($file,$status);
  if ($status ne 0) {Astro::FITS::CFITSIO::fits_get_errstatus($status,$ret); return $ret}

#Create the header
  $fptr->write_key(TLOGICAL,"SIMPLE",1,"Standard FITS",$status);
  if ($status ne 0) {Astro::FITS::CFITSIO::fits_get_errstatus($status,$ret); $fptr->close_file($status);return $ret}
 #write the header
    my @keys=keys %$header ;
 foreach my $i (1..$#keys){
         my $h=$keys[$i];
	 my $val=$$header{$h};
	 my $comm= $$header{COMMENTS}{$h};
	 my $type=getheadtype(\$val);
	 if ($h =~ /COMMENT/) {Astro::FITS::CFITSIO::fits_write_comment($fptr,$comm,$status)}
	 elsif  ($h =~ /HISTORY/) {Astro::FITS::CFITSIO::fits_write_history($fptr,$comm,$status)}
	 else {$fptr->write_key($type,$h,$val,$comm, $status)}
         if ($status ne 0) {Astro::FITS::CFITSIO::fits_get_errstatus($status,$ret); $fptr->close_file($status);return $ret}
   }

 $status=0;
 $fptr->close_file($status);

 return $ret;
} #end sub wfits


sub UpHead
  {

# Merge headers to lists of  fits files

my ($fits, $hdr)=@_;
my @fits=@$fits; my @hdr=@$hdr;

foreach my $i (0..$#fits) {
  my (%hh, %fh);
  tie (%fh, 'Tie::IxHash');
  tie (%hh, 'Tie::IxHash');

   %fh=rhead($fits[$i]);
   %hh=rheadasc($hdr[$i]);

  foreach my $key (keys %hh) {
# print "$key ";
   $fh{$key}=$hh{$key}
  }
#print "\n";

    my $wret=apphead($fits[$i], \%fh);
    warn "$fits[$i]: $wret" if ($wret);
 }

 }





sub getheadtype
{
#get the right type for the values in the header (called by wfits)
#the code was borrowed from the PDL IO library

my $val=shift;

my $typ;

 if ($$val =~ /^ *([+-]?)(?=\d|\.\d)\d*(\.\d*)?([Ee]([+-]?\d+))? *$/) { # Number?
        $typ=TFLOAT;
	$typ=TSHORT if ($$val !~ /\./) ;	#Is it really a float ?
  }  elsif ($$val eq 'F' or $val eq 'T') {          # Logical flags ?
      $typ=TLOGICAL;

} else {
         $$val=$1 if ($$val=~m/'(.*)'/); # Take off surrounding ''
          $typ=TSTRING;
  }

 return $typ;
}  #end sub getheadtype	


sub rheadasc {

#store the header of a FITS file into an ORDERED hash
#a sequential number is appended to the HISTORY and COMMENT rows in order to store them
#in the hash as well

my $file=shift;


my ($n);

if (!-e "$file") {print "ERROR in sub rheadasc: can't open file $file\n"; die}

open (HFILE, "$file");


#Preserve the order in the hash !!!
my %header;
tie ( %header, 'Tie::IxHash');
my $nhist=1;
my $ncomm=1;

while (<HFILE>) {
   chomp ($_);
   my $line=$_;
   my $key=substr($line,0,8);
   my $value="";
   if (length($line) > 9) {
      $value=substr($line,9);
   }
   my $comment='';
     $key=~ tr/ //d;
   if ($key eq 'END') {last}

    if ($key eq 'HISTORY') {
      if (substr($line, 8,1) ne '=')  {$value=substr($line,8)}
      $key=$key.$nhist;
      $nhist++;
     }
   elsif ($key eq 'COMMENT')  {
      if (substr($line, 8,1) ne '=')  {$value=substr($line,8)}
       $key=$key.$ncomm;
       $ncomm++
    }
   else {
      $_=$value;
#print "$value ";
      ($value, $comment)=/\s*('.*?'|\S+)\s*\/?(.*)/ ;
#print " ---> $value # $comment\n";
     }

   next if  ($key =~ /BSCALE/i || $key =~ /BZERO/i);

       $header{$key} = $value;
       $header{COMMENTS}{$key} = $comment;



}


close (HFILE);

return %header;
} #end sub rheadasc


sub wheadasc
{

#Dump the header to an ASCII file

my ($file, $header, $opt, $aname, $ahistcomment)=@_;

$opt=lc($opt);

my $ret=0;
my $histcomment;
my $rightnow;

if (-e $file) {system ("rm $file")}
open (FILE, ">$file");

#write the header
    my @keys=keys %$header ;
 foreach my $i (0..$#keys){
         my $s;
         my $h=$keys[$i];
	 if ($opt eq 'astrom') {
            next if ($h !~ /CRVAL/ && $h !~ /CRPIX/ && $h !~ /CD/ &&
             $h !~ /PV/ && $h ne /DISTORD/ && $h ne 'EQUINOX' && $h ne 'BADCCD' &&
             $h ne 'IMAGEID' && $h ne 'RA' && $h ne 'DEC' && $h ne 'EXPTIME'
	      && $h ne 'DATE')
         }
         if ($h eq 'COMMENTS' || $h eq 'END') {next}
	 my $val=$$header{$h};
	 my $comm= $$header{COMMENTS}{$h};
	# my $type=getheadtype(\$val);
	 if ($h =~ /COMMENT/) {$s=sprintf("%-7s %s", 'COMMENT', $val);}
	 elsif ($h =~ /HISTORY/) {$s=sprintf("%-7s %s", 'HISTORY', $val)}
	 else {$s=sprintf("%-8s= %-20s", $h, $val);
            if ($comm ne '') {$s=$s.sprintf(" / %s",$comm)}
          }
         print FILE "$s\n";
   }

$histcomment=sprintf("%-7s %s", 'HISTORY', "");
print FILE "$histcomment\n";
$histcomment=sprintf("%-7s %s", 'HISTORY', "$aname: $GaBoDSVersion");
print FILE "$histcomment\n";
$rightnow=&mkdate;
$histcomment=sprintf("%-7s %s", 'HISTORY', "$aname: called at $rightnow");
print FILE "$histcomment\n";

if (defined ($ahistcomment)) {
  $histcomment=sprintf("%-7s %s", 'HISTORY', "$aname: $ahistcomment");
  print FILE "$histcomment\n";
}

print FILE "END\n" if ($opt ne 'noend');
close (FILE);


return $ret;

} #end sub whead



sub wldac
{
my ($file, @vals, $ttype, $tform, $tunit);

$file=shift(@_);
$tunit=pop(@_);
$tform=pop(@_);
$ttype=pop(@_);
@vals=@_;


Astro::FITS::CFITSIO::PerlyUnpacking(0);

my ($status, $fptr);
my ($simple,$bitpix,$naxis,$naxes,$pcount,$gcount,$extend);
 my ($nrows,$tfields);

system ("rm $file") if (-e $file);

 $status=0;
 $fptr = Astro::FITS::CFITSIO::create_file($file,$status);
	check_status($status);

$simple=1;
$bitpix=8;
$naxis=0;
$naxes=['0 0'];
$pcount=0;
$gcount=1;
$extend=1;


  $status=0;
  $fptr->write_imghdr($bitpix,$naxis,$naxes,$status);
  	check_status($status);

# TO BE MODIFIED


$nrows = $vals[0]->getdim(0);

$tfields = $#vals+1;
$pcount = 0;
my $binname='LDAC_OBJECTS';

 $status=0;
 $fptr->create_tbl(BINARY_TBL,$nrows,$tfields,$ttype,$tform,$tunit,$binname, $status);
      check_status($status);


my %funcs = (
        'byte' => { 'cfitsio' => \&Astro::FITS::CFITSIO::fits_write_col_byt },
        'short' => { 'cfitsio' => \&Astro::FITS::CFITSIO::fits_write_col_sht },
        'long' => { 'cfitsio' => \&Astro::FITS::CFITSIO::fits_write_col_int},
        'float' => {'cfitsio' => \&Astro::FITS::CFITSIO::fits_write_col_flt },
        'double' =>  { 'cfitsio' => \&Astro::FITS::CFITSIO::fits_write_col_dbl },
);

my $ncol=1;

foreach my $array (@vals) {
  my $type=lc($array->info("%T"));
  $nrows=$array->nelem;
  $status=0;

  &{$funcs{$type}{'cfitsio'}}($fptr, $ncol,1,1,$nrows,$array->get_dataref,$status);
  	check_status($status);

 $ncol++;

}

  $status=0;
 $fptr->close_file($status);
  	check_status($status);

} #end sub wldac



sub check_status {
    my $status = shift;
    my $errtxt;
    if ($status) {
      Astro::FITS::CFITSIO::fits_get_errstatus($status,$errtxt);
	croak "$0: Astro::FITS::CFITSIO error, aborting...$errtxt";
    }

    return 1;
}


1;

