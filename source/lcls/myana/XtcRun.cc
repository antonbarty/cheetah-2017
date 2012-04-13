#include "XtcRun.hh"

#include "pdsdata/xtc/Sequence.hh"
#include "pdsdata/xtc/TransitionId.hh"

#include <fcntl.h>

static bool _live=false;

void XtcSlice::live_read(bool l) { _live=l; }


XtcSlice::XtcSlice(std::string fname) :
  _base(fname.substr(0,fname.find("-c"))),
  _current(_chunks.begin()),
  _fd(-1)
{
  _chunks.push_back(fname);
}

XtcSlice::~XtcSlice() 
{
  if (_fd) ::close(_fd); 
}

bool XtcSlice::add_file(std::string fname) 
{
  if (fname.compare(0,_base.size(),_base)!=0) 
    return false;
  
  for(std::list<std::string>::iterator it=_chunks.begin();
      it!=_chunks.end(); it++)
    if (fname < (*it)) {
      _chunks.insert(it,fname);
      return true;
    }
  _chunks.push_back(fname);
  return true;
}

void XtcSlice::init()
{
  _current = _chunks.begin();
  if (!_open())
    exit(1);
}

Result XtcSlice::next(Pds::Dgram* dg)
{
  //  Fill the header
  memcpy(dg, &_hdr, sizeof(Pds::Dgram));
  if (dg->xtc.sizeofPayload() > 18000000)
    {
      printf("*** Datagram too large %d.  Aborting run.\n",
             dg->xtc.sizeofPayload());
      return Error;
    }

  //  Read and fill the payload
  Result r = _read(dg->xtc.payload(), dg->xtc.sizeofPayload(), false);
  if (r != OK) {
    printf("Unexpected eof in %s\n",_current->data());
    return r;
  }

  return _next();
}

Result XtcSlice::skip()
{
  off_t off = _hdr.xtc.sizeofPayload();
  if (lseek64(_fd, off, SEEK_CUR) == (off_t)-1) {
    printf("Unexpected eof in %s\n",_current->data());
    return Error;
  }
  
  return _next();
}

bool XtcSlice::_open()
{
  const char* fname = _current->data();
  _fd = ::open(fname,O_LARGEFILE,O_RDONLY);
  if (_fd == -1) {
    char buff[256];
    sprintf(buff,"Error opening file %s\n",fname);
    perror(buff);
    return false;
  }
  else {
    //      printf("Opened file %s\n",fname);
  }
  ::read(_fd, &_hdr, sizeof(Pds::Dgram));
  return true;
}

Result XtcSlice::_next()
{
  if (_hdr.seq.service()==Pds::TransitionId::EndRun) {
    //      printf("Reached eor in %s\n",_current->data());
    ::close(_fd); _fd = 0;
    return End;
  }
     
  //  Read the next header
  Result r = _read(&_hdr, sizeof(Pds::Dgram), true);
  if (r != OK) {
    if (!_live) {
      printf("Unexpected eof in %s\n",_current->data());
      ::close(_fd); _fd = 0;
      if (++_current == _chunks.end()) 
        return End;
      if (!_open())
        return Error;
	  return OK;
    }
    return r;
  }

  return OK;
}

Result XtcSlice::_read(void* buf, ssize_t insz, bool seekNewChunk)
{
  char* p = (char*)buf;

  ssize_t sz  = insz;
  ssize_t rsz = ::read(_fd, p, sz);
  p  += rsz;
  sz -= rsz;

  if (sz == 0)
    return OK;

  if (!_live)
    return Error;

  while(sz) {
    timespec tp;
    tp.tv_sec  = 1;
    tp.tv_nsec = 0;
    nanosleep(&tp,0);

    if (sz==insz && seekNewChunk) {
      int chunkPosition = _current->find("-c")+2;
      int chunkIndex = strtoul(_current->data()+chunkPosition, NULL, 10)+1;
      char chunkBuff[4]; sprintf(chunkBuff,"%02d",chunkIndex);
      std::string fname = _current->substr(0,chunkPosition) + std::string(chunkBuff) +
        _current->substr(chunkPosition+2);
	
      struct stat _stat;
      if (stat(fname.data(),&_stat)==0) {
        _chunks.push_back(fname);
        _current = _chunks.begin();
        while( *_current != fname )
          ++_current;
        ::close(_fd);
        if (!_open())
          return Error;
        else
          return OK;
      }
    }

    if ((rsz = ::read(_fd, p, sz))==-1) {
      perror("Error reading from file");
      return Error;
    }
    p  += rsz;
    sz -= rsz;
  }

  return OK;
}



XtcRun::XtcRun() {}

XtcRun::~XtcRun() 
{
  for(std::list<XtcSlice*>::iterator it=_slices.begin();
      it!=_slices.end(); it++)
    delete (*it);
  _slices.clear();
} 

void XtcRun::reset   (std::string fname) 
{
  for(std::list<XtcSlice*>::iterator it=_slices.begin();
      it!=_slices.end(); it++)
    delete (*it);
  _slices.clear();
  _slices.push_back(new XtcSlice(fname));
  _base = fname.substr(0,fname.find("-s"));
}

bool XtcRun::add_file(std::string fname) 
{
  if (fname.compare(0,_base.size(),_base)!=0)
    return false;

  for(std::list<XtcSlice*>::iterator it=_slices.begin();
      it!=_slices.end(); it++)
    if ((*it)->add_file(fname))
      return true;
  _slices.push_back(new XtcSlice(fname));
  return true;
}

const char* XtcRun::base() const
{ return _base.data(); }

unsigned XtcRun::run_number() const 
{ return atoi(_base.data()+_base.find("-r")+2); }


void XtcRun::init() 
{
  for(std::list<XtcSlice*>::iterator it=_slices.begin();
      it!=_slices.end(); it++)
    (*it)->init();
}

Result XtcRun::next(Pds::Dgram* dg)
{
  //
  //  Process L1A with lowest clock time first
  //
  Pds::ClockTime tmin(-1,-1);
  std::list<XtcSlice*>::iterator n  = _slices.begin();
  for(std::list<XtcSlice*>::iterator it = _slices.begin();
      it != _slices.end(); it++) {
    if ((*it)->hdr().seq.service()==Pds::TransitionId::L1Accept &&
        tmin > (*it)->hdr().seq.clock())
      tmin = (*(n = it))->hdr().seq.clock();
  }

  //
  //  On a transition, advance all slices
  //
  if ((*n)->hdr().seq.service()!=Pds::TransitionId::L1Accept) {
    for(std::list<XtcSlice*>::iterator it = _slices.begin();
        it != _slices.end();) {
      if (it != n && (*it)->skip()==End) {
        delete (*it);
        std::list<XtcSlice*>::iterator ee = it++;
        _slices.erase(ee);
      }
      else
        it++;
    }
  }
  Result r = (*n)->next(dg);
  if (r == End) {
    delete (*n);
    _slices.erase(n);
    if (_slices.size())
      r = OK;
  }
  return r;
}
