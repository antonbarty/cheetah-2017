
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/ProcInfo.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/XtcFileIterator.hh"
#include "pdsdata/camera/FrameV1.hh"
#include "pdsdata/fccd/FccdConfigV2.hh"
#include "pdsdata/princeton/FrameV1.hh"
#include "pdsdata/princeton/ConfigV1.hh"

using namespace Pds;

class myLevelIter : public XtcIterator {
public:
  enum {Stop, Continue};
  myLevelIter(Xtc* xtc, unsigned depth, int fd) : XtcIterator(xtc), _depth(depth), _fd(fd), _outfd(0), _selectname(0) {}
  myLevelIter(Xtc* xtc, unsigned depth, int fd, FILE *outfd, char *selectname) :
    XtcIterator(xtc), _depth(depth), _fd(fd), _outfd(outfd), _selectname(selectname) {}

  void process(DetInfo& info, Princeton::ConfigV1& config) {
    info = DetInfo(info.processId(), DetInfo::SxrEndstation, info.detId(), info.device(), info.devId() );
    printf("*** Processing Princeton ConfigV1 object\n");
  }
  void process(DetInfo& info, const Princeton::FrameV1& frame) {
    info = DetInfo(info.processId(), DetInfo::SxrEndstation, info.detId(), info.device(), info.devId() );
    static int frameV1Count = 0;
    if (_fd >= 0) {
      int imageSize = 512 * 512 * 2; // !! hard-coded image size
      int readCount = 0;
      int maxTries = 10;
      while (readCount < imageSize) {
        // seek to beginning of file
        if (lseek(_fd, 0, SEEK_SET) == (off_t)-1) {
          perror("lseek");
          break;
        }
        
        readCount = read(_fd, (void *)frame.data(), imageSize);
        if (-1 == readCount) {
          perror("read");
          break;
        } else if (readCount < imageSize)
          continue;
        // avoid looping forever
        if (maxTries-- < 1) {
          break;
        }
      }
      if (readCount != imageSize) {
        printf(" >> %s Error: readCount=%d, imageSize=%d\n", __FUNCTION__, readCount, imageSize);
      }
    }
    ++frameV1Count;
    printf("*** Processing Princeton FrameV1 object #%d\n", frameV1Count);    
  }
  
  void process(const DetInfo& d, const Camera::FrameV1& f) {
    static int frameV1Count = 0;
    if (_fd >= 0) {
      int width = (f.width() == 1152) ? 576 : f.width();  // FCCD workaround
      int imageSize = width * f.height() * 2;
      int readCount = 0;
      int maxTries = 10;
      while (readCount < imageSize) {
        readCount = read(_fd, (void *)f.data(), imageSize);
        if (-1 == readCount) {
          perror("read");
          break;
        } else if (readCount < imageSize) {
          // seek to beginning of file
          if (lseek(_fd, 0, SEEK_SET) == (off_t)-1) {
            perror("lseek");
            break;
          }
        }
        // avoid looping forever
        if (maxTries-- < 1) {
          break;
        }
      }
      if (readCount != imageSize) {
        printf(" >> %s Error: readCount=%d, imageSize=%d\n", __FUNCTION__, readCount, imageSize);
      }
    }
    ++frameV1Count;
    printf("*** Processing FrameV1 object #%d\n", frameV1Count);
  }
  void process(const DetInfo& info, const FCCD::FccdConfigV2& config) {
    printf("*** Processing FCCD config object\n");
  }
  int process(Xtc* xtc) {
    unsigned i=_depth; while (i--) printf("  ");
    Level::Type level = xtc->src.level();
    printf("%s level  payload size %d contains: %s: ",Level::name(level), xtc->sizeofPayload(), TypeId::name(xtc->contains.id()));
    DetInfo& info = *(DetInfo*)(&xtc->src);
    if (level==Level::Source) {
      printf("%s,%d  %s,%d\n",
             DetInfo::name(info.detector()),info.detId(),
             DetInfo::name(info.device()),info.devId());
    } else {
      ProcInfo& info = *(ProcInfo*)(&xtc->src);
      printf("IpAddress 0x%x ProcessId 0x%x\n",info.ipAddr(),info.processId());
    }
    if (level < 0 || level >= Level::NumberOfLevels )
    {
        printf("Unsupported Level %d\n", (int) level);
        return Continue;
    }    
    switch (xtc->contains.id()) {
    case (TypeId::Id_Xtc) : {
      myLevelIter iter(xtc,_depth+1,_fd,_outfd,_selectname);
      iter.iterate();
      break;
    }
    case (TypeId::Id_Frame) :
      process(info, *(const Camera::FrameV1*)(xtc->payload()));
      break;
    case (TypeId::Id_FccdConfig) :
      process(info, *(const FCCD::FccdConfigV2*)(xtc->payload()));
      break;
    case (TypeId::Id_PrincetonConfig) :
    {
      process( (DetInfo&) info, *(Princeton::ConfigV1*)(xtc->payload()));
      break;
    }
    case (TypeId::Id_PrincetonFrame) :
    {
      process( (DetInfo&) info, *(Princeton::FrameV1*)(xtc->payload()));
      break;
    }        
    default :
      break;
    }
    // if payload name matches the selection, append the payload to the output file
    if (_selectname && (strcmp(TypeId::name(xtc->contains.id()), _selectname) == 0)) {
      if (xtc->sizeofPayload() > 0) {
        if (::fwrite(xtc->payload(),xtc->sizeofPayload(),1,_outfd) != 1) {
          perror("::fwrite");
        }
      } else {
        fprintf(stderr, "Type %s payload not written (size = %d)\n", _selectname, xtc->sizeofPayload());
      }
    }
    return Continue;
  }
private:
  unsigned _depth;
  int _fd;
  FILE *_outfd;
  char *_selectname;
};

void usage(char* progname) {
  fprintf(stderr,"Usage: %s -f <filename> -o <outfilename> [-i <imagefilename>] [-c count] [-s <selectpayloadname>] [-h]\n", progname);
}

void help(void) {
  fprintf(stderr,"\nSupported <selectpayloadname> values:\n");
  for (int ii = (int)TypeId::Id_Frame; ii < (int)TypeId::NumberOf; ii++) {
    fprintf(stderr,"  %-18s", TypeId::name((TypeId::Type)ii));
    if ((ii > 1) && !((ii - 1) % 4))  {
      fprintf(stderr,"\n");
    }
  }
  fprintf(stderr,"\n");
}

int main(int argc, char* argv[]) {
  int c;
  char* xtcname=0;
  char* outfilename=0;
  char* selectname=0;
  char* imagefilename=0;
  int copycount=0;
  int parseErr = 0;

  while ((c = getopt(argc, argv, "hf:o:i:c:s:")) != -1) {
    switch (c) {
    case 'h':
      usage(argv[0]);
      help();
      exit(0);
    case 'f':
      xtcname = optarg;
      printf("xtcmodify file(%s)\n", xtcname);
      break;
    case 's':
      selectname = optarg;
      printf("xtcmodify select(%s)\n", selectname);
      break;
    case 'o':
      outfilename = optarg;
      printf("xtcmodify outfile(%s)\n", outfilename);
      break;
    case 'i':
      imagefilename = optarg;
      break;
    case 'c':
      if (sscanf(optarg, "%d", &copycount) != 1) {
        usage(argv[0]);
        exit(2);
      }
      break;
    default:
      parseErr++;
    }
  }
  
  if (!xtcname || !outfilename) {
    usage(argv[0]);
    exit(2);
  }

  int xtcinfd = open(xtcname, O_RDONLY | O_LARGEFILE);
  if (xtcinfd < 0) {
    perror("Unable to open input file");
    exit(2);
  }

  int imagefd = -1;
  if ((imagefilename) && ((imagefd = open(imagefilename, O_RDONLY)) < 0)) {
    perror("Unable to open image file");
    exit(2);
  }

  FILE* xtcoutfd = fopen(outfilename, "w");
  if (xtcoutfd == NULL) {
    perror("Unable to open output file");
    exit(2);
  }
  
  XtcFileIterator iter(xtcinfd,0x900000);
  Dgram* dg;
  
  while ((dg = iter.next())) {
    printf("%s transition: time 0x%x/0x%x, payloadSize 0x%x\n",TransitionId::name(dg->seq.service()),
           dg->seq.stamp().fiducials(),dg->seq.stamp().ticks(),dg->xtc.sizeofPayload());
    if (selectname) {
      myLevelIter iter(&(dg->xtc),0,imagefd,xtcoutfd,selectname);
      iter.iterate();
    } else {
      myLevelIter iter(&(dg->xtc),0,imagefd);
      iter.iterate();
      if (::fwrite(dg,sizeof(*dg)+dg->xtc.sizeofPayload(),1,xtcoutfd) != 1) {
        perror("::fwrite");
      }
      if ((copycount > 0) && (dg->seq.service() == TransitionId::L1Accept)) {
        int written;
        for (written = 0; written < copycount;) {
          myLevelIter iter(&(dg->xtc),0,imagefd);
          iter.iterate();
          if (::fwrite(dg,sizeof(*dg)+dg->xtc.sizeofPayload(),1,xtcoutfd) == 1) {
            written++;
          } else {
            perror("::fwrite");
            break;
          }
        }
        printf(">>> wrote %d more copies of L1Accept datagram\n", written);
        break;
      }
    }
  }

  ::close(xtcinfd);
  if (imagefd >= 0) {
    ::close(imagefd);
  }
  ::fclose(xtcoutfd);
  return 0;
}
