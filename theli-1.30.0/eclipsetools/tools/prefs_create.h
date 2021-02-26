#ifndef _PREFS_H_
#define _PREFS_H_

#define MAXKEYWORD              999
#define MAXIMAGE                100
#ifndef MAXCHAR
#define MAXCHAR                 256
#endif

#define QMALLOC(ptr, typ, nel) \
                {if (!(ptr = (typ *)malloc((size_t)(nel)*sizeof(typ)))) \
                  error(1, "Not enough memory for ", \
                        #ptr " (" #nel " elements) !");;}

static const char		notokstr[] = {" \t=,;\n\r\""};

/* prefs stuff */
typedef struct
{
  char		prefs_name[MAXCHAR];      /* prefs filename */
  char          output[MAXCHAR];          /* output image name */
  char		*(header_keys[MAXKEYWORD]); /* header keys to transfer */
  int           nheader_keys;	          /* number of header keys to transger */
  char          *(image_name[MAXIMAGE]);  /* names of input images */
  int           nimage_name;              /* number of input images */
  int           header_dummy;             /* number of dummy keywords for the main header */
}	prefstruct;

extern prefstruct	prefs;


/* function definitions */
extern int	cistrcmp(char *cs, char *ct, int mode);

extern void	dumpprefs(void);
extern void     readprefs(char *filename,char **argkey,char **argval,int narg);

extern void    error(int, char *, char *);
extern void    warning(char *msg1, char *msg2);

#endif






