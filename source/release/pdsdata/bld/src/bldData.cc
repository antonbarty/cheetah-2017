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

int BldDataEBeamV1::print() const
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

int BldDataEBeamV2::print() const
{    
    printf( "ebeamCharge   ( in nC )  : %lf\n", fEbeamCharge  ); 
    printf( "ebeamL3Energy ( in MeV ) : %lf\n", fEbeamL3Energy); 
    printf( "ebeamLTUPosX  ( in mm )  : %lf\n", fEbeamLTUPosX ); 
    printf( "ebeamLTUPosY  ( in mm )  : %lf\n", fEbeamLTUPosY ); 
    printf( "ebeamLTUAngX  ( in mrad ): %lf\n", fEbeamLTUAngX ); 
    printf( "ebeamLTUAngY  ( in mrad ): %lf\n", fEbeamLTUAngY );
    printf( "ebeamPkCurrBC2( in mA )  : %lf\n", fEbeamPkCurrBC2 );
    printf( "ebeamEnergyBC2( in MeV ) : %lf\n", fEbeamEnergyBC2 );
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

int BldDataIpimbV0::print() const 
{   
    printf("BLD Shared IPIMB Data:\n");
    printf("  Trig Count      : %llu \n",ipimbData.triggerCounter());
    printf("  IpimbDataCh-0   : %f   \n",ipimbData.channel0Volts());
    printf("  IpimbDataCh-1   : %f   \n",ipimbData.channel1Volts());    	
    printf("  IpimbDataCh-2   : %f   \n",ipimbData.channel2Volts());	
    printf("  IpimbDataCh-3   : %f   \n",ipimbData.channel3Volts());

    printf("  IpmFexDataCh-0  : %f   \n",ipmFexData.channel[0]);	
    printf("  IpmFexDataCh-1  : %f   \n",ipmFexData.channel[1]);
    printf("  IpmFexDataCh-2  : %f   \n",ipmFexData.channel[2]);
    printf("  IpmFexDataCh-3  : %f   \n",ipmFexData.channel[3]);
    printf("  IpmFexDataSum   : %f   \n",ipmFexData.sum);
    printf("  IpmFexDataXpos  : %f   \n",ipmFexData.xpos);
    printf("  IpmFexDataYpos  : %f   \n",ipmFexData.ypos);
    
    return 0;
}

int BldDataIpimbV1::print() const 
{   
    printf("BLD Shared IPIMB Data:\n");
    printf("  Trig Count      : %llu \n",ipimbData.triggerCounter());
    printf("  IpimbDataCh-0   : %f   \n",ipimbData.channel0Volts());
    printf("  IpimbDataCh-1   : %f   \n",ipimbData.channel1Volts());    	
    printf("  IpimbDataCh-2   : %f   \n",ipimbData.channel2Volts());	
    printf("  IpimbDataCh-3   : %f   \n",ipimbData.channel3Volts());
    printf("  IpimbDataCh-0ps : %f   \n",ipimbData.channel0psVolts());
    printf("  IpimbDataCh-1ps : %f   \n",ipimbData.channel1psVolts());    	
    printf("  IpimbDataCh-2ps : %f   \n",ipimbData.channel2psVolts());	
    printf("  IpimbDataCh-3ps : %f   \n",ipimbData.channel3psVolts());

    printf("  IpmFexDataCh-0  : %f   \n",ipmFexData.channel[0]);	
    printf("  IpmFexDataCh-1  : %f   \n",ipmFexData.channel[1]);
    printf("  IpmFexDataCh-2  : %f   \n",ipmFexData.channel[2]);
    printf("  IpmFexDataCh-3  : %f   \n",ipmFexData.channel[3]);
    printf("  IpmFexDataSum   : %f   \n",ipmFexData.sum);
    printf("  IpmFexDataXpos  : %f   \n",ipmFexData.xpos);
    printf("  IpmFexDataYpos  : %f   \n",ipmFexData.ypos);
    
    return 0;
}




