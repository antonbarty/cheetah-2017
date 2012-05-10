#include "ami/data/EntryScan.hh"

#define SIZE(n) (4*n+InfoSize)

static const unsigned DefaultNbins = 100;

using namespace Ami;

EntryScan::~EntryScan() {}

EntryScan::EntryScan(const Pds::DetInfo& info, unsigned channel,
		     const char* name, 
		     const char* xtitle, 
		     const char* ytitle) :
  _desc(info, channel, name, xtitle, ytitle, DefaultNbins)
{
  build(DefaultNbins);
}

EntryScan::EntryScan(const DescScan& desc) :
  _desc(desc)
{
  build(desc.nbins());
}

void EntryScan::params(unsigned nbins)
{
  _desc.params(nbins);
  build(nbins);
}

void EntryScan::params(const DescScan& desc)
{
  _desc = desc;
  build(_desc.nbins());
}

void EntryScan::build(unsigned nbins)
{
  _p = reinterpret_cast<BinV*>(allocate(SIZE(nbins)*sizeof(double)));
}

const DescScan& EntryScan::desc() const {return _desc;}
DescScan& EntryScan::desc() {return _desc;}

void EntryScan::addy(double y, double x, double w) 
{
  unsigned bin = unsigned(info(Current));
  if (x != _p[bin]._x) {
    if (_p[bin]._nentries!=0)
      bin++;
    if (bin == _desc.nbins())
      bin = 0;

    _p[bin]._x        = x;
    _p[bin]._nentries = w;
    _p[bin]._ysum     = y*w;
    _p[bin]._y2sum    = y*y*w;
    info(bin,Current);
  }
  else
    addy(y,bin,w);
}

void EntryScan::setto(const EntryScan& entry) 
{
  double* dst = reinterpret_cast<double*>(_p);
  const double* end = dst + SIZE(_desc.nbins());
  const double* src = reinterpret_cast<const double*>(entry._p);
  do {
    *dst++ = *src++;
  } while (dst < end);
  valid(entry.time());
}

void EntryScan::diff(const EntryScan& curr, 
		     const EntryScan& prev) 
{
  double* dst = reinterpret_cast<double*>(_p);
  const double* end = dst + SIZE(_desc.nbins());
  const double* srccurr = reinterpret_cast<const double*>(curr._p);
  const double* srcprev = reinterpret_cast<const double*>(prev._p);
  do {
    *dst++ = *srccurr++ - *srcprev++;
  } while (dst < end);
  valid(curr.time());
}

int EntryScan::_insert_bin(const BinV& bv, int& fb)
{
  int ib = desc().nbins()-1;
  while(ib >= fb) {
    if (_p[ib]._x == bv._x) {
      _p[ib]._nentries += bv._nentries;
      _p[ib]._ysum     += bv._ysum    ;
      _p[ib]._y2sum    += bv._y2sum   ;
      return 1;
    }
    ib--;
  }
  if (ib < 0) return 0;

  _p[ib]._nentries = bv._nentries;
  _p[ib]._x        = bv._x       ;
  _p[ib]._ysum     = bv._ysum    ;
  _p[ib]._y2sum    = bv._y2sum   ;
  fb = ib;
  return 1;
}

void EntryScan::sum(const double* a,
		    const double* b)
{
  unsigned nb = desc().nbins();
  const BinV* p_a = reinterpret_cast<const BinV*>(a+1);
  const BinV* p_b = reinterpret_cast<const BinV*>(b+1);
  const double* i_a = reinterpret_cast<const double*>(&p_a[nb]);
  const double* i_b = reinterpret_cast<const double*>(&p_a[nb]);

  int fb = nb;
  int cb_a = int(i_a[Current]), last_a = (cb_a+1)%nb;
  int cb_b = int(i_b[Current]), last_b = (cb_b+1)%nb;
  bool done_a = false, done_b = false;
  do {
    if (cb_a!=last_a)
      if (!_insert_bin(p_a[cb_a],fb)) 
	break;
      else {
	cb_a = (cb_a==0) ? nb-1 : cb_a-1;
	done_a = (cb_a == last_a);
      }
    if (cb_b!=last_b)
      if (!_insert_bin(p_b[cb_b],fb)) 
	break;
      else {
	cb_b = (cb_b==0) ? nb-1 : cb_b-1;
	done_b = (cb_b == last_b);
      }
  } while(!done_a || !done_b);

  while( fb-- )
    _p[fb]._nentries = 0;

  info(double(nb-1), Current);
  info(i_a[Normalization]+i_b[Normalization],Normalization);

  valid( *a<*b ? *b : *a);

//   printf(" bin \t: n_c \t: n_a \t: n_b \t: x_c \t: x_a \t: x_b \t: y_c \t: y_a \t: y_b\n");
//   for(unsigned i=0; i<desc().nbins(); i++) {
//     if (p_a[i]._nentries || p_b[i]._nentries || _p[i]._nentries) {
//       printf(" %03d\t:", i);
//       printf(" %g\t: %g\t: %g\t:", _p[i]._nentries, p_a[i]._nentries, p_b[i]._nentries);
//       printf(" %g\t: %g\t: %g\t:", _p[i]._x       , p_a[i]._x       , p_b[i]._x);
//       printf(" %g\t: %g\t: %g\n" , _p[i]._ysum    , p_a[i]._ysum    , p_b[i]._ysum);
//     }
//   }
}

void EntryScan::sum(const EntryScan& curr, 
		    const EntryScan& prev) 
{
  double* dst = reinterpret_cast<double*>(_p);
  const double* end = dst + SIZE(_desc.nbins());
  const double* srccurr = reinterpret_cast<const double*>(curr._p);
  const double* srcprev = reinterpret_cast<const double*>(prev._p);
  do {
    *dst++ = *srccurr++ + *srcprev++;
  } while (dst < end);
  valid(curr.time());
}
