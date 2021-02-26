/*
   16.01.2005:
   I removed compiler warnings (gcc with -W -pedantic -Wall)

   07.02.2005:
   I included two macros: IMCAT_CALLOC and IMCAT_FREE that call
   calloc and free and automatically check the result.
*/

#define IMCAT_CALLOC(ptr, type, nmbr) \
          if ((ptr = (type *)calloc(nmbr,sizeof(type))) == NULL) \
             error_exit("Mem calloc failure\n");

#define IMCAT_FREE(ptr) \
          if (ptr != NULL) {free(ptr); ptr=NULL;}


void	*checkalloc(int nel, int size);
void	allocShortArray(short ***f, int N1, int N2);
void	allocFloatArray(float ***f, int N1, int N2);
void	freeShortArray(short **f);
void	freeFloatArray(float **f);
void	copyFloatToShort(float **fsrc, short **fdst, int N1, int N2);
void	copyShortToFloat(short **fsrc, float **fdst, int N1, int N2);
float	***alloc3DFloatArray(int N1, int N2, int N3);
void	free3DFloatArray(float ***f, int N3);


