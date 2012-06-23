#include <stdio.h>
#include <getopt.h>
#include "XtcEpicsFileReader.hh"

namespace Pds
{
static const char sXtcEpicsTestVersion[] = "2.00";

// forward declarations
int xtcEpicsReadTest( char* sFnXtc );

}

using namespace Pds;

void showUsageXtcEpicsTest()
{
    printf( "Usage:  xtcEpicsTest  [-v|--version] [-h|--help] <xtc filename>  <pv1 name> [ <pv2 name> ... ]\n" 
      "  Options:\n"
      "    -v      Show file version\n"
      "    -h      Show Usage\n"
      );
}

void showVersionXtcEpicsTest()
{
    printf( "Version:  xtcEpicsTest  Ver %s\n", sXtcEpicsTestVersion );
}

int main(int argc,char **argv)
{   
    int iOptionIndex = 0;
    struct option loOptions[] = 
    {
       {"ver",  0, 0, 'v'},
       {"help", 0, 0, 'h'},
       {0,      0, 0, 0  }
    };
    
    while ( int opt = getopt_long(argc, argv, ":vh", loOptions, &iOptionIndex ) )
    {
        if ( opt == -1 ) break;
        
        switch (opt) 
        {
        case 'v':               /* Print usage */
            showVersionXtcEpicsTest();
            return 0;
        case '?':               /* Terse output mode */
            printf( "xtcEpicsTest:main(): Unknown option: %c\n", optopt );
            break;
        case ':':               /* Terse output mode */
            printf( "xtcEpicsTest:main(): Missing argument for %c\n", optopt );
            break;
        default:
        case 'h':               /* Print usage */
            showUsageXtcEpicsTest();
            return 0;
        }
    }
    
    argc -= optind;
    argv += optind;
    
    if ( argc < 1 ) 
    {
        showUsageXtcEpicsTest();
        return 1;        
    }
        
    xtcEpicsReadTest( argv[0] );
                    
    return 0;
}


using std::string;

namespace Pds
{
    
int xtcEpicsReadTest( char* sFnXtc )
{
    if ( sFnXtc == NULL )
    {
        printf( "xtcEpicsReadTest(): Invalid argument(s)\n" );
        return 1;
    }
    
    try
    {    
        XtcEpicsFileReader xtcEpicsFile(sFnXtc);
        xtcEpicsFile.doFileRead();
    }
    catch (string& sError)
    {
        printf( "xtcEpicsReadTest():XtcEpicsFileReader failed: %s\n", sError.c_str() );
    }
    
    return 0;
}

} // namespace Pds
