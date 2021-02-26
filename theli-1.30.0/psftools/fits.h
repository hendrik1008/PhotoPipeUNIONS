#define	MAX_COMMENTS	1024
#define COM_LENGTH	80
#define MAX_FITS_DIM	7

#define	SHORT_PIXTYPE	16
#define FLOAT_PIXTYPE	-32
#define	INT_PIXTYPE	32


void	write_fits_head(int N1, int N2, int comc, char *comv[]);
void	write_fits_line(short *f, int N1);
void	write_fits_tail(int N1, int N2);
void	write_fits(short **f, int N1, int N2, int comc, char *comv[]);
void	read_fits_head(int *N1, int *N2, int *comc, char *comv[]);
void	read_fits_line(short *f, int N1);
void	read_fits(short ***f, int *N1, int *N2, int *comc, char *comv[]);
void	write_ascii_head(int N1, int N2, int comc, char *comv[]);
void	write_ascii_line(short *f, int N1);
void	write_ascii(short **f, int N1, int N2, int comc, char *comv[]);
void	read_ascii_head(int *N1, int *N2, int *comc, char *comv[]);
void	read_ascii_line(short *f, int N1);
void	read_ascii(short ***f, int *N1, int *N2, int *comc, char *comv[]);
void	argsToString(int argc, char **argv, char *string);
void	add_comment(int argc, char **argv, int *comc, char **comv);
void	add_1comment(char *comment, int *comc, char **comv);
void	set_fits_opf(FILE *opf);
void	set_fits_ipf(FILE *ipf);

/* floating point version */
void	fwrite_fits(float **f, int N1, int N2, int comc, char *comv[]);
void	fread_fits(float ***f, int *N1, int *N2, int *comc, char *comv[]);
void	fwrite_fits_line(float *f, int N1);
void	fread_fits_line(float *f, int N1);

void	set_output_pixtype(int thetype);
void	get_input_pixtype(int *thetype);

/* N-Dimensional image routines */
void	read_fits_head_ND(int *Naxes, int *N, int *comc, char *comv[]);
void	write_fits_head_ND(int Naxes, int *N, int comc, char *comv[]);
void	write_fits_tail_ND(int Naxes, int *N);


