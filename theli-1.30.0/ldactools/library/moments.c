#include <math.h>
#include "options_globals.h"
#include "utils_globals.h"

void moments(float *data, int n, float *ave, float *adev, 
             float *sdev, float *svar, float *skew, float *curt)
{
    int j;
    float s,p;

    if (n <= 0) 
       syserr(FATAL,"moments", "n must be at least 1\n");
    s=0.0;
    for (j=0;j<n;j++) s += data[j];
    *ave=s/n;
    *adev=(*svar)=(*skew)=(*curt)=0.0;
    for (j=0;j<n;j++) {
       *adev += fabs(s=data[j]-(*ave));
       *svar += (p=s*s);
       *skew += (p *= s);
       *curt += (p *= s);
    }
    *adev /= n;
    if(n>1)
    {
      *svar /= (n-1);
      *sdev=sqrt(*svar);
    }
    if (*svar) {
       *skew /= (n*(*svar)*(*sdev));
       *curt=(*curt)/(n*(*svar)*(*svar))-3.0;
    } 
    else
    { 
      syserr(WARNING,"moments", "No skew/kurtosis when variance = 0\n");
      *skew=0.0;
      *curt=0.0;
    }
}

