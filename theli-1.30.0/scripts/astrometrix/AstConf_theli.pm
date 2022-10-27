# Lat modified: April 5, 2002

 use strict;
 use vars qw(@AstConf %AstCVals);

my $AstConf='astrom.conf';



 sub InitAstrom {

 @AstConf=(

    {
   COMM =>  ' ========= DIRECTORIES',
    },

    {
   KEY  =>  'FITS_DIR',
   COMM =>  ' The directory containing the FITS files',
    },

    {
   KEY  =>  'CATS_DIR',
   COMM =>  ' The directory containing the SExtractor catalogs (=> global)',
    },

    {
   KEY  =>  'OUTDIR_TOP',
   COMM =>  ' The directory where everything will be written (top level)',
    },


    {
   KEY  =>  'OUTDIR',
   COMM =>  ' The output directory for catalogs/headers',
    },

   {
   KEY  =>  'DIAGDIR',
   COMM =>  ' The directory with logs & diagnostic files',
    },


#   {
#   KEY  =>  'SEXCONFDIR',
#   COMM =>  ' The directory with SExtractor configuration files',
#    },


    {
   COMM =>  ' ========= The I/O keyword',
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
   COMM =>  ' Group files in lists using EXPNAME ? [y/n]',
    },

    {
   KEY  =>  'CATALOG',
   COMM =>  ' The astrometric catalog ',
    },

    {
   KEY  =>  'ASKWEB',
   COMM =>  ' Retrieve from WEB if usno or gsc ? ',
    },

    {
   KEY  =>  'RADIUS',
   COMM =>  ' The  width of the astrometric catalog ',
    },


    {
   KEY  =>  'MAXNUM_LIN',
   COMM =>  ' The maximum number of sources to be retrieved (field = 1 mosaic)',
    },

    {
   KEY  =>  'MAXNUM_GLOB',
   COMM =>  ' The maximum number of sources to be retrieved (field >= 1 mosaics)',
    },


    {
   KEY  =>  'ERRCAT',
   COMM =>  ' The uncertainty in the positions of the catalog',
    },

    {
   KEY  =>  'OBSERVATORY',
   COMM =>  ' The observatory from which the data come',
    },

   {
    KEY => 'HDRFITS',
       COMM =>  'Update the headers of FITS files ? [y/n]',
    },



    {
   COMM =>  ' ========= K TABLE (initial plate solution)',
    },


    {
   KEY  =>  'INITCD',
   COMM =>  ' Name of the table with initial values of CRVAL, CRPIX and CD (optional)',
    },


    {
   KEY  =>  'TABLE',
   COMM =>  ' Name of the K-table',
    },

    {
   KEY  =>  'KANGLE',
   COMM =>  ' initial value for the angle',
    },

    {
   KEY  =>  'REJANGLE',
   COMM =>  ' Threshold for the rejection of wrong angles',
    },

    {
   KEY  =>  'DELTANGLE',
   COMM =>  ' Maximum difference between angles and the median',
    },


    {
   COMM =>  ' ========= The WCS fix/global solution',
    },

    {
   KEY  =>  'WTYPE',
   COMM =>  ' weight type used by sextractor: ',
    },

   {
   KEY  =>  'XSATUR',
   COMM =>  'Reject saturated sources (y/n) ?',
    },



    {
   KEY  =>  'THRESH',
   COMM =>  ' Threshold used by sextractor ',
    },

   {
    KEY => 'XPSOL',
       COMM =>  'A table with pre-computed plate solutions ',
    },


    {
   KEY  =>  'LINTOL',
   COMM =>  ' The tolerance (arcsec) in catalog matching for linear WCS',
    },


   {
   KEY  =>  'LINPSOL',
   COMM =>  ' Compute an initial plate solution together with the linear fix (y/n) ?',
    },



    {
   KEY  =>  'PSTOL',
   COMM =>  ' The tolerance (arcsec) in catalog matching when the plate solution is computed',
    },


#    {
#   KEY  =>  'XTRESH',
#   COMM =>  ' The threshold to be used use when cleaning catalogs',
#    },


    {
   KEY  =>  'NITERFIT',
   COMM =>  ' The maximum number of iterations for rejection of outliers',
    },

    {
   KEY  =>  'SIGMA',
   COMM =>  ' The sigma rejection for the fit',
    },


    {
   KEY  =>  'ITERATIONS',
   COMM =>  ' The number of iterations',
    },

    {
   KEY  =>  'PSORD_OL_OFF',
   COMM =>  ' The order of the polynomial for the plate sol. without overlapping sources',
    },

    {
   KEY  =>  'PSORD_OL_ON',
   COMM =>  ' The order of the polynomial for the plate sol. with overlapping sources',
    },


    {
   KEY  =>  'NARROW',
   COMM =>  ' Set Y[es] for ditherings with a small offset',
    },

    {
   COMM =>  ' ========= Location of external executables',
    },

    {
   KEY  =>  'GZIP',
   COMM =>  'The location of gzip',
    },

    {
   KEY  =>  'SEX',
   COMM =>  'The location of SExtractor',
    },

    {
   KEY  =>  'WGET',
   COMM =>  'The location of wget (GSC only)',
    },


     );

# ============> EDIT HERE TO CHANGE DEFAULT VALUES
%AstCVals = (
  EXPNAME=>'(.*\D)(\d+)(\D+)$',
 FITS_DIR => '.',
 OUTDIR_TOP => './',
 CATS_DIR => 'astrom',
 DIAGDIR=>'AstroDiag',
 CATALOG => 'USNO' ,
 ASKWEB => 'y',
 GROUPCCD => 'y',
 ERRCAT => 0.3 ,
 MAXNUM_LIN =>   10000,
 MAXNUM_GLOB => 100000,
 OBSERVATORY => 'CFH12K' ,
 TABLE => 'psol.dat' ,
 THRESH => 10,
 WTYPE => 'BACKGROUND ' ,
 XSATUR => 'Y',

 LINTOL => 5 ,
 LINPSOL => 'Y',
 PSTOL => 1 ,
 NITERFIT => 2,
 SIGMA => 2,
 ITERATIONS => 3 ,
 PSORD_OL_OFF => 2 ,
 PSORD_OL_ON => 3,
 NARROW => 'Y' ,
 RADIUS => 40,
 MAXNUM=>9999,
 REJANGLE=> 1.5,
 DELTANGLE=>1,
 HDRFITS=>'n',
 XPSOL=>'',

 GZIP=>'gzip',
 SEX=>'sex',
 WGET => 'wget'
    );
# ==================================================

#my %ObsPars =(
##entries: latitude, longitude
# ESO => ['-29:15.4','70:43.8'],
# CFHT => ['19:49.6','155:28.3']
#)

     };

sub mkAstConf
{

my $pfile;

if ($#_ >= 0) {$pfile=shift}
else {$pfile=$AstConf}

open (FILE, ">$pfile");
my $entry;
foreach $entry (@AstConf) {
  my $nokey=0;
   if (exists $entry->{KEY}) {
      my $key=$entry->{KEY};
      print FILE "$key\t";
      if (exists $AstCVals{$key}) {
           print FILE "$AstCVals{$key}\t";
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


sub ReadAstConf
{

my $pfile;

if ($#_ >= 0) {$pfile=shift}
else {$pfile=$AstConf}

return if (! -e $pfile);

open (FILE, "<$pfile");

while (<FILE>) {
  chomp;

  next if ($_ =~ /^(\s*|\t*)\#/);
  my ($vals, $comm)=split(/\#/, $_);
  my ($key, $val)=split(/\s+|\t+/, $vals);

 #  next if (! exists $AstCVals{$key});

    $AstCVals{$key} = $val;
 }

 close (FILE);
 }


