#include "ami/data/Complex.hh"

using namespace Ami;

void Complex::transform(Complex* f,unsigned array_size,int isign)
{
  register int n=array_size*2;
  register int j=1;
  for (register int i=1;i<n+1;i+=2){
    if (j>i+1) swap(f[(j-1)/2],f[(i-1)/2]);
    register int m=n/2;
    while (m>=2 && j>m){
      j-=m;
      m/=2;
    }
    j+=m;
  }
  
  register int m=0;
  register int mmax=2; 
  float work, wpr, wpi;
  
  while (n>mmax) {
    register int istep=2*mmax;
    double theta=2*3.141592653589793238/(double)(isign*mmax);
    work = mmax==2 ? 1.0 : sin(0.5*theta);
    wpr=-2.0*work*work;
    wpi = mmax==2 ? 0.0 : sin(theta);
    Complex wp(wpr,wpi);
    Complex w(1.0,0.0);
    for (m=1;m<mmax+1;m+=2){
      for (register int i=m;i<n+1;i+=istep){
	register int i_cmplx=(i-1)/2;
	register int j_cmplx=i_cmplx+mmax/2;
       
	Complex temp(w);
	temp*=f[j_cmplx];
	f[j_cmplx]=f[i_cmplx]-temp;
	f[i_cmplx]+=temp;
      }
      w+=w*wp;
    }
    mmax=istep;
  }
  if (isign<0)
    for(unsigned k=0; k<array_size; k++)
      f[k] *= 1./float(array_size);
}

