/*----------------------------------------------------------------------------

   File name    :   ic.c
   Author       :   Nick Kaiser & Thomas Erben
   Created on   :   01.01.2000
   Description  :   perform calculations on FITS images

 ---------------------------------------------------------------------------*/

/*
        $Id: ic.c,v 1.8 2018/03/28 12:06:14 thomas Exp $
        $Author: thomas $
        $Date: 2018/03/28 12:06:14 $
        $Revision: 1.8 $
*/

/*----------------------------------------------------------------------------
                                History Coments
 ---------------------------------------------------------------------------*/

/*
   24.01.2005:
   I changed exit(0) to return 0 in the main program
   to avoid compiler warnings

   28.01.2005:
   I added HISTORY information to the produced FITS
   images

   06.03.2005:
   I added the input images to the HISTORY information
   in the output image.

   09.03.2005:
   - I added memory tracing via xmemory
   - I removed memory leaks at the end of the program.

   02.08.2005:
   I added some comments to the code

   12.02.2011:
   I added the boolean operators 'and' and 'or'
   (operators that take two arguments)

   03.04.2013:
   xmemory is no longer used by default but only if the variable
   __XMEM_DEBUG__ is defined at compile time. Otherwise, regular malloc
   etc. calls are used for memory management.
 */

/*----------------------------------------------------------------------------
                    Includes, Defines and Global Variables
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


#include "fits.h"
#include "error.h"
#include "iostream.h"
#include "getop.h"
#include "ic.h"
#include "global_vers.h"
#include "t_iso8601.h"

#ifdef __XMEM_DEBUG__
#include "xmemory.h"
#endif

/* uncomment the following line to obtain a status on allocated/freed
   memory at the end of the program. Only available if also
   __XMEM_DEBUG__ is defined at compile time.
 */

/* #define MEMORY_DEBUG */

#define LOOP for (ix = 0; ix < N1; ix++)

#define MAX_OPS 1024
#define STACK_DEPTH 100
double *st[STACK_DEPTH];
int     pos;

int        nim = 0;
static int ix, iy, N1, N2;
float **   f;

#define MAX_NEW_COMMENTS    100

#define MAGIC               FLOAT_MAGIC

#ifndef MAXCHAR
#define MAXCHAR             256
#endif

/*----------------------------------------------------------------------------
                           Function declarations
 ---------------------------------------------------------------------------*/

double  drand48();
void printUsage(void);

int main(int argc, char *argv[])
{
  int arg, argccopy, M1, M2, im, iop, nop = 0, idim;
  /* the command line argument where image names start */
  int imagearg;
  int outputpixtype, *badpix;
  char *rpnexpr, rpnstring[128], *argvcopy[100];
  char tmpstring[MAXCHAR];
  float *fout;
  float magicsub;
  iostream **ipstream;
  int createimage;
  op *theop, *oplist[MAX_OPS];
  long seed;
  double *tempdblptr;
  char *newcommentname[MAX_NEW_COMMENTS];
  char *newcommentval[MAX_NEW_COMMENTS];
  int ncomments, icom, externbscaling;
  double bscale, bzero;
  fitsheader **fits, *fitsout;
  fitscomment *com;

  /* defaults */
  createimage    = 0;
  outputpixtype  = 0;
  magicsub       = MAGIC;
  ncomments      = 0;
  externbscaling = 0;

  /* start the args copy */
  arg = argccopy = 0;
  argvcopy[argccopy++] = argv[arg++];

  /* parse the args */
  if (argc < 2) {
    printUsage();
    exit(1);
  }
  while (arg < argc) {
    if (argv[arg][0] != '-') {                  /* it must be the rpnexpr */
      break;
    } else {
      switch (argv[arg++][1]) {
      case 'c':
        createimage = 1;
        argvcopy[argccopy++] = argv[arg - 1];
        argvcopy[argccopy++] = argv[arg];
        sscanf(argv[arg++], "%d", &N1);
        argvcopy[argccopy++] = argv[arg];
        sscanf(argv[arg++], "%d", &N2);
        break;
      case 'p':
        argvcopy[argccopy++] = argv[arg - 1];
        argvcopy[argccopy++] = argv[arg];
        sscanf(argv[arg++], "%d", &outputpixtype);
        if (outputpixtype != UCHAR_PIXTYPE &&
            outputpixtype != SHORT_PIXTYPE &&
            outputpixtype != FLOAT_PIXTYPE &&
            outputpixtype != DBL_PIXTYPE &&
            outputpixtype != INT_PIXTYPE) {
          error_exit("ic: illegal pixtype\n");
        }
        break;
      case 's':
        argvcopy[argccopy++] = argv[arg - 1];
        argvcopy[argccopy++] = argv[arg];
        sscanf(argv[arg++], "%ld", &seed);
        srand48(seed);
        break;
      case 'm':
        argvcopy[argccopy++] = argv[arg - 1];
        argvcopy[argccopy++] = argv[arg];
        sscanf(argv[arg++], "%f", &magicsub);
        break;
      case 'h':
        newcommentname[ncomments] = argv[arg++];
        newcommentval[ncomments] = argv[arg++];
        ncomments++;
        break;
      case 'b':
        externbscaling = 1;
        argvcopy[argccopy++] = argv[arg - 1];
        argvcopy[argccopy++] = argv[arg];
        sscanf(argv[arg++], "%lf", &bscale);
        argvcopy[argccopy++] = argv[arg];
        sscanf(argv[arg++], "%lf", &bzero);
        break;
      case 'u':
      default:
        printUsage();
        exit(1);
        break;
      }
    }
  }

  /* no expression to evaluate was given */
  if (argc == arg) {
    printUsage();
    exit(1);
  }

  rpnexpr = argv[arg];
  sprintf(rpnstring, "'%.68s'", rpnexpr);
  argvcopy[argccopy++] = rpnstring;
  arg++;

  /* set BITPIX=-32 as default for result images if
     we have no input image
   */
  if (!outputpixtype && createimage) {
    outputpixtype = FLOAT_PIXTYPE;
  }


  /* now read the image headers and check they are same size */
  nim = argc - arg;  /* the number of input images */
  imagearg = arg;
  if (nim && createimage) {
    printUsage();
    exit(1);
  }
  ipstream = (iostream **)calloc(nim, sizeof(iostream *));
  fits = (fitsheader **)calloc(nim, sizeof(fitsheader *));
  f = (float **)calloc(nim, sizeof(float *));
  for (im = 0; im < nim; im++) {
    ipstream[im] = openiostream(argv[arg++], "r");

    fits[im] = readfitsheader(ipstream[im]->f);
    if (im == 0) {
      N1 = fits[im]->n[0];
      N2 = 1;
      for (idim = 1; idim < fits[im]->ndim; idim++) {
        N2 *= fits[im]->n[idim];
      }
      if (!outputpixtype) {
        outputpixtype = fits[im]->extpixtype;
      }
    } else {
      M1 = fits[im]->n[0];
      M2 = 1;
      for (idim = 1; idim < fits[im]->ndim; idim++) {
        M2 *= fits[im]->n[idim];
      }
      if ((M1 != N1) || (M2 != N2)) {
        error_exit("ic: images have non-identical size or pixtype\n");
      }
    }
  }

  /* allocate output image */
  if (nim) {
    fitsout = copyfitsheader(fits[0]);
    fitsout->extpixtype = outputpixtype;
  } else {
    fitsout = new2Dfitsheader(N1, N2, outputpixtype);
  }
  if (externbscaling) {
    fitsout->bscaling = 1;
    fitsout->bscale   = bscale;
    fitsout->bzero    = bzero;
  }

  fout = (float *)calloc(N1, sizeof(float));

  /* set internal value of image width */
  set_getop_N1(N1);
  /* allocate space for badpix */
  badpix = (int *)calloc(N1, sizeof(int));
  /* allocate the data space */
  for (pos = 0; pos < STACK_DEPTH; pos++) {
    st[pos] = (double *)calloc(N1, sizeof(double));
  }

  /* now parse the rpnstring */
  while ((theop = getop(&rpnexpr))) {
    oplist[nop++] = theop;
    if (nop == MAX_OPS) {
      error_exit("ic: too many operators\n");
    }
  }

  /* and add any other comments */
  for (icom = 0; icom < ncomments; icom++) {
    if (strcmp(newcommentname[icom], "HISTORY") &&
        strcmp(newcommentname[icom], "COMMENT")) {
      removenamedcomments(newcommentname[icom], fitsout);
    }
    com = newtextcomment(newcommentname[icom], newcommentval[icom], NULL);
    appendcomment(com, fitsout);
  }

  /* add the history comments */
  appendcomment(newtextcomment("HISTORY", "", NULL), fitsout);
  snprintf(tmpstring, MAXCHAR - 1, "ic: %s", __PIPEVERS__);
  appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);
  snprintf(tmpstring, MAXCHAR - 1, "ic: called at %s", get_datetime_iso8601());
  appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);
  add_comment(argccopy, argvcopy, fitsout);
  if (nim > 0) {
    sprintf(tmpstring, "ic: inputimages:");
    appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);
    for (im = 0; im < nim; im++) {
      snprintf(tmpstring, MAXCHAR - 1, "ic: %%%d: %s", im + 1,
               get_basename(argv[imagearg++]));
      appendcomment(newtextcomment("HISTORY", tmpstring, NULL), fitsout);
    }
  }
  writefitsheader(fitsout);

  /* allocate space for the image lines */
  for (im = 0; im < nim; im++) {
    f[im] = (float *)calloc(N1, sizeof(float));
  }

  /* now do the biz... */
  pos = -1;
  for (iy = 0; iy < N2; iy++) {
    for (im = 0; im < nim; im++) {
      readfitsline(f[im], fits[im]);
    }
    LOOP { badpix[ix] = 0; };
    for (iop = 0; iop < nop; iop++) {
      theop = oplist[iop];
      switch (theop->type) {
      case CONSTANT_TYPE:
        pos++;
        if (pos >= STACK_DEPTH) {
          fprintf(stderr, "ic: st full\n");
          exit(-1);
        }
        LOOP { st[pos][ix] = theop->data[ix]; };

        /* if (st[pos] == MAGIC)
                badpix = 1;*/
        break;
      case IM_VALUE_TYPE:
        pos++;
        if (pos >= STACK_DEPTH) {
          fprintf(stderr, "ic: st full\n");
          exit(-1);
        }
        LOOP {
          /* st[pos][ix] = f[theop->opno][iy][ix]; */
          st[pos][ix] = f[theop->opno][ix];
          if (st[pos][ix] == MAGIC) {
            badpix[ix] = 1;
          }
        }
        break;
      case NUM0_FUNC_TYPE:
        pos++;
        if (pos >= STACK_DEPTH) {
          fprintf(stderr, "ic: st full\n");
          exit(-1);
        }
        switch (theop->opno) {
        case RAND:
          LOOP { st[pos][ix] = drand48(); }
          break;
        case X:
          LOOP { st[pos][ix] = x(); }
          break;
        case Y:
          LOOP { st[pos][ix] = y(); }
          break;
        case XP:
          LOOP { st[pos][ix] = xp(); }
          break;
        case YP:
          LOOP { st[pos][ix] = yp(); }
          break;
        case IF:
        case IFQ:
          pos -= 3;
          if (pos < 0) {
            fprintf(stderr, "ic: stack empty\n");
            exit(-1);
          }
          LOOP { st[pos][ix] = (double)(st[pos + 2][ix] ? st[pos][ix] :
                                        st[pos + 1][ix]); }
          break;
        case ENTER:
          LOOP { st[pos][ix] = st[pos - 1][ix]; }
          break;
        case GRAND:
          LOOP { st[pos][ix] = gasdev(); }
          break;
        default:
          fprintf(stderr, "ic: bad opno\n");
          exit(-1);
          break;
        }
        break;
      case NUM1_FUNC_TYPE:
        switch (theop->opno) {
        case EXP:
          LOOP { st[pos][ix] = exp(st[pos][ix]); }
          break;
        case LOG:
          LOOP { st[pos][ix] = log(st[pos][ix]); }
          break;
        case LOG10:
          LOOP { st[pos][ix] = log10(st[pos][ix]); }
          break;
        case FABS:
          LOOP { st[pos][ix] = fabs(st[pos][ix]); }
          break;
        case NOT:
          LOOP { st[pos][ix] = (double)(!((int)st[pos][ix])); }
          break;
        case SQRT:
          LOOP { st[pos][ix] = sqrt(st[pos][ix]); }
          break;
        case FLOOR:
          LOOP { st[pos][ix] = floor(st[pos][ix]); }
          break;
        case CEIL:
          LOOP { st[pos][ix] = ceil(st[pos][ix]); }
          break;
        case SIN:
          LOOP { st[pos][ix] = sin(st[pos][ix]); }
          break;
        case ASIN:
          LOOP { st[pos][ix] = asin(st[pos][ix]); }
          break;
        case SINH:
          LOOP { st[pos][ix] = sinh(st[pos][ix]); }
          break;
        case COS:
          LOOP { st[pos][ix] = cos(st[pos][ix]); }
          break;
        case ACOS:
          LOOP { st[pos][ix] = acos(st[pos][ix]); }
          break;
        case COSH:
          LOOP { st[pos][ix] = cosh(st[pos][ix]); }
          break;
        case TAN:
          LOOP { st[pos][ix] = tan(st[pos][ix]); }
          break;
        case ATAN:
          LOOP { st[pos][ix] = atan(st[pos][ix]); }
          break;
        case TANH:
          LOOP { st[pos][ix] = tanh(st[pos][ix]); }
          break;
        case BESSJ0:
          LOOP { st[pos][ix] = j0(st[pos][ix]); }
          break;
        case BESSJ1:
          LOOP { st[pos][ix] = j1(st[pos][ix]); }
          break;
        case BESSY0:
          LOOP { st[pos][ix] = y0(st[pos][ix]); }
          break;
        case BESSY1:
          LOOP { st[pos][ix] = y1(st[pos][ix]); }
          break;
        case SWAP:
          if (pos < 1) {
            fprintf(stderr, "ic: stack empty\n");
            exit(-1);
          }
          tempdblptr = st[pos];
          st[pos] = st[pos - 1];
          st[pos - 1] = tempdblptr;
          break;
        default:
          fprintf(stderr, "ic: bad opno\n");
          exit(-1);
          break;
        }
        break;
      case NUM2_FUNC_TYPE:
        pos--;
        if (pos < 0) {
          fprintf(stderr, "ic: st empty\n");
          exit(-1);
        }
        switch (theop->opno) {
        case TIMES0:
        case TIMES1:
          LOOP { st[pos][ix] = st[pos][ix] * st[pos + 1][ix]; }
          break;
        case PLUS:
          LOOP { st[pos][ix] = st[pos][ix] + st[pos + 1][ix]; }
          break;
        case MINUS:
          LOOP { st[pos][ix] = st[pos][ix] - st[pos + 1][ix]; }
          break;
        case DIVIDE:
          LOOP { st[pos][ix] = st[pos][ix] / st[pos + 1][ix]; }
          break;
        case POW:
          LOOP { st[pos][ix] = pow(st[pos][ix], st[pos + 1][ix]); }
          break;
        case GT:
          LOOP { st[pos][ix] = (double)(st[pos][ix] > st[pos + 1][ix]); }
          break;
        case GE:
          LOOP { st[pos][ix] = (double)(st[pos][ix] >= st[pos + 1][ix]); }
          break;
        case LT:
          LOOP { st[pos][ix] = (double)(st[pos][ix] < st[pos + 1][ix]); }
          break;
        case LE:
          LOOP { st[pos][ix] = (double)(st[pos][ix] <= st[pos + 1][ix]); }
          break;
        case EQ:
          LOOP { st[pos][ix] = (double)(st[pos][ix] == st[pos + 1][ix]); }
          break;
        case NE:
          LOOP { st[pos][ix] = (double)(st[pos][ix] != st[pos + 1][ix]); }
          break;
        case FMOD:
          LOOP { st[pos][ix] = fmod(st[pos][ix], st[pos + 1][ix]); }
          break;
        case ATAN2:
          LOOP { st[pos][ix] = atan2(st[pos][ix], st[pos + 1][ix]); }
          break;
        case BESSJN:
          LOOP { st[pos][ix] = jn((int)st[pos][ix], st[pos + 1][ix]); }
          break;
        case BESSYN:
          LOOP { st[pos][ix] = yn((int)st[pos][ix], st[pos + 1][ix]); }
          break;
        case MAX:
          LOOP { st[pos][ix] = (st[pos][ix] > st[pos + 1][ix] ?
                                st[pos][ix] : st[pos + 1][ix]); }
          break;
        case MIN:
          LOOP { st[pos][ix] = (st[pos][ix] < st[pos + 1][ix] ?
                                st[pos][ix] : st[pos + 1][ix]); }
          break;
        case AND:
          LOOP { st[pos][ix] = (double)(st[pos][ix] =
                                        st[pos][ix] && st[pos + 1][ix]); }
          break;
        case OR:
          LOOP { st[pos][ix] = (double)(st[pos][ix] =
                                        st[pos][ix] || st[pos + 1][ix]); }
          break;
        default:
          fprintf(stderr, "ic: bad opno\n");
          exit(-1);
          break;
        }
        break;
      default:
        fprintf(stderr, "ic: bad op type\n");
        exit(-1);
        break;
      }
    }
    LOOP { fout[ix] = (badpix[ix] ? magicsub : st[pos][ix]); }
    writefitsline(fout, fitsout);
    pos--;
  }
  writefitstail(fitsout);

  /* free memory */
  for (pos = 0; pos < STACK_DEPTH; pos++) {
    free(st[pos]);
  }

  free(badpix);
  free(fout);

  delfitsheader(fitsout);

  if (nim > 0) {
    for (im = 0; im < nim; im++) {
      delfitsheader(fits[im]);
      free(f[im]);
    }
    free(ipstream);
    free(fits);
    free(f);
  }

#ifdef MEMORY_DEBUG
  xmemory_status();
#endif

  return(0);
}

double  xp(void)
{
  return((double)ix);
}

double  yp(void)
{
  return((double)iy);
}

double  x(void)
{
  return((double)ix / (double)N1);
}

double  y(void)
{
  return((double)iy / (double)N2);
}

double gasdev(void)
{
  static int    iset = 0;
  static double gset;
  double        fac, r, v1, v2;
  double ran0();

  if (iset == 0) {
    do {
      v1 = 2.0 * drand48() - 1.0;
      v2 = 2.0 * drand48() - 1.0;
      r  = v1 * v1 + v2 * v2;
    } while (r >= 1.0);
    fac  = sqrt(-2.0 * log(r) / r);
    gset = v1 * fac;
    iset = 1;
    return(v2 * fac);
  } else {
    iset = 0;
    return(gset);
  }
}

/*
 * This function only gives the usage message for the program
 */

void printUsage(void)
{
  fprintf(stdout, "PROGRAMNAME\n");
  fprintf(stdout, "	ic -- image calculator\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "SYNOPSIS\n");
  fprintf(stdout, "	ic [options....] rpnexpr fitsfile... \n");
  fprintf(stdout, "	where options are:\n");
  fprintf(stdout, "		-u    # print usage\n");
  fprintf(stdout, "		-c N1 N2	# create an image of this size\n");
  fprintf(stdout, "		-p pixtype	# specify output pixtype\n");
  fprintf(stdout, "		-s seed		# seed rand num generator\n");
  fprintf(stdout, "		-m magicsub	# substitute magic value\n");
  fprintf(stdout, "		-h name val	# add header comment 'name = val'\n");
  fprintf(stdout, "		-b BSCALE BZERO	# add scaling header information\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "DESCRIPTION \n");
  fprintf(stdout, "	'ic' does arithmetic on one or more images according to the\n");
  fprintf(stdout, "	reverse-polish notation expression 'rpnexpr'.  Images are\n");
  fprintf(stdout, "	referred to in rpnexpr as '%%1', '%%2'.... and must all have the\n");
  fprintf(stdout, "	same size.  If the fitsfilename is given as '-' then that file\n");
  fprintf(stdout, "	will be read from stdin.  If fitsfilename is given as 'command\n");
  fprintf(stdout, "	|' then we read from a pipe executing that command.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "	'ic' operates on images a line at a time, and so can be used\n");
  fprintf(stdout, "	on very large images.  A reference to an input image such as\n");
  fprintf(stdout, "	'%%1' causes a line of that image to be pushed onto a stack.\n");
  fprintf(stdout, "	Single argument math functions operate on the line at the top\n");
  fprintf(stdout, "	of the stack and multi-argument functions pop lines as\n");
  fprintf(stdout, "	necessary and then push the resultant line.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "	If any of the input images is flagged as MAGIC (as defined\n");
  fprintf(stdout, "	'fits.h') then the result will be MAGIC also (though with '-m'\n");
  fprintf(stdout, "	option 'ic' will output 'magicsub' in place of the usual\n");
  fprintf(stdout, "	SHRT_MIN)\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "	Fits header comments are inherited from the first image -\n");
  fprintf(stdout, "	other comments are discarded.  You may add extra comments with\n");
  fprintf(stdout, "	the -h option; see below.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "	The functions supported include all of the standard C math\n");
  fprintf(stdout, "	library (including bessel functions j0(x), j1(x), jn(n,x),\n");
  fprintf(stdout, "	y0(x), y1(x), yn(n,x)) plus the operators '+', '-', '*', '/',\n");
  fprintf(stdout, "	and 'mult' is provided as a synonym for '*' to avoid potential\n");
  fprintf(stdout, "	problems if you invoke ic from a script. There are the logical\n");
  fprintf(stdout, "	operations '>', '<', '>=', '<=', '!=', '==', 'and', 'or' and\n");
  fprintf(stdout, "	the negation operator '!'. There is a random number generator\n");
  fprintf(stdout, "	'rand' which generates a uniform random number on the range\n");
  fprintf(stdout, "	0.0-1.0 and 'grand' which generates a zero-mean, unit variance\n");
  fprintf(stdout, "	normal variate. There are two functions 'xp', 'yp' to get the\n");
  fprintf(stdout, "	horixontal and vertical pixel positions respectively, and two\n");
  fprintf(stdout, "	further functions 'x', 'y' which return the position in units\n");
  fprintf(stdout, "	of the lenght of the side of the image.  There is a constant\n");
  fprintf(stdout, "	MAGIC (defined in magic.h - and currently set to -32768) which\n");
  fprintf(stdout, "	is a flag for bad data.  There is a function 'if' (a.k.a. '?')\n");
  fprintf(stdout, "	which mimics the C language '(c ? t : f)' which returns 't' or\n");
  fprintf(stdout, "	'f' respectively depending on the truth or falseness of the\n");
  fprintf(stdout, "	condition 'c'.  The rpn syntax for this expression is 't f c\n");
  fprintf(stdout, "	?' in which '?' pops the condition 'c' followed by 'f' and\n");
  fprintf(stdout, "	then 't' and pushes 't' or 'f' as appropriate. The condition\n");
  fprintf(stdout, "	'c' will of course most likely be a compound logical\n");
  fprintf(stdout, "	expression.  There are functions 'max' and 'min' which pop two\n");
  fprintf(stdout, "	values and pushes the maximum or minimum respectively.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "	There is also a function 'enter' which duplicates the top\n");
  fprintf(stdout, "	value of the stack, and a function 'swap' which unsurprisingly\n");
  fprintf(stdout, "	swaps the top two values on the stack.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "	Use -c option (with no input images) to generate an image from\n");
  fprintf(stdout, "	scratch.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "	Use -p option to specify output pixtpye which can be one of\n");
  fprintf(stdout, "		  8	# 1-byte unsigned char\n");
  fprintf(stdout, "		 16	# 2-byte signed integer\n");
  fprintf(stdout, "		 32	# 4-byte signed int\n");
  fprintf(stdout, "		-32	# 4-byte floating point\n");
  fprintf(stdout, "		-64	# 8-byte floating point   \n");
  fprintf(stdout, "	Otherwise the output will have same pixtype as that of the first\n");
  fprintf(stdout, "	input image, or, with -c option, will have pixtype -32.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "	Use the -b option to apply scaling of pixel values on output\n");
  fprintf(stdout, "	and record the BSCALE, BZERO values in the header.  Otherwise\n");
  fprintf(stdout, "	the BSCALE, BZERO values (if any) are inherited from the\n");
  fprintf(stdout, "	(first) input image.  The definition of BSCALE and BZERO is\n");
  fprintf(stdout, "	such that the internal values are computed from disk values\n");
  fprintf(stdout, "	as:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "		f_internal = BZERO + BSCALE * f_disk\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "	Output goes to stdout.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "	The '-h' option can be used to add new header values or to\n");
  fprintf(stdout, "	modify existing ones.  If an existing name is provided then\n");
  fprintf(stdout, "	the existing value will be overwritten with the new value.\n");
  fprintf(stdout, "	Otherwise, or if the name is 'HISTORY' or 'COMMENT', the new\n");
  fprintf(stdout, "	header line will be appended.  The -h option can be given\n");
  fprintf(stdout, "	repeatedly to insert a number of comments.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "EXAMPLES\n");
  fprintf(stdout, "	Subtract b.fits from a.fits:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "		ic '%%1 %%2 -' a.fits b.fits\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "	Take sqrt of image to be read from stdin; output as float:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "		ic -p -32 '%%1 sqrt' -\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "	Create a 512 x 512 image with a linear horizontal ramp and\n");
  fprintf(stdout, "	multiply by 10:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "		ic -c 512 512 'x 10 *'\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "	Replace all pixels in a.fits with value < 10 with MAGIC:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "		ic '%%1 MAGIC %%1 10 > ?' a.fits\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "	Filter to clip image at fmin = 0.25 fmax = 0.75:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "		ic '%%1 0.25 max 0.75 min' -\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "AUTHOR\n");
  fprintf(stdout, "        Original version:\n");
  fprintf(stdout, "	Nick Kaiser:  kaiser@hawaii.edu\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "        THELI version maintained by\n");
  fprintf(stdout, "        Thomas Erben: terben@astro.uni-bonn.de\n");
}
