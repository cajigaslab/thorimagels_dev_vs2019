#include "stdafx.h"
#include "AcquireSim.h"
#include <cmath>
//#include "AcquireData.h"
//#include "RealTimeDataXML.h"

//#include "ipps.h"
//#include "ippcore.h"
//#include "IPPlib.h"


//#include "strsafe.h"
//using namespace std;

std::auto_ptr<IPPSDll> ippsDll2(new IPPSDll(L".\\.\\ipps-7.0.dll"));

int main(void)
{
Ipp64f x[8];
Ipp64f X[10];

int n;



IppStatus status;
IppsFFTSpec_R_64f* spec;

status=ippsDll2->ippsFFTInitAlloc_R_64f(&spec,3,IPP_FFT_DIV_INV_BY_N,ippAlgHintNone);

for (n=0;n<8;n++)
    {
        x[n]=(float)cos(2*3.141592*n/4);
    }

status=ippsDll2->ippsFFTFwd_RToCCS_64f(x,X,spec,NULL);
status=ippsDll2->ippsMagnitude_64fc((Ipp64fc*)X,x,4);

status=ippsDll2->ippsFFTFree_R_64f(spec);
//printf("fft magn=");
for (int i=0;i<8;i++)
std::cout<<x[i]<<std::endl;


return 0;
}