#ifndef Cspad_Detector_hh
#define Cspad_Detector_hh

namespace Pds {
  namespace CsPad {
    enum {MaxQuadsPerSensor=4, ASICsPerQuad=16};
    enum {RowsPerBank=26, FullBanksPerASIC=7, BanksPerASIC=8, ColumnsPerASIC=185, MaxRowsPerASIC=194};
    enum {PotsPerQuad=80, TwoByTwosPerQuad=4};
    enum RunModes  {NoRunning, RunButDrop, RunAndSendToRCE, RunAndSendTriggeredByTTL, ExternalTriggerSendToRCE, ExternalTriggerDrop, NumberOfRunModes };
    enum DataModes {normal=0, shiftTest=1, testData=2, reserved=3};
  };
};

#endif
