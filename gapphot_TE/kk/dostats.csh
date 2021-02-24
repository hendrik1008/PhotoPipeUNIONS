#!/bin/tcsh

setenv PATH ~/data/shapelets/kk/:$PATH

# compute average shear for the whole catalogue
shearstats < test.g > shearstats.out

