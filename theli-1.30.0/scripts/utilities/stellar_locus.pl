#!/usr/bin/perl

# This programme can be used to automatically determine rh (or FWHM) limits
# for the stellar locus.
# As input you have to provide a 1-comlumn ascii file containing the rh
# values of non-saturated stars, where the fainter SN or magnitude cut
# should be relatively high to minimize confusion with galaxies
# The programme can iteratively increase the rh-width if a minimum number
# of stars is added in each step.

# Arguments:
# 1.: The ascii file containing the rh values
# 2.: Start value for the total width ~ 0.2-1
# 3.: Step size for scanning. Choose something small, e.g. 0.01
# 4.: Value to increase the width in each step.:~ 0.1-0.4
# 5.: Maximum number of increasing iterations:: <~ 4
# 6.: Minimum fraction of stars which must be added in an iteration
#     relative to the number in the starting bin, in order to accept this
#     iteration. Otherwise the width will not be increased further: ~ 0.01-0.1

# output: rh_min_select, rh_max_select, Number of selected stars, Highest start position, rh_min, rh_max

$file            = $ARGV[0];
$width           = $ARGV[1];    #total width: START VALUE!
$step            = $ARGV[2];
$increaseby      = $ARGV[3];    # increasing step for width
$increaseNmax    = $ARGV[4];    # Max number of increasements
$increaseMinfrac = $ARGV[5];    # Minimal fractional increasement
                                # of Nstars compared to starting value

# If more than one start value gives the highest number
# of objects, the average of these star values is taken

# Except if we are already increasing the width, than the lowest one is taken

if (-r ($file)) {
  open(DATAFILE, ($file));
  $txtBuf = join("", <DATAFILE>);
  close(DATAFILE);
} else {
  die "File $file not found!";
}

(@daten) = split /\s*\n\s*/, $txtBuf;

@Array = sort(NummernSort @daten);

$min = $Array[0];
$max = $Array[$#Array];

$keepInterating = 1;
$Ninterate      = 0;
while ($keepInterating == 1) {

  $highestNumber   = 0;          # the highest number of stars
  $highestStartSum = 0;          # $start with the  highest number of stars.
                                 # If more than one position, all are added
  $highestStartPositions = 0;    # the number of added positions above

  if (($max - $min) < $width) {
    $rh_min        = ($max + $min - $width) / 2;
    $rh_max        = ($max + $min + $width) / 2;
    $highestNumber = $#Array + 1;
  } else {

    for ($start = $min; $start < ($max - $width); $start += $step) {
      $countIn = 0;

      foreach (@Array) {

        if (($_ >= $start) && ($_ <= ($start + $width))) {
          $countIn++;
        }
      }

      if ($countIn == $highestNumber) {
        if ($Ninterate) {

# we already increase the width. Then always use the lowest bin
# with the max number
        } else {
          $highestStartSum += $start;
          $highestStartPositions++;
        }
      }

      if ($countIn > $highestNumber) {
        $highestNumber         = $countIn;
        $highestStartSum       = $start;
        $highestStartPositions = 1;
      }

      #	print "$countIn\t";
    }
    $rh_min = $highestStartSum / $highestStartPositions;
    $rh_max = $rh_min + $width;
  }

  # check if we continue iterating
  $keepInterating  = 0;
  $acceptNewResult = 0;

  if ($Ninterate == 0) {

    # this is still the unincreased round. Use this as reference
    $Nstarref        = $highestNumber;
    $acceptNewResult = 1;

    if ($increaseNmax > 0) {
      $keepInterating = 1;
    }
  } else {

    if (($highestNumber - $Nstarref_last) > ($increaseMinfrac * $Nstarref)) {
      $keepInterating  = 1;
      $acceptNewResult = 1;
    }
  }

  if ($Ninterate >= $increaseNmax) {
    $keepInterating = 0;
  }

#print "$Ninterate $Nstarref $width $highestNumber $acceptNewResult\n";
#printf "%.2f %.2f %d %d %.3f %.3f\n", $rh_min, $rh_max, $highestNumber, $highestStartPositions, $min, $max;

  $Ninterate++;
  $width += $increaseby;
  $Nstarref_last = $highestNumber;

  if ($acceptNewResult) {
    $bestResult = sprintf "%.2f %.2f %d %d %.3f %.3f\n", $rh_min, $rh_max,
      $highestNumber, $highestStartPositions, $min, $max;
  }

}
print "$bestResult";

#printf "%.2f %.2f %d %d %.3f %.3f\n", $rh_min, $rh_max, $highestNumber, $highestStartPositions, $min, $max;
sub NummernSort {
  if    ($a < $b)  { return -1; }
  elsif ($a == $b) { return 0; }
  else             { return 1; }
}
