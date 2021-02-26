#include <math.h>
#include "nrutil.h"

void spline(float x[],float y[],int n,float yp1,float ypn,float y2[])
{
   int i,k;
   float p,qn,sig,un,*u;

   u=KSBvector(1,n-1);
   if (yp1 > 0.99e30)
      y2[1]=u[1]=0.0;
   else {
      y2[1]= -0.5;
      u[1]=(3.0/(x[2]-x[1]))*((y[2]-y[1])/(x[2]-x[1])-yp1);
   }
   for (i=2;i<=n-1;i++) {
      sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);
      p=sig*y2[i-1]+2.0;
      y2[i]=(sig-1.0)/p;
      u[i]=(y[i+1]-y[i])/(x[i+1]-x[i])-(y[i]-y[i-1])/(x[i]-x[i-1]);
      u[i]=(6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
   }
   if (ypn>0.99e30)
      qn=un=0.0;
   else {
      qn=0.5;
      un=(3.0/(x[n]-x[n-1]))*(ypn-(y[n]-y[n-1])/(x[n]-x[n-1]));
   }
   y2[n]=(un-qn*u[n-1])/(qn*y2[n-1]+1.0);
   for (k=n-1;k>=1;k--)
      y2[k]=y2[k]*y2[k+1]+u[k];
   free_KSBvector(u,1,n-1);
}

void splint(float xa[],float ya[],float y2a[],int n,float x,float *y)
{
   int klo,khi,k;
   float h,b,a;

   klo=1;
   khi=n;
   while (khi-klo > 1) {
      k=(khi+klo) >> 1;
      if (xa[k] > x) khi=k;
      else klo=k;
   }
   h=xa[khi]-xa[klo];
   if (h== 0.0) KSBnrerror("Bad xa input to routine splint");
   a=(xa[khi]-x)/h;
   b=(x-xa[klo])/h;
   *y=a*ya[klo]+b*ya[khi]+((a*a*a-a)*y2a[klo]+(b*b*b-b)*y2a[khi])*(h*h)/6.0;
}

void splie2(float x1a[], float x2a[], float **ya, int m, int n, float **y2a)
{
  void spline(float x[],float y[],int n,float yp1,float ypn,float y2[]);
  int j;

  for (j=1;j<=m;j++)
    spline(x2a,ya[j],n,1.0e30,1.0e30,y2a[j]);
}

void splin2(float x1a[], float x2a[], float **ya, float **y2a, int m, int n, float x1, float x2, float *y)
{
  void spline(float x[],float y[],int n,float yp1,float ypn,float y2[]);
  void splint(float xa[],float ya[],float y2a[],int n,float x,float *y);
  int j;
  float *ytmp,*yytmp;

  ytmp=KSBvector(1,m);
  yytmp=KSBvector(1,m);
  for (j=1;j<=m;j++)
    splint(x2a,ya[j],y2a[j],n,x2,&yytmp[j]);
  spline(x1a,yytmp,m,1.0e30,1.0e30,ytmp);
  splint(x1a,yytmp,ytmp,m,x1,y);
  free_KSBvector(yytmp,1,m);
  free_KSBvector(ytmp,1,m);
}

void polint(float xa[],float ya[],int n,float x,float *y,float *dy)
{
   int i,m,ns=1;
   float den,dif,dift,ho,hp,w;
   float *c,*d;

   dif=fabs(x-xa[1]);
   c=KSBvector(1,n);
   d=KSBvector(1,n);
   for (i=1;i<=n;i++) {
      if ( (dift=fabs(x-xa[i])) < dif) {
	 ns=i;
	 dif=dift;
      }
      c[i]=ya[i];
      d[i]=ya[i];
   }
   *y=ya[ns--];
   for (m=1;m<n;m++) {
      for (i=1;i<=n-m;i++) {
	 ho=xa[i]-x;
	 hp=xa[i+m]-x;
	 w=c[i+1]-d[i];
	 if ( (den=ho-hp) == 0.0) KSBnrerror("Error in routine polint");
	 den=w/den;
	 d[i]=hp*den;
	 c[i]=ho*den;
      }
      *y += (*dy=(2*ns < (n-m) ? c[ns+1] : d[ns--]));
   }
   free_KSBvector(d,1,n);
   free_KSBvector(c,1,n);
}

float interp_d2(int nx, int ny, float **data, float x, float y)
{
  int ix0, ix1, ix2, ix3, iy0, iy1, iy2, iy3;
  float dx, dy;
  float fy0, fy1, fy2, fy3;
  float cx0, cx1, cx2, cx3, cy0, cy1, cy2, cy3;

  if (x > nx || x < 0.0 || y > ny || y < 0.0) { return 0.0;}

  ix1=(int)(x);
  iy1=(int)(y);
  ix2=ix1+1;
  iy2=iy1+1;
  ix3=ix1+2;
  iy3=iy1+2;
  ix0=ix1-1;
  iy0=iy1-1;

  dx=x-(float)(ix1);
  dy=y-(float)(iy1);

  if (ix0<0)  ix0=ix0+nx;
  if (ix1<0)  ix1=ix1+nx;
  if (ix2>nx) ix2=ix2-nx;
  if (ix3>nx) ix3=ix3-nx;
  if (iy0<0)  iy0=iy0+ny;
  if (iy1<0)  iy1=iy1+ny;
  if (iy2>ny) iy2=iy2-ny;
  if (iy3>ny) iy3=iy3-ny;

  cx0=-dx*(dx-1.0)*(dx-2.0)*.1666666;
  cx1=(dx*dx-1.0)*(dx-2.0)*.5;
  cx2=-dx*(dx+1.0)*(dx-2.0)*.5;
  cx3=dx*(dx*dx-1.0)*0.1666666;
  cy0=-dy*(dy-1.0)*(dy-2.0)*.1666666;
  cy1=(dy*dy-1.0)*(dy-2.0)*.5;
  cy2=-dy*(dy+1.0)*(dy-2.0)*.5;
  cy3=dy*(dy*dy-1.0)*0.1666666;
  fy0=cx0*data[ix0][iy0] + cx1*data[ix1][iy0] +
      cx2*data[ix2][iy0] + cx3*data[ix3][iy0];
  fy1=cx0*data[ix0][iy1] + cx1*data[ix1][iy1] +
      cx2*data[ix2][iy1] + cx3*data[ix3][iy1];
  fy2=cx0*data[ix0][iy2] + cx1*data[ix1][iy2] +
      cx2*data[ix2][iy2] + cx3*data[ix3][iy2];
  fy3=cx0*data[ix0][iy3] + cx1*data[ix1][iy3] +
      cx2*data[ix2][iy3] + cx3*data[ix3][iy3];

  return cy0*fy0+cy1*fy1+cy2*fy2+cy3*fy3;
}


