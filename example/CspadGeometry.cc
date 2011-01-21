#include "CspadGeometry.hh"

#include <math.h>
#include <stdio.h>

static const unsigned MaxSections=32;
static const double dshort = 109.92;
static const double dlong  = 274.80;
//
//  Henrik Lemke's alignment results for quadrant 2 [um and deg]
//
static const double align_x[] = {     0, -23348,  34527,  34801, 
			 	  21646,  45030, -13058, -12846 };
static const double align_y[] = {      0,     78, -12876,  10461,
				  -47173, -47525, -56306, -32895 };
static const double align_rot[] = { -0.316, -0.006, -0.509, -0.649,
			 	    -0.776, -0.810, -0.346, -0.345 };

double& CsVector::operator[](unsigned i)       { return _v[i]; }
double  CsVector::operator[](unsigned i) const { return _v[i]; }

CsVector CsVector::operator+(const CsVector& v) const { 
  CsVector r;
  r[0] = v._v[0]+_v[0];
  r[1] = v._v[1]+_v[1];
  return r;
}

CsVector& CsVector::operator=(const CsVector& v) {
  _v[0] = v._v[0];
  _v[1] = v._v[1];
  return *this;
}

CsVector& CsVector::operator*=(double s) {
  _v[0] *= s; 
  _v[1] *= s;
  return *this; 
}


CsMatrix::CsMatrix() {}

CsMatrix::CsMatrix(double theta)
{
  double c = cos(M_PI/180.*theta);
  double s = sin(M_PI/180.*theta);
  _v[0][0] =  c;
  _v[0][1] = -s;
  _v[1][0] =  s;
  _v[1][1] =  c;
}

CsVector&  CsMatrix::operator[](unsigned i)
{ return _v[i]; }

const CsVector&  CsMatrix::operator[](unsigned i) const
{ return _v[i]; }

CsVector CsMatrix::operator*(const CsVector& v) const {
  CsVector r;
  r[0] = _v[0][0]*v[0] + _v[0][1]*v[1];
  r[1] = _v[1][0]*v[0] + _v[1][1]*v[1];
  return r;
}

CsMatrix CsMatrix::operator+(const CsMatrix& v) const {
  CsMatrix r;
  r[0] = _v[0] + v[0];
  r[1] = _v[1] + v[1];
  return r;
}

CsMatrix& CsMatrix::operator=(const CsMatrix& v) {
  _v[0] = v._v[0];
  _v[1] = v._v[1];
  return *this;
}

SectionGeometry::SectionGeometry() {}

CsVector          SectionGeometry::pixel_centroid  (unsigned col, 
						    unsigned row) const
{
  CsVector v;
  v[0] = dshort*(double(col)-92);
  v[1] = row < 194 ? 
    (-0.5*dlong + (double(row)-193)*dshort) :
    (+0.5*dlong + (double(row)-194)*dshort);
  return translation + rotation*v;
}

//
//  Project/interpolate section data onto a square grid.
//  "grid_spacing" is the step size of the grid in [um]
//  "grid_colstep" is the number of doubles in one row of the grid array
//  "grid_origin"  is the location of the detector origin on the grid
//
void SectionGeometry::map(const CspadSection& data,
			  double*             grid_val,
			  double              grid_size,
			  unsigned            grid_colstep,
			  const CsVector&     grid_offset) const
{
  static unsigned COLS=185;
  static unsigned ROWS=2*194;

  CsVector dx ; dx [0]=dshort/grid_size; dx [1]=               0;
  CsVector dy ; dy [0]=               0; dy [1]=dshort/grid_size;
  CsVector dyl; dyl[0]=               0; dyl[1]= dlong/grid_size;
  dx  = rotation*dx ;
  dy  = rotation*dy ;
  dyl = rotation*dyl;

  const float* p = reinterpret_cast<const float*>(data);

  CsVector v;
  v[0] = -92.5*dshort;
  v[1] = (-0.5*dlong - 193.5*dshort);

  CsVector pcol = translation + rotation*v + grid_offset;
  pcol *= (1./grid_size);
  for(unsigned col=0; col<COLS; col++) {
    CsVector prow = pcol;
    for(unsigned row=0; row<193; row++) {
      _overlay(*p++,prow,dx+dy,grid_val,grid_colstep);
      prow = prow + dy;
    }
    _overlay(*p++,prow,dx+dyl,grid_val,grid_colstep);
    prow = prow + dyl;
    _overlay(*p++,prow,dx+dyl,grid_val,grid_colstep);
    prow = prow + dyl;
    for(unsigned row=195; row<ROWS; row++) {
      _overlay(*p++,prow,dx+dy,grid_val,grid_colstep);
      prow = prow + dy;
    }
    pcol = pcol + dx;
  }
}

//
//  For a given detector pixel, interpolate its value onto the grid
//  weighted by area of overlap
//
void SectionGeometry::_overlay(float d,             // pixel value
                               const CsVector& p,   // pixel corner
			       const CsVector& dxy, // pixel extents
			       double* grid,        // grid array
                               unsigned gcol) const // grid array row size
{
  //
  //  Assume this pixel is oriented to multiples of 90d on the grid
  //  
  CsVector p0 = p;
  CsVector p1 = p+dxy;
  if (p1[0]<p0[0]) { double t=p1[0]; p1[0]=p0[0]; p0[0]=t; }
  if (p1[1]<p0[1]) { double t=p1[1]; p1[1]=p0[1]; p0[1]=t; }

  const CsVector& plo = p0;
  const CsVector& phi = p1;

  const unsigned xlo = unsigned(plo[0]);
  const unsigned ylo = unsigned(plo[1]);
  const unsigned xhi = unsigned(phi[0]);
  const unsigned yhi = unsigned(phi[1]);
  const double dxlo(1-fmod(plo[0],1));
  const double dxhi(fmod(phi[0],1));

  unsigned y(ylo);
  double dy = d*(1-fmod(plo[1],1));
  do { 
    unsigned x=xlo;
    grid[y*gcol+x] += dy*dxlo;

    while(++x < xhi) {
      grid[y*gcol+x] += dy;
    }
    grid[y*gcol+x] += dy*dxhi;

    if (++y < yhi)
      dy = d;
    else if (y==yhi)
      dy = d*fmod(phi[1],1);
    else
      break;
  } while (1);
}


CspadGeometry::CspadGeometry() :
  _sections(new SectionGeometry[MaxSections])
{
  // load constants for quadrant 2 
  //
  // until we know something about the alignment of the quadrants,
  //   place them arbitrarily
  //
  double xoff(-35000), yoff(23000);
  const double rsect[] = { 0, 90, 180, 90 };
  for(int q=0; q<4; q++) {
    CsMatrix rq(q*90);
    for(int i=0; i<8; i++) {
      double rs = rsect[i>>1]+q*90;

      CsMatrix r(align_rot[i]+rs);
      CsVector t; t[0] = xoff-align_x[i]; t[1] = yoff-align_y[i];

      _sections[q*8+i].translation = rq*t;
      _sections[q*8+i].rotation = r;
    }
  }
}

const SectionGeometry& CspadGeometry::section(unsigned quad,
					      unsigned section) const
{ return _sections[quad*8+section]; }




