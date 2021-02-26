#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#include <fcntl.h>
#endif
#include "utils_defs.h"

/* 15.07.01:
 * added an explicit cast in ED_GETVEC to avoid compiler
 * C++ warnings.
 * Added an explicit (size_t) cast in ED_FREAD and ED_FWRITE
 * for the same purpose.
 *
 * 09.12.2002:
 * removed the close_cat call at the end of the
 * ED_GETKEY macro. This is moved to the read_key
 * function. This was necessary as the read_key
 * function not always really opens a file but the
 * close_cat function always closes one.
 *
 * 19.10.2004:
 * removed the superfluous inclusion of malloc.h
 */

#define ED_MALLOC(ptr, func, type, nmbr) \
          if ((ptr = (type *)malloc(sizeof(type)*nmbr)) == NULL) \
             syserr(FATAL, func, \
              "Mem alloc error with %d elements\n",\
                 nmbr)
#define ED_CALLOC(ptr, func, type, nmbr) \
          if ((ptr = (type *)calloc(nmbr,sizeof(type))) == NULL) \
             syserr(FATAL, func, \
              "Mem calloc error with %d elements\n",\
                 nmbr)

#define ED_REALLOC(ptr, func, type, nmbr) \
          if ((ptr = (type *)realloc(ptr,sizeof(type)*nmbr)) == NULL) \
             syserr(FATAL, func, \
              "Mem realloc error with %d elements\n",\
                 nmbr)

#define ED_FREE(ptr) \
          if (ptr != NULL) {free(ptr); ptr=NULL;}

#define ED_CHECK_TYPE(ptr, func, type, tname) \
          if (ptr->ttype != type) \
             syserr(FATAL, func, \
              "Key %s should be of type %s\n", ptr->name, tname)

#define ED_COPY_PTR(lptr, func, key, type, tname) \
          {if (key->ttype != type) \
             syserr(FATAL, func, \
              "Key %s should be of type %s\n", key->name, tname); \
                 lptr = key->ptr; }

#ifdef WIN32
#define ED_FCHECK(func, file) \
          _access(file, 0)
#else
#define ED_FCHECK(func, file) \
          access(file, F_OK)
#endif

#define ED_FOPEN(ptr, func, file, mode) \
          if ((ptr = fopen(file, mode)) == NULL) \
             syserr(FATAL, func, \
              "File open error for %s in %s\n",mode,file)

#define ED_OPEN(ptr, func, file, flag, mode) \
          if ((ptr = open(file, flag, mode)) < 0) \
             syserr(FATAL, func, \
              "File open error for %s in %s\n",flag,file)

#define ED_FREAD(ptr, func, fd, size) \
          if (fread(ptr, 1, size, fd) != (size_t)size) \
             syserr(FATAL, func, \
              "File read error for %s bytes\n",size)

#define ED_READ(ptr, func, fd, size) \
          if (read(fd, ptr, size) != size) \
             syserr(FATAL, func, \
              "File read error for %s bytes\n",size)

#define ED_FWRITE(ptr, func, fd, size) \
          if (fwrite(ptr, 1, size, fd) != (size_t)size) \
             syserr(FATAL, func, \
              "File read error for %s bytes\n",size)

#define ED_WRITE(ptr, func, fd, size) \
          if (write(fd, ptr, size) != size) \
             syserr(FATAL, func, \
              "File read error for %s bytes\n",size)

#define ED_FCLOSE(ptr, func) \
          if (fclose(ptr) != 0) \
             syserr(FATAL, func, \
               "File close error\n")

#define ED_CLOSE(ptr, func) \
          if (close(ptr) != 0) \
             syserr(FATAL, func, \
               "File close error\n")

#define ED_FSEEK(ptr, func, place) \
          if (fseek(ptr, place, SEEK_SET) != 0) \
             syserr(FATAL, func, \
                "File seek error\n")

#define ED_CHECK_KEY(key, prog, typea, typeb) \
          if (key->ttype != typea  && key->ttype != typeb) \
             syserr(FATAL, prog, \
              "Key %s has incorrect type.\n", key->name)

#define ED_GETKEY(tab, key, keyname, dest, nmbr, prog) \
          {if (!(key = read_key(tab, keyname))) \
               syserr(FATAL, prog, "No Key %s in Table\n",keyname); \
              dest = key->ptr; nmbr = key->nobj; }

#define ED_GETVEC(dest, func, key, keyname, totype, fromtype) \
              { int i, esize; char *ptr; \
                i = strlen(keyname); \
                ptr = keyname; ptr +=i+1; i=(int) *ptr;\
                esize = t_size[key->ttype]; \
                if (!(dest= (totype *)calloc(key->nobj,sizeof(totype)))) \
                      syserr(FATAL, func, \
                      "Mem calloc error with %d elements\n",key->nobj); \
                ptr = (char *)key->ptr; if (i>1) ptr += esize *(i-1); \
                for (i=0;i<key->nobj;i++) { \
                   dest[i] = (totype) *(fromtype *)ptr;\
                   ptr += key->nbytes; \
                } }\

#define ED_GETFIELDVEC(dest, func, key, keyname, nobj, totype, fromtype) \
              { int i, esize; char *ptr; \
                i = strlen(keyname); \
                ptr = keyname; ptr +=i+1; i=(int) *ptr;\
                esize = t_size[key->ttype]; \
                if (!(dest= (totype *)calloc(nobj,sizeof(totype)))) \
                      syserr(FATAL, func, \
                      "Mem calloc error with %d elements\n",nobj); \
                ptr = key->ptr; if (i>1) ptr += esize *(i-1); \
                for (i=0;i<nobj;i++) { \
                   dest[i] = (totype) *(fromtype *)ptr;\
                   ptr += key->nbytes; \
                } }\


#define ED_PUTVEC(orig, func, key, keyname, totype, fromtype) \
              { int i, esize; char *ptr;\
                i = strlen(keyname); \
                ptr = keyname; ptr +=i+1; i=(int) *ptr;\
                esize = t_size[key->ttype]; \
                ptr = key->ptr; if (i>1) ptr += esize *(i-1); \
                for (i=0;i<key->nobj;i++) { \
                   *(totype *)ptr = (totype) orig[i];\
                   ptr += key->nbytes; \
                } }\


#define ABS(a) (a)<0 ? -((a)) : (a)
#define MAX(x,y) ((x)>(y)?(x):(y))
#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#define RINT(x) (int)(floor(x+0.5))
