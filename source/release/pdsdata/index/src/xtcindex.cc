#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "pdsdata/xtc/XtcFileIterator.hh"
#include "pdsdata/index/XtcIterL1Accept.hh"
#include "pdsdata/index/IndexList.hh"

using namespace Pds;
using Index::XtcIterL1Accept;
using Index::IndexList;

void usage(char *progname)
{
  printf( 
    "Usage:  %s  [-f <xtc filename>] [-i <input index>] [-o <index filename>] [-h]\n"
    "  Options:\n"
    "    -h                     Show usage.\n"
    "    -f <xtc filename>      Set input xtc filename\n"
    "    -i <index filename>    Set input index filename\n"
    "       Note 1: -i option is used for testing. The program will parse\n"
    "         the index file to see if it is valid, and output to another\n"
    "         index file if -o option is specified\n"
    "       Note 2: -i will overwrite -f option\n"
    "    -o <index filename>    Set output index filename\n",
    progname
  );
}

int generateIndex(char* sXtcFilename, char* sOutputIndex);
int readIndex    (char* sInputIndex , char* sOutputIndex);

int main(int argc, char *argv[])
{
  int c;
  char *sXtcFilename    = NULL;
  char *sOutputIndex    = NULL;
  char *sInputIndex     = NULL;

  while ((c = getopt(argc, argv, "hf:o:i:")) != -1)
  {
    switch (c)
    {
    case 'h':
      usage(argv[0]);
      exit(0);
    case 'f':
      sXtcFilename    = optarg;
      break;
    case 'o':
      sOutputIndex    = optarg;
      break;
    case 'i':
      sInputIndex     = optarg;
      break;
    default:
      printf( "Unknown option: -%c", c );
    }
  }

  if (! (sXtcFilename || sInputIndex) )
  {
    usage(argv[0]);
    exit(2);
  }

  if ( sInputIndex )
    return readIndex( sInputIndex, sOutputIndex );
  else   
    return generateIndex( sXtcFilename, sOutputIndex );
}

int generateIndex(char* sXtcFilename, char* sOutputIndex)
{
  int fd = open(sXtcFilename, O_RDONLY | O_LARGEFILE);
  if (fd < 0)
  {
    printf("Unable to open xtc file %s\n", sXtcFilename);
    return 1;
  }
  
  IndexList         indexList (sXtcFilename);
  XtcFileIterator   iterFile  (fd, 0x2000000); // largest L1 data: 32 MB
    
  Dgram *dg;
  int64_t i64Offset = lseek64(fd, 0, SEEK_CUR);
  while ((dg = iterFile.next()))
  {             
    if (dg->seq.service() == TransitionId::L1Accept)
    {
      bool bInvalidNodeData = false;
      indexList.startNewNode(*dg, i64Offset, bInvalidNodeData);
      
      if ( bInvalidNodeData )
        continue;     
      
      XtcIterL1Accept iterL1Accept(&(dg->xtc), 0, i64Offset + sizeof(*dg), indexList);           
      iterL1Accept.iterate();
            
      bool bPrintNode = true;
      indexList.finishNode(bPrintNode);
    }
    else if (dg->seq.service() == TransitionId::BeginCalibCycle)
    {
      indexList.addCalibCycle(i64Offset, dg->seq.clock().seconds(), dg->seq.clock().nanoseconds());
    }
      
    i64Offset = lseek64(fd, 0, SEEK_CUR); // get the file offset for the next iteration
  }

  indexList.finishList();  
  
  int iVerbose = 2;
  indexList.printList(iVerbose);  
    
  if ( sOutputIndex != NULL )
  {
    //int fdIndex = open(sOutputIndex, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
    int fdIndex = open(sOutputIndex, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ); //!! debug
    if ( fdIndex == -1 )
      printf( "xtcIndex(): Open index file %s failed (%s)\n", sOutputIndex, strerror(errno) );
    else
    {  
      indexList.writeToFile(fdIndex);    
      ::close(fdIndex);
    }  
  }
  
  ::close(fd);
  return 0;
}

int readIndex(char* sInputIndex, char* sOutputIndex)
{
  int fd = open(sInputIndex, O_RDONLY | O_LARGEFILE);
  if (fd < 0)
  {
    printf("Unable to open xtc file %s\n", sInputIndex);
    return 1;
  }
  
  IndexList indexList;
  indexList.readFromFile(fd);
  
  ::close(fd);
  
  int iVerbose = 2;
  indexList.printList(iVerbose);  
    
  if ( sOutputIndex != NULL )
  {
    //int fdIndex = open(sOutputIndex, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
    int fdIndex = open(sOutputIndex, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ); //!! debug
    if ( fdIndex == -1 )
      printf( "xtcIndex(): Open index file %s failed (%s)\n", sOutputIndex, strerror(errno) );
    else
    {  
      indexList.writeToFile(fdIndex);    
      ::close(fdIndex);
    }  
  }
  
  
  return 0;
}
