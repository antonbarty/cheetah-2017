#ifndef Cspad2x2_Detector_hh
#define Cspad2x2_Detector_hh

namespace Pds {
  namespace CsPad2x2 {
    enum {QuadsPerSensor=1, ASICsPerQuad=4};
    enum {RowsPerBank=26, FullBanksPerASIC=7, BanksPerASIC=8, ColumnsPerASIC=185, MaxRowsPerASIC=194};
    enum {PotsPerQuad=80, TwoByTwosPerQuad=1};
    enum RunModes  {NoRunning, RunButDrop, RunAndSendToRCE, RunAndSendTriggeredByTTL, ExternalTriggerSendToRCE, ExternalTriggerDrop, NumberOfRunModes };
    enum DataModes {normal=0, shiftTest=1, testData=2, reserved=3};
  };
};

#endif
