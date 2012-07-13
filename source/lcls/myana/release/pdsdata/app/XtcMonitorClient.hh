#ifndef XTC_MONITOR_CLIENT_HH
#define XTC_MONITOR_CLIENT_HH


namespace Pds {

  class Dgram;

  class XtcMonitorClient {
  public:
    XtcMonitorClient() {}
    virtual ~XtcMonitorClient() {};
    
  public:
    //
    //  tr_index must be unique among clients
    //  unique values of ev_index produce a serial chain of clients sharing events
    //  common values of ev_index produce a set of clients competing for events
    //
    int run(const char * partitionTag, int tr_index=0);
    int run(const char * partitionTag, int tr_index, int ev_index);
    virtual int processDgram(Dgram*);
  };
}
#endif
