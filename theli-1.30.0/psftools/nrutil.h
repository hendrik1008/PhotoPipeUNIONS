#ifndef _NR_UTILS_H_
#define _NR_UTILS_H_

#define SIGN(a,b) ((b) > 0.0 ? fabs(a) : -fabs(a))

void KSBnrerror(char error_text[]);
float *KSBvector(long nl, long nh);
float **KSBmatrix(long nrl, long nrh, long ncl, long nch);
float ***f3tensor(long nrl, long nrh, long ncl, long nch, long ndl, long ndh);
void free_KSBvector(float *v, long nl, long nh);
void free_KSBmatrix(float **m, long nrl, long nrh, long ncl, long nch);
void free_f3tensor(float ***t, long nrl, long nrh, long ncl, long nch,
	long ndl, long ndh);


#endif /* _NR_UTILS_H_ */


