#ifndef XTC_EPICS_FILE_READER_H
#define XTC_EPICS_FILE_READER_H

#include <string>

namespace Pds
{
    
class XtcEpicsFileReader
{
public:
    XtcEpicsFileReader( char* lcFnXtc );
    ~XtcEpicsFileReader();
    int doFileRead();
    
private:        
    std::string _sFnXtc;    
    
    // Class usage control: Value semantics is disabled
    XtcEpicsFileReader( XtcEpicsFileReader& );
    XtcEpicsFileReader& operator=(const XtcEpicsFileReader& );    
};    

} // namespace Pds 

#endif
