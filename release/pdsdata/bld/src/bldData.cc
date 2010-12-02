#include <stdio.h>
#include "pdsdata/bld/bldData.hh"

using namespace Pds;

int BldDataFEEGasDetEnergy::print() const
{    
    printf("GDET:FEE:11:ENRC ( in mJ ): %lf\n", f_11_ENRC );
    printf("GDET:FEE:12:ENRC ( in mJ ): %lf\n", f_12_ENRC );
    printf("GDET:FEE:21:ENRC ( in mJ ): %lf\n", f_21_ENRC );
    printf("GDET:FEE:22:ENRC ( in mJ ): %lf\n", f_22_ENRC );
    
    return 0;
}

int BldDataEBeamV0::print() const
{    
    printf( "ebeamCharge   ( in nC )  : %lf\n", fEbeamCharge  ); 
    printf( "ebeamL3Energy ( in MeV ) : %lf\n", fEbeamL3Energy); 
    printf( "ebeamLTUPosX  ( in mm )  : %lf\n", fEbeamLTUPosX ); 
    printf( "ebeamLTUPosY  ( in mm )  : %lf\n", fEbeamLTUPosY ); 
    printf( "ebeamLTUAngX  ( in mrad ): %lf\n", fEbeamLTUAngX ); 
    printf( "ebeamLTUAngY  ( in mrad ): %lf\n", fEbeamLTUAngY );
    return 0;
}

int BldDataEBeam::print() const
{    
    printf( "ebeamCharge   ( in nC )  : %lf\n", fEbeamCharge  ); 
    printf( "ebeamL3Energy ( in MeV ) : %lf\n", fEbeamL3Energy); 
    printf( "ebeamLTUPosX  ( in mm )  : %lf\n", fEbeamLTUPosX ); 
    printf( "ebeamLTUPosY  ( in mm )  : %lf\n", fEbeamLTUPosY ); 
    printf( "ebeamLTUAngX  ( in mrad ): %lf\n", fEbeamLTUAngX ); 
    printf( "ebeamLTUAngY  ( in mrad ): %lf\n", fEbeamLTUAngY );
    printf( "ebeamPkCurrBC2( in mA )  : %lf\n", fEbeamPkCurrBC2 );
    return 0;
}

int BldDataPhaseCavity::print() const
{    
    printf("FitTime1 ( in pico-seconds ): %lf\n", fFitTime1 );
    printf("FitTime2 ( in pico-seconds ): %lf\n", fFitTime2 );
    printf("Charge1  ( in pico-columbs ): %lf\n", fCharge1 );
    printf("Charge2  ( in pico-columbs ): %lf\n", fCharge2 );
    
    return 0;
}
