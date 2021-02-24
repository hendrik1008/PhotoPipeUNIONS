#!/usr/bin/env python3

"""
script/module to obtain an LDAC catalogue from an astronomical
standard star catalogue. We want a catalogue with the columns Ra, Dec
and magnitude.  The magnitude depends on the catalogue to be queried.

If used as a script the output is an LDAC catalogue with the name of
the queries standardstar catatalogue. Also the table with the object
entries shares that name.
"""

import sys, os
import numpy as np
import astropy.coordinates as coord
import astropy.units as au
import astroquery.vizier as av
import argparse
import ldac

#$1: Ra center of field
#$2: Dec center of field
#$3: ra-range of field (arcmin)
#$4: dec-range of field (arcmin)
#$5: standard star catalogue
#$6: output LDAC catalogue (table name is the name of the
#    standard star catalogue; columns are Ra, Dec and magnitude)

def get_standard_cat(racen, deccen, rarange, decrange):
    """
    The function retries coordinates and magnitudes for Vizier
    catalogues. We currently support:
    2MASS, UCAC4, GSC1, GSC2 and GAIA-DR1.

    racen: right-ascension of the field
    deccen: declination of the field center
    rarange: Ra-range (in arcminutes) around the field center
    decrange: Dec-range (in arcminutes) around the field center
    catalogue: The standard star catalogue to be queried

    The function returns a numpy triplet with the columns Ra, Dec and
    magnitude

    >>> query = get_standard_cat(210.0, -10.0, 10, 10, '2MASS')
    """

    querycat =""
    querycat = 'II/246/out'
    racol = '_RAJ2000'
    deccol = '_DEJ2000'

    # check for not implemented catalogue:
    if querycat == "":
        raise ValueError

    # define columns to be retrieved and circumvent the 50 objects
    # limit in standard astroquery queries.
    v = av.Vizier(columns=[ racol, deccol, 'Jmag', 'Hmag', 'Kmag', 'e_Jmag', 'e_Hmag', 'e_Kmag', 'Qflg', 'Rflg', 'Bflg', 'Cflg', 'Xflg', 'Aflg' ])

    v.ROW_LIMIT = 99999999999

    result =  v.query_region(
        coord.SkyCoord(ra=racen, dec=deccen, unit=(au.deg, au.deg)),
        width=rarange * au.arcmin,
        height=decrange * au.arcmin,
        catalog=querycat)

    return result[0]

if __name__ == "__main__":
    # Handle command line arguments
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
    SCRIPT NAME:
      get_standard_cat.py - retrieve position and magnitudes fron Vizier

    DESCRIPTION:
      The program retrieves positions and magnitudes from astronomical
      standardstar catalogues from Vizier. The script currently
      supports 2MASS, UCAC4, GSC1, GSC2 and GAIA-DR1.

    EXAMPLES:
      - get_standard_cat,py -r 210.0 -d -10.0 -c 2MASS \\
                            -o 2MASS.cat

        This retrieves sources from 2MASS in a box of 10 square arcmintues
        around Ra=210.0, Dec=-10.0.
        We create an LDAC-catalogue with a Table 2MASS and the columns
        Ra, Dec and Mag.
        The obtained magnitude depends on the catalogue. Please see
        the catalogue descriptions on Vizier and the source code
        of this script.

      - get_standard_cat,py -r 210.0 -d -10.0 -c GAIA-DR1 \\
                            --ra-range 20.0 --dec-range 10.0 \\
                            -o GAIA.cat.cat

        This retrieves GAIA-DR1 sources in a box of 20'x10' around
        Ra=210.0 and Dec=-10.0. The source table within GAIA.cat
        hast he name GAIA-DR1.

    AUTHOR:
      Thomas Erben (terben@astro.uni-bonn.de
    """
    )
    parser.add_argument('-r', '--ra', help='Ra centre of catalogue',
                        type=float, required=True)
    parser.add_argument('-d', '--dec', help='Dec centre of catalogue',
                        type=float, required=True)
    parser.add_argument('--ra-range', help='boxsize in Ra (arcmin)',
                        type=float, default=10.0)
    parser.add_argument('--dec-range', help='boxsize in Dec (arcmin)',
                        type=float, default=10.0)
    parser.add_argument('-o', '--output_catalogue', help='output file name',
                        required=True)

    args = parser.parse_args()

    # read command line arguments:
    racen = args.ra
    deccen = args.dec
    rarange = args.ra_range
    decrange = args.dec_range
    outcat = args.output_catalogue

    result = get_standard_cat(racen, deccen, rarange, decrange)

    Qflg=np.zeros(len(result['Qflg']), dtype=np.int)
    for i in range(len(result['Qflg'])):
        result['Qflg'][i] = result['Qflg'][i].replace('A','1')
        result['Qflg'][i] = result['Qflg'][i].replace('B','2')
        result['Qflg'][i] = result['Qflg'][i].replace('C','3')
        result['Qflg'][i] = result['Qflg'][i].replace('D','4')
        result['Qflg'][i] = result['Qflg'][i].replace('E','5')
        result['Qflg'][i] = result['Qflg'][i].replace('F','6')
        result['Qflg'][i] = result['Qflg'][i].replace('U','7')
        result['Qflg'][i] = result['Qflg'][i].replace('X','8')
        Qflg[i]=int(result['Qflg'][i])
    
    Rflg=np.zeros(len(result['Rflg']), dtype=np.int)
    for i in range(len(result['Rflg'])):
        result['Rflg'][i] = result['Rflg'][i].replace('4','5')
        result['Rflg'][i] = result['Rflg'][i].replace('3','4')
        result['Rflg'][i] = result['Rflg'][i].replace('2','3')
        result['Rflg'][i] = result['Rflg'][i].replace('1','2')
        result['Rflg'][i] = result['Rflg'][i].replace('0','1')
        Rflg[i]=int(result['Rflg'][i])

    Bflg=np.zeros(len(result['Bflg']), dtype=np.int)
    for i in range(len(result['Bflg'])):
        result['Bflg'][i] = result['Bflg'][i].replace('7','8')
        result['Bflg'][i] = result['Bflg'][i].replace('6','7')
        result['Bflg'][i] = result['Bflg'][i].replace('5','6')
        result['Bflg'][i] = result['Bflg'][i].replace('4','5')
        result['Bflg'][i] = result['Bflg'][i].replace('3','4')
        result['Bflg'][i] = result['Bflg'][i].replace('2','3')
        result['Bflg'][i] = result['Bflg'][i].replace('1','2')
        result['Bflg'][i] = result['Bflg'][i].replace('0','1')
        Bflg[i]=int(result['Bflg'][i])

    Cflg=np.zeros(len(result['Cflg']), dtype=np.int)
    for i in range(len(result['Cflg'])):
        result['Cflg'][i] = result['Cflg'][i].replace('0','1')
        result['Cflg'][i] = result['Cflg'][i].replace('p','2')
        result['Cflg'][i] = result['Cflg'][i].replace('c','3')
        result['Cflg'][i] = result['Cflg'][i].replace('d','4')
        result['Cflg'][i] = result['Cflg'][i].replace('s','5')
        result['Cflg'][i] = result['Cflg'][i].replace('b','6')
        Cflg[i]=int(result['Cflg'][i])
    
    Xflg=np.zeros(len(result['Xflg']), dtype=np.int)
    for i in range(len(result['Xflg'])):
        Xflg[i]=int(result['Xflg'][i])

    Aflg=np.zeros(len(result['Aflg']), dtype=np.int)
    for i in range(len(result['Aflg'])):
        Aflg[i]=int(result['Aflg'][i])

    ldactab = ldac.LDACTable()
    ldactab.setname('OBJECTS')
    ldactab['RAJ2000'] = result['_RAJ2000']
    ldactab['DECJ2000'] = result['_DEJ2000']
    ldactab['Jmag'] = result['Jmag']
    ldactab['Hmag'] = result['Hmag']
    ldactab['Kmag'] = result['Kmag']
    ldactab['eJmag'] = result['e_Jmag']
    ldactab['eHmag'] = result['e_Hmag']
    ldactab['eKmag'] = result['e_Kmag']
    ldactab['Qflg'] = Qflg
    ldactab['Rflg'] = Rflg
    ldactab['Bflg'] = Bflg
    ldactab['Cflg'] = Cflg
    ldactab['Xflg'] = Xflg
    ldactab['Aflg'] = Aflg
    
    if os.path.exists(outcat):
        os.remove(outcat)
    ldactab.saveas(outcat)
