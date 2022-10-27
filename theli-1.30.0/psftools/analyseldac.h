/* changed the i and j parameters from int to long (TE: 14.12.1998)
   included types.h (16.03.1999)
   changed arguments in zap function to enable it again.
   (TE: 20.03.1999)

   expanded the argument list for do_object_stats to match the
   updates in analyseldac.c (to enable the use of object positions
   in the input catalog instead of reestimating the centroid in
   do_object_stats)
*/

#include "utils_globals.h"

void		dosky(				float **f,
								int N1,
								int N2,
								LONG i,
								LONG j,
								int a1,
								int a2,
								skyquad * sky,
								float fmode,
								float sigma);
void		do_object_stats(	int anumber,
                                        LONG aipos,
                                        LONG ajpos,
					float arg,
					float **f,
					int N1,
					int N2,
					float(*fsky)(int i, int j),
					float sigma,
					float ne,
					float rc,
					float alpha,
                                        float goodxpos,
                                        float goodypos);

int			objectcmp(object *pk1, object *pk2);
int			shortcmp(short *s1, short *s2);
float		fsky(int di, int dj);
void		setskyparameters(skyquad *sky);
void		zap(int zapmode, LONG x, LONG y, double rg, int radiustype,
                float a, float **f, float **fzap, short **nzap, int N1, int N2);










