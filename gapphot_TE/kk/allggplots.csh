#!/bin/tcsh


foreach lens (0 1 2 3 4) 
 foreach psf (0 1 2 3 4 5)
  cd lens$lens/psf${psf}lens$lens
  cat */test.g > test.g.all
cat > in.smo <<EOF
  macro read "/home/strw/kuijken/data/shapelets/kk/plots.smo"
  device postencap all.ps
  ptype 1 0 expand 0.5
  allpanels
  expand 2
  toplabel psf${psf}lens$lens
  device null
  quit
EOF
  sm < in.smo
  \rm test.g.all
  pwd
  \rm in.smo
  cd ../..
 end
end
