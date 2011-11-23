#ifndef Pds_XtcMonitorMsg_hh
#define Pds_XtcMonitorMsg_hh

namespace Pds {
  class XtcMonitorMsg {
  public:
    XtcMonitorMsg() {}
    XtcMonitorMsg(int bufferIndex) {_bufferIndex = bufferIndex;}
    ~XtcMonitorMsg() {}; 
  public:
    int bufferIndex     () const {return _bufferIndex;}
    int numberOfBuffers () const { return _numberOfBuffers; }
    int sizeOfBuffers   () const { return _sizeOfBuffers; }
  public:
    XtcMonitorMsg* bufferIndex(int b) {_bufferIndex=b; return this;}
    void numberOfBuffers      (int n) {_numberOfBuffers = n;} 
    void sizeOfBuffers        (int s) {_sizeOfBuffers = s;} 
  public:
    static void sharedMemoryName     (const char* tag, char* buffer);
    static void eventInputQueue      (const char* tag, unsigned client, char* buffer);
    static void eventOutputQueue     (const char* tag, unsigned client, char* buffer);
    static void transitionInputQueue (const char* tag, unsigned client, char* buffer);
    static void discoveryQueue       (const char* tag, char* buffer);
  private:
    int _bufferIndex;
    int _numberOfBuffers;
    unsigned _sizeOfBuffers;
  };
};

#endif
