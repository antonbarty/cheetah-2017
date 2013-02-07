#include "ami/app/SyncAnalysis.hh"
#include <string.h>

using namespace Ami;

SyncAnalysis::SyncAnalysis(const Pds::DetInfo& detInfo, Pds::TypeId::Type dataType,
              Pds::TypeId::Type configType, void* payload, const char* title):
  detInfo(detInfo),
  _dataType(dataType),
  _configType(configType),
  _configPayload(payload),
  _title(title),
  _liteShotsFull(0),
  _darkShotsFull(0),
  _offByOneStatus(0),
  _arrayBuiltFlag(false),
  _newEvent(false),
  _valMin(0.0),
  _valMax(5000.0),
  _scalingFactor(0.02)
{

} 

SyncAnalysis::~SyncAnalysis()
{
  if (_litePayload)
    delete [] _litePayload; 
  if (_darkPayload)
    delete [] _darkPayload; 
};

void SyncAnalysis::syncReset() 
{
  _liteShotIndex = 0;
  _darkShotIndex = 0;
  _liteShotValue = 0;
  _darkShotValue = 0; 
  _darkShotsFull = 0;
  _liteShotsFull = 0;
  memset(_litePayload, 0, _litePayloadsize);   
  memset(_darkPayload, 0, _darkPayloadsize); 

}


void SyncAnalysis::logConfigPayload    (void* payload)
{
   _configPayload = payload;	
}

void SyncAnalysis::logEventDataPayload (void* payload)
{
   _dataPayload = payload;	
   _newEvent = true;
   this->logDetPayload(payload);   
}


void SyncAnalysis::buildArray(unsigned liteDataLength,unsigned darkDataLength)
{
  _litePayloadsize = sizeof(unsigned long long) + (sizeof(double)*liteDataLength);
  _litePayload = new unsigned long long[(_litePayloadsize>>3)+1];
  memset(_litePayload, 0, _litePayloadsize);  
  _liteShotArray = static_cast<double*> ((void*)(_litePayload+1));

  _darkPayloadsize = sizeof(unsigned long long) + (sizeof(double)*darkDataLength);
  _darkPayload = new unsigned long long[(_darkPayloadsize>>3)+1];
  memset(_darkPayload, 0, _darkPayloadsize);  
  _darkShotArray = static_cast<double*> ((void*)(_darkPayload+1));

  _liteArrayLength = liteDataLength;
  _darkArrayLength = darkDataLength;
  _arrayBuiltFlag  = true;
  _liteShotIndex   = 0;
  _darkShotIndex   = 0;
  _liteShotValue   = 0;
  _darkShotValue   = 0;
  _darkShotsFull   = 0;
  _liteShotsFull   = 0;  

}


void SyncAnalysis::logDataPoint(double val, bool darkShot)
{
  if (darkShot) {
    _darkShotValue = val;
    *(_darkShotArray+ _darkShotIndex) = val;
    if(_darkShotIndex == (_darkArrayLength-1)) {
      _darkShotIndex = 0;
      _darkShotsFull = 1;
    } else _darkShotIndex++;        
  } else {
    _liteShotValue = val;
    *(_liteShotArray+ _liteShotIndex) = val;
    if(_liteShotIndex == (_liteArrayLength-1)) {
      _liteShotIndex = 0;
      _liteShotsFull = 1;
    } else _liteShotIndex++;
  }

  if (darkShot) 
    printf("%s - val = %f Gval = %f Dval = %f  diff = %f%% darkIndex = %u\n",_title,val,_liteShotValue,_darkShotValue,((_liteShotValue-_darkShotValue)/(_valMax-_valMin))*100.0,_darkShotIndex); 

}



namespace Ami {
  
  // for taking care of extra data in Config (e.g - for EVR & Opal) -- Use "detConfigPtr" for complete config
  template<>  opalDataSpace::DataSpace(const Pds::DetInfo& detInfo, Pds::TypeId::Type dataType,Pds::TypeId::Type configType, void* payload, const char* title):
    SyncAnalysis(detInfo, dataType, configType, payload, title) { 
    detConfig = *reinterpret_cast<Opal1kConfigType*>(payload);
    unsigned configSize = detConfig.size();
    detConfigPtr = (Opal1kConfigType*)(new unsigned [(configSize/sizeof(unsigned))+1]);
    memcpy(detConfigPtr, payload, configSize);
  }


  template <class CONFIG, class DATA> double DataSpace<CONFIG, DATA>::processData()
  { 
    printf("ERROR:: DataSpace::Empty processData()\n");
    return 0;
  }

  // explicit template specialization for data processing functions 
  template<> double acqDataSpace::processData() 
  { 
    Pds::Acqiris::ConfigV1* acqConfig = &detConfig;
    Pds::Acqiris::DataDescV1* acqData = detDataPtr;
    Pds::Acqiris::HorizV1& h = acqConfig->horiz();
    unsigned n = acqConfig->nbrChannels();
    double val = 0;  
    for (unsigned i=0;i<n;i++) {	
      const int16_t* data = acqData->waveform(h);
      data += acqData->indexFirstPoint();
      float slope = acqConfig->vert(i).slope();
      float offset = acqConfig->vert(i).offset();
      unsigned nbrSamples = h.nbrSamples();
      for (unsigned j=0;j<nbrSamples;j++) {
        int16_t swap = (data[j]&0xff<<8) | (data[j]&0xff00>>8);
        val = val + fabs(swap*slope-offset);      //fabs value for area under curve 
      }
      acqData = acqData->nextChannel(h);	
    }
    return val;
  };


  template<> double opalDataSpace::processData()  
  {
    Pds::Camera::FrameV1* opalFrame = detDataPtr;
    const uint16_t* dataArray = reinterpret_cast<const uint16_t*>(opalFrame->data()); 
    unsigned totalPixels = opalFrame->width()*opalFrame->height(); 
    unsigned offset = opalFrame->offset();
    unsigned val = 0; 
    for (unsigned i = 0 ; i<totalPixels ; i++)  { 
      if (*(dataArray+i) > offset)
        val = val + (*(dataArray+i) );             // opal data with Pedestal/Offset adjustment 
    }
    return ((double) val);
  }


  template<> double fccdDataSpace::processData() 
  {
    Pds::Camera::FrameV1* fccdFrame = detDataPtr;
    const uint16_t* dataArray = reinterpret_cast<const uint16_t*>(fccdFrame->data());  
    unsigned totalPixels = fccdFrame->width()*fccdFrame->height();
    unsigned offset = fccdFrame->offset();
    unsigned val = 0; 
    for (unsigned i = 0 ; i<totalPixels ; i++)  { 
      if (*(dataArray+i) > offset)
      val = val + (*(dataArray+i) );            // FCCD data with Pedestal/Offset adjustment 
    }
    return ((double) val);
  }


  template<> double pulnixDataSpace::processData() 
  { 
    Pds::Camera::FrameV1* pulnixFrame = detDataPtr;
    const uint16_t* dataArray = reinterpret_cast<const uint16_t*>(pulnixFrame->data());  
    unsigned totalPixels = pulnixFrame->width()*pulnixFrame->height();
    unsigned offset = pulnixFrame->offset();
    unsigned val = 0; 
    for (unsigned i = 0 ; i<totalPixels ; i++)  { 
      if (*(dataArray+i) > offset)
        val = val + (*(dataArray+i) );             // Pulnix data with Pedestal/Offset adjustment 
    }
    return ((double) val);
  }

  template<> double pnccdDataSpace::processData() 
  { 
    pnCCDConfigType& pnccdConfig = detConfig;
    const Pds::PNCCD::FrameV1*  pnccdFrame   = detDataPtr;
    const uint16_t* dataArray = reinterpret_cast<const uint16_t*>(pnccdFrame->data());
    unsigned val = 0;
    for (unsigned j=0; j<4 ; j++ ) {                // for all 4 quadrants in sequence UpL+LoL+LoR+UpR
      unsigned dataSize = pnccdFrame->sizeofData(pnccdConfig);
      for(unsigned i= 0; i< dataSize ; i++) {
        val = val + ( ((*(dataArray+i) ) & 0x3fff) >>1 ); 
      }
      pnccdFrame = pnccdFrame->next(pnccdConfig);
      dataArray = reinterpret_cast<const uint16_t*>(pnccdFrame->data());
    }
    return ((double)val);
  }

  template<> double princetonDataSpace::processData() 
  { 
    PrincetonConfigType& princetonConfig = detConfig; 
    Pds::Princeton::FrameV1*  princetonFrame   = detDataPtr; 
    const uint16_t* dataArray = reinterpret_cast<const uint16_t*>(princetonFrame->data());
    unsigned totalPixels = princetonConfig.width()*princetonConfig.height();
    unsigned val = 0;   
    for (unsigned i = 0 ; i<totalPixels ; i++) 
      val = val + (*(dataArray+i) ); 
    return ((double)val);
  }

  template<> double ipimbDataSpace::processData() 
  { 
    Pds::Ipimb::DataV2* ipimbData = detDataPtr; 
    double val =  fabs(ipimbData->channel0Volts()) + fabs(ipimbData->channel1Volts()) + fabs(ipimbData->channel2Volts()) + fabs(ipimbData->channel3Volts()) ; 
    return val;
  }

  template<> double eBeamDataSpace::processData()   
  {
    Pds::BldDataEBeam* EBeamData = detDataPtr; 
    double val = fabs(EBeamData->fEbeamCharge);
    return val; 
  }

  template<> double gasDetectorDataSpace::processData() 
  {
    Pds::BldDataFEEGasDetEnergy* gasDetectorData = detDataPtr;
    //gas detector abs() adddition of 4 energy sensors 
    double val = fabs(gasDetectorData->f_11_ENRC) + fabs(gasDetectorData->f_12_ENRC) + fabs(gasDetectorData->f_21_ENRC) + fabs(gasDetectorData->f_22_ENRC); 
    return val;
  }


  template<> double phaseCavityDataSpace::processData() 
  {
    Pds::BldDataPhaseCavity* phaseCavityData = detDataPtr; 
    double val = fabs(phaseCavityData->fCharge1) + fabs(phaseCavityData->fCharge2); 
    return val;
  }

};
