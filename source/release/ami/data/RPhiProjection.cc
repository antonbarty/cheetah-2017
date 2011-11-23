#include "RPhiProjection.hh"

#include "ami/data/DescEntry.hh"
#include "ami/data/DescTH1F.hh"
#include "ami/data/DescProf.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryProf.hh"
#include "ami/data/EntryFactory.hh"

#include "ami/data/Cds.hh"

#include <stdio.h>
#include <math.h>

using namespace Ami;

RPhiProjection::RPhiProjection(const DescEntry& output, 
			       Axis axis, double lo, double hi,
			       double xc, double yc) :
  AbsOperator(AbsOperator::RPhiProjection),
  _axis      (axis),
  _lo        (lo),
  _hi        (hi),
  _xc        (xc),
  _yc        (yc),
  _output    (0)
{
  memcpy(_desc_buffer, &output, output.size());
}

RPhiProjection::RPhiProjection(const char*& p, const DescEntry& input) :
  AbsOperator(AbsOperator::RPhiProjection)
{
  _extract(p, _desc_buffer, DESC_LEN);
  _extract(p, &_axis      , sizeof(_axis));
  _extract(p, &_lo        , sizeof(_lo ));
  _extract(p, &_hi        , sizeof(_hi ));
  _extract(p, &_xc        , sizeof(_xc  ));
  _extract(p, &_yc        , sizeof(_yc  ));

  const DescEntry& o = *reinterpret_cast<const DescEntry*>(_desc_buffer);
  _output = static_cast<EntryTH1F*>(EntryFactory::entry(o));
}

RPhiProjection::RPhiProjection(const char*& p) :
  AbsOperator(AbsOperator::RPhiProjection),
  _output    (0)
{
  _extract(p, _desc_buffer, DESC_LEN);
  _extract(p, &_axis      , sizeof(_axis));
  _extract(p, &_lo        , sizeof(_lo ));
  _extract(p, &_hi        , sizeof(_hi ));
  _extract(p, &_xc        , sizeof(_xc  ));
  _extract(p, &_yc        , sizeof(_yc  ));
}

RPhiProjection::~RPhiProjection()
{
  if (_output) delete _output;
}

DescEntry& RPhiProjection::output   () const 
{ 
  return _output ? _output->desc() : *reinterpret_cast<DescEntry*>(const_cast<char*>(_desc_buffer)); 
}

void*      RPhiProjection::_serialize(void* p) const
{
  _insert(p, _desc_buffer, DESC_LEN);
  _insert(p, &_axis      , sizeof(_axis));
  _insert(p, &_lo        , sizeof(_lo ));
  _insert(p, &_hi        , sizeof(_hi ));
  _insert(p, &_xc        , sizeof(_xc  ));
  _insert(p, &_yc        , sizeof(_yc  ));
  return p;
}

Entry&     RPhiProjection::_operate(const Entry& e) const
{
  if (!e.valid())
    return *_output;

  const EntryImage* _input = static_cast<const EntryImage*>(&e);
  const DescImage& inputd = _input->desc();
  const double           p = _input->info(EntryImage::Pedestal);
  double xc = _xc;
  double yc = _yc;
  double lo = _lo;
  double hi = _hi;
  if (_input) {
    switch(output().type()) {
    case DescEntry::TH1F:  // unnormalized
      { const DescTH1F& d = static_cast<const DescTH1F&>(output());
	EntryTH1F*      o = static_cast<EntryTH1F*>(_output);
	//  Loop over all pixels in the rectangular region within distance R
	o->reset();
	if (_axis == R) {
	  int x0,x1,y0,y1;
	  if (inputd.nframes()) {
	    for(unsigned fn=0; fn<inputd.nframes(); fn++)
	      if (inputd.rphi_bounds(x0,x1,y0,y1,
				     xc,yc, d.xup(),fn)) {
		for(int j=y0; j<y1; j++) {
		  double dy  = inputd.biny(j)-yc;
		  double dy2 = dy*dy;
		  for(int k=x0; k<x1; k++) {
		    double dx  = inputd.binx(k)-xc;
		    double dx2 = dx*dx;
		    double f   = atan2(dy,dx);
		    if ( (f>=lo && f<=hi) ||
			 (f+2*M_PI <= hi) )
		      o->addcontent(_input->content(k,j)-p,sqrt(dx2+dy2));
		  }
		}
	      }
	  }
	  else if (inputd.rphi_bounds(x0,x1,y0,y1,
                                      xc,yc, d.xup())) {
	    for(int j=y0; j<y1; j++) {
	      double dy  = inputd.biny(j)-yc;
	      double dy2 = dy*dy;
	      for(int k=x0; k<x1; k++) {
		double dx  = inputd.binx(k)-xc;
		double dx2 = dx*dx;
		double f   = atan2(dy,dx);
		if ( (f>=lo && f<=hi) ||
		     (f+2*M_PI <= hi) )
		  o->addcontent(_input->content(k,j)-p,sqrt(dx2+dy2));
	      }
	    }
	  }
	}
	else { // (_axis == Phi)
	  int x0,x1,y0,y1;
	  if (inputd.nframes()) {
	    for(unsigned fn=0; fn<inputd.nframes(); fn++)
	      if (inputd.rphi_bounds(x0,x1,y0,y1,
				     _xc,_yc, hi, fn)) {
		double losq = lo*lo;
		double hisq = hi*hi;
		for(int j=y0; j<y1; j++) {
		  double dy  = inputd.biny(j)-yc;
		  double dy2 = dy*dy;
		  for(int k=x0; k<x1; k++) {
		    double dx  = inputd.binx(k)-xc;
		    double dx2 = dx*dx;
		    double rsq = dx2 + dy2;
		    if (rsq >= losq && rsq <= hisq) {
		      double f = atan2(dy,dx);
		      double y = _input->content(k,j)-p;
		      o->addcontent(y, f);
		      o->addcontent(y, f+2*M_PI);
		    }
		  }
		}
	      }
	  }
	  else if (inputd.rphi_bounds(x0,x1,y0,y1,
				 _xc,_yc, hi)) {
	    double losq = lo*lo;
	    double hisq = hi*hi;
	    for(int j=y0; j<y1; j++) {
	      double dy  = inputd.biny(j)-yc;
	      double dy2 = dy*dy;
	      for(int k=x0; k<x1; k++) {
		double dx  = inputd.binx(k)-xc;
		double dx2 = dx*dx;
		double rsq = dx2 + dy2;
		if (rsq >= losq && rsq <= hisq) {
		  double f = atan2(dy,dx);
		  double y = _input->content(k,j)-p;
		  o->addcontent(y, f);
		  o->addcontent(y, f+2*M_PI);
		}
	      }
	    }
	  }
	}
	o->info(_input->info(EntryImage::Normalization),EntryTH1F::Normalization);
	break; }
    case DescEntry::Prof:  // normalized
      { const DescProf& d = static_cast<const DescProf&>(output());
	EntryProf*      o = static_cast<EntryProf*>(_output);
	//  Loop over all pixels in the rectangular region within distance R
	o->reset();
	if (_axis == R) {
	  int x0,x1,y0,y1;
	  if (inputd.nframes()) {
	    for(unsigned fn=0; fn<inputd.nframes(); fn++)
	      if (inputd.rphi_bounds(x0,x1,y0,y1,
				     _xc,_yc, d.xup(),fn)) {
		for(int j=y0; j<y1; j++) {
		  double dy  = inputd.biny(j)-yc;
		  double dy2 = dy*dy;
		  for(int k=x0; k<x1; k++) {
		    double dx  = inputd.binx(k)-xc;
		    double dx2 = dx*dx;
		    double f   = atan2(dy,dx);
		    if ( (f>=lo && f<=hi) ||
			 (f+2*M_PI <= hi) )
		      o->addy(_input->content(k,j)-p,sqrt(dx2+dy2));
		  }
		}
	      }
	  }
	  else if (inputd.rphi_bounds(x0,x1,y0,y1,
				      _xc,_yc, d.xup())) {
	    for(int j=y0; j<y1; j++) {
	      double dy  = inputd.biny(j)-yc;
	      double dy2 = dy*dy;
	      for(int k=x0; k<x1; k++) {
		double dx  = inputd.binx(k)-xc;
		double dx2 = dx*dx;
		double f   = atan2(dy,dx);
		if ( (f>=lo && f<=hi) ||
		     (f+2*M_PI <= hi) )
		  o->addy(_input->content(k,j)-p,sqrt(dx2+dy2));
	      }
	    }
	  }
	}
	else { // (_axis == Phi)
	  int x0,x1,y0,y1;
	  if (inputd.nframes()) {
	    for(unsigned fn=0; fn<inputd.nframes(); fn++)
	      if (inputd.rphi_bounds(x0,x1,y0,y1,
				     _xc,_yc, hi, fn)) {
		double losq = lo*lo;
		double hisq = hi*hi;
		for(int j=y0; j<y1; j++) {
		  double dy  = inputd.biny(j)-yc;
		  double dy2 = dy*dy;
		  for(int k=x0; k<x1; k++) {
		    double dx  = inputd.binx(k)-xc;
		    double dx2 = dx*dx;
		    double rsq = dx2 + dy2;
		    if (rsq >= losq && rsq <= hisq) {
		      double f = atan2(dy,dx);
		      double y = _input->content(k,j)-p;
		      o->addy(y, f);
		      o->addy(y, f+2*M_PI);
		    }
		  }
		}
	      }
	  }
	  else if (inputd.rphi_bounds(x0,x1,y0,y1,
				      _xc,_yc, hi)) {
	    double losq = lo*lo;
	    double hisq = hi*hi;
	    for(int j=y0; j<y1; j++) {
	      double dy  = inputd.biny(j)-yc;
	      double dy2 = dy*dy;
	      for(int k=x0; k<x1; k++) {
		double dx  = inputd.binx(k)-xc;
		double dx2 = dx*dx;
		double rsq = dx2 + dy2;
		if (rsq >= losq && rsq <= hisq) {
		  double f = atan2(dy,dx);
		  double y = _input->content(k,j)-p;
		  o->addy(y, f);
		  o->addy(y, f+2*M_PI);
		}
	      }
	    }
	  }
	}
	o->info(_input->info(EntryImage::Normalization),EntryProf::Normalization);
	break; }
    default:
      break; 
    }
    _output->valid(e.time());
  }
  return *_output;
}

