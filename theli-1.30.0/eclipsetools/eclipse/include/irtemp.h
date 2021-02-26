


/*

 $Id: irtemp.h,v 1.1 2003/09/04 15:44:42 terben Exp $
 $Author: terben $
 $Date: 2003/09/04 15:44:42 $
 $Revision: 1.1 $
 $Log: irtemp.h,v $
 Revision 1.1  2003/09/04 15:44:42  terben
 first checked in version of preprocessing tools based on
 ESO's qfits and eclipse libraries. For now, there
 are two tools: One to split multi-extension FITS files and
 to perform header updates at the same time. The other
 to perform data preprocessing (overscan correction, trimming,
 bias subtraction, flat fielding, fringe correction).
 In contrast to the FLIPS tools, all aoperations are done
 keeping data in memory.

 Revision 1.3  2001/07/31 06:54:46  ndevilla
 Switched constant arrays to const.

 Revision 1.2  2001/03/06 17:13:04  ndevilla
 Queries into the star database now return pointers to the static
 internal structure, instead of copying their informations. Memory
 usage has been reduced A LOT.
 The star now contain the source from which they were taken (catalog name).
 This is not yet used by customers of this module.

 The table conversion tool has been updated correspondingly.

 Revision 1.1  1999/05/04 14:39:16  isaacp
 Initial revision


 */

#ifndef _IRTEMP_H_
#define _IRTEMP_H_


#define IRSTD_NTEMP			44


static const sptype_temp irstd_temperature_table[IRSTD_NTEMP] =
{
{ "O5V", 40000 },
{ "B0V", 27000 },
{ "B0.5V", 25000 },
{ "B1V", 22000 },
{ "B2V", 19000 },
{ "B3V", 16000 },
{ "B4V", 15000 },
{ "B5V", 14000 },
{ "B6V", 13000 },
{ "B7V", 12500 },
{ "B8V", 12000 },
{ "B9V", 11000 },
{ "A0V", 10000 },
{ "A1V", 9500 },
{ "A2V", 9000 },
{ "A5V", 8500 },
{ "A7V", 8000 },
{ "F0V", 7500 },
{ "F2V", 7000 },
{ "F5V", 6500 },
{ "G0V", 6000 },
{ "G5V", 5520 },
{ "G6V", 5500 },
{ "K0V", 4900 },
{ "K4V", 4500 },
{ "K5V", 4130 },
{ "K7V", 4000 },
{ "M0V", 3750 },
{ "M2V", 3500 },
{ "M5V", 2800 },
{ "M8V", 2400 },
{ "G0III", 5600 },
{ "G5III", 5000 },
{ "K0III", 4500 },
{ "K5III", 3800 },
{ "M0III", 3200 },
{ "B0I", 30000 },
{ "A0I", 12000 },
{ "F0I", 7000 },
{ "G0I", 5700 },
{ "G5I", 4850 },
{ "K0I", 4100 },
{ "K5I", 3500 },
{ 0, 0 }
} ;


#endif

