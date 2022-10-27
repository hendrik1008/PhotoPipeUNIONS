# HISTORY COMMENTS for changements within the
# GaBoDS pipeline context. The original file
# is from v1.2 of Mario Radovich's ASTROMETRIX
#
# 18.07.2005:
#
# new configuration parameters for PHOTOMETRIX:
# - MAG: specifies the magnitude to be used for
#        relative photometric zeropoints (this
#        was MAG_APER before, now MAG_AUTO is the
#        default).
#
# - K_FLXSCALE: The header keyword that specifies the
#               relative photometric zeropoint and is
#               written to the headers. This was FLSCALE
#               before but now FLXSCALE is the default.
#

use strict;
use vars qw(@PhotConf %PhotCVals);

my $PhotConf='photom.conf';



 sub InitPhotom {

 @PhotConf=(

    {
   COMM =>  ' ==== DIRECTORIES',
    },

    {
   KEY  =>  'HDR_DIR',
   COMM =>  ' The directory containing headers',
    },

    {
   KEY  =>  'FITS_DIR',
   COMM =>  ' The directory containing the FITS files',
    },

    {
   KEY  =>  'CATS_DIR',
   COMM =>  ' The directory containing the SExtractor catalogs',
    },

    {
   KEY  =>  'MCATS_DIR',
   COMM =>  ' The directory containing the master catalogs',
    },


   {
   KEY  =>  'OUTDIR_TOP',
   COMM =>  ' The directory where everything will be written (top level)',
    },


    {
   KEY  =>  'OUTDIR',
   COMM =>  ' The output directory',
    },


   {
   KEY  =>  'DIAGDIR',
   COMM =>  ' The directory with logs & diagnostic files',
    },


    {
   COMM =>  ' ====  The I/O keyword',
    },

    {
   KEY  =>  'LIST',
   COMM =>  ' Input lists separated by commas',
    },

 {
   KEY  =>  'EXPNAME',
   COMM =>  ' The perl regular expression to separe root/CCD in filenames',
    },

  {
   KEY  =>  'GROUPCCD',
   COMM =>  '  Group files in lists using EXPNAME ? [y/n]',
    },

   {
    KEY => 'HDRFITS',
       COMM =>  'Update the headers of FITS files ? [y/n]',
    },




#   {
#   KEY  =>  'SELECT',
#   COMM =>  ' Use only the selected lists',
#    },

   {
   KEY  =>  'SUM_FILE',
   COMM =>  ' The name of the summary file',
    },

   {
   KEY  =>  'OUT_HEAD',
   COMM =>  ' The name of the output header',
    },



    {
   COMM =>  ' ====  Settings',
    },

   {
   KEY  =>  'MAG',
   COMM =>  'The magnitude (catalog quantity) to use for relative photometry ',
    },

   {
   KEY  =>  'MAGERR',
   COMM =>  'The magnitude error (catalog quantity) to use for relative photometry ',
    },

   {
   KEY  =>  'MAG_APERTURE',
   COMM =>  'The aperture diameter (pixels) if catalogs with magnitudes are rebuilt ',
    },

   {
   KEY  =>  'THRESH',
   COMM =>  'The threshold used in catalog extraction ',
    },

    {
   KEY  =>  'WTYPE',
   COMM =>  'The weight type used by sextractor: ',
    },


   {
   KEY  =>  'ZP0',
   COMM =>  'The renormalized zero point ',
    },

   {
   KEY  =>  'AIRMODE',
   COMM =>  'AIRMASS is computed from: PHOT - photometric meas. or COEFF - given coeffs ',
    },

   {
    KEY  =>  'DMAG',
   COMM =>  'Maximum difference in magnitudes',
    },

   {
   KEY  =>  'LMAG',
   COMM =>  ' Low limit for magnitude',
    },

   {
   KEY  =>  'HMAG',
   COMM =>  ' High limit for magnitude',
    },

   {
    KEY  =>  'DPOS',
   COMM =>  'Maximum difference in positions (arcsec)',
    },

   {
   KEY  =>  'MAXITER',
   COMM =>  'Relative phot.: maximum number of iterations',
    },

   {
   KEY  =>  'MINCHI2',
   COMM =>  'Relative phot.: minimum chi**2 in residuals',
    },


   {
   KEY  =>  'MIN_OVERLAP',
   COMM =>  'Minimum number of points for overlap ',
    },

    {
   COMM =>  ' ====  FITS header keywords',
    },

   {
   KEY  =>  'OBSERVATORY',
   COMM =>  'The name of the observatory/instrument (CFH12K, ESO,....) ',
    },

   {
   KEY  =>  'K_DATE',
   COMM =>  'The name of the keyword with DATE ',
    },

   {
   KEY  =>  'K_TIME',
   COMM =>  'The name of the keyword with TIME ',
    },


   {
   KEY  =>  'K_EXPTIME',
   COMM =>  'The name of the keyword with EXPTIME ',
    },

   {
   KEY  =>  'K_AIRMASS',
   COMM =>  'The name of the keyword with AIRMASS ',
    },

   {
   KEY  =>  'K_FLXSCALE',
   COMM =>  'The name of the keyword to be used for the relative flux scale parameter ',
    },


    {
   COMM =>  ' ========= Location of external executables',
    },

    {
   KEY  =>  'SEX',
   COMM =>  'The location of SExtractor',
    },

     );

# Here values are stored
%PhotCVals = (
 OUTDIR_TOP => './',
 DIAGDIR=>'PhotoDiag',
 EXPNAME=>'(.*\D)(\d+)(\D+)$',
 GROUPCCD => 'y',
 HDRFITS => 'n',
 OUT_HEAD => 'gphot.head',
 MAG => 'MAG_AUTO',
 MAGERR => 'MAGERR_AUTO',
 MAG_APERTURE => 20,
 THRESH => 10,
 WTYPE => 'BACKGROUND ' ,
 ZP0 => 30,
 LMAG => -100 ,  #e.g. 16
 HMAG => 100 ,   #e.g. 22
 MIN_OVERLAP => 10,
 DMAG => 2,
 DPOS => 0.5,
 MAXITER => 50,
 MINCHI2 => 0.001,
 AIRMODE => 'PHOT',
 SUM_FILE => 'results.dat',
 OBSERVATORY => 'CFH12K',
 K_DATE => 'DATE-OBS',
 K_TIME => 'UTC-OBS',
 K_EXPTIME => 'EXPTIME',
 K_AIRMASS => 'AIRMASS',
 K_FLXSCALE => 'FLXSCALE',

 SEX=>'sex'
    );

#my %ObsPars =(
##entries: latitude, longitude
# ESO => ['-29:15.4','70:43.8'],
# CFHT => ['19:49.6','155:28.3']
#)

     };

sub mkPhotConf
{

my $pfile;

if ($#_ >= 0) {$pfile=shift}
else {$pfile=$PhotConf}

open (FILE, ">$pfile");
my $entry;
foreach $entry (@PhotConf) {
  my $nokey=0;
   if (exists $entry->{KEY}) {
      my $key=$entry->{KEY};
      print FILE "$key\t";
      if (exists $PhotCVals{$key}) {
           print FILE "$PhotCVals{$key}\t";
	}
    } else {$nokey=1}
    if (exists $entry->{COMM}) {
        my $comm=$entry->{COMM};
         if ($nokey==1 && $comm !~ '--') {print FILE "\t\t\t"}
	
           print FILE "#$comm";
	}
  print FILE "\n";	
   }

 close (FILE);
 }


sub ReadPhotConf
{

my $pfile;

if ($#_ >= 0) {$pfile=shift}
else {$pfile=$PhotConf}

return if (! -e $pfile);

open (FILE, "<$pfile");

while (<FILE>) {
  chomp;

  next if ($_ =~ /^(\s*|\t*)\#/);
  my ($vals, $comm)=split(/\#/, $_);
  my ($key, $val)=split(/\s+|\t+/, $vals);

 #  next if (! exists $PhotCVals{$key});

    $PhotCVals{$key} = $val;
 }

 close (FILE);
 }


