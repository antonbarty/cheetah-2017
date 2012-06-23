#include "CspadGeometry.hh"

#include <math.h>
#include <stdio.h>

static const unsigned MaxSections=32;
static const double dshort = 109.92;
static const double dlong  = 274.80;
//
//  Henrik Lemke's alignment results for quadrant 2 [um and deg]
//
static const double cspad1_align_x[] = 
  {     0, -23348,  34527,  34801,  21646,  45030, -13058, -12846 };
static const double cspad1_align_y[] = 
  {      0,     78, -12876, 10461, -47173, -47525, -56306, -32895 };
static const double cspad1_align_rot[] =
  { -0.316, -0.006, -0.509, -0.649, -0.776, -0.810, -0.346, -0.345 };

//
//  CSPAD 2 Centers and Rotations
//
struct section_align { double x; double y; double rot; };
typedef struct section_align section_align_s;
#if 0
//  Pre Run3 Metrology
static const section_align_s cspad2_q0_align[] = { 
  { -125162, -73311, 0.220436 },
  { -101705, -73312.5, 0.0284687 },
  { -160112, -61032.8, 0.214426 },
  { -160125, -84431, 0.208162 },
  { -147726, -26302.5, 0.196363 },
  { -171111, -26311, 0.201227 },
  { -113398, -17008.2, 0.0728726 },
  { -113358, -40350.5, 0.136905 }
};

static const section_align_s cspad2_q1_align[] = { 
  { -69033.2, -56531.2, 0.0558174 },
  { -68957.2, -79885, 0.36166 },
  { -79945, -21857.5, -0.167872 },
  { -56603.5, -21934.8, -0.01712 },
  { -21774.5, -10452.2, -0.00922993 },
  { -21834.5, -33807, -0.0597204 },
  { -12486.5, -68717.8, 0.20892 },
  { -35876, -68766.8, 0.17523 }
};

static const section_align_s cspad2_q2_align[] = { 
  { -52564.2, -112849, 0.0352972 },
  { -75961.8, -112930, 0.353318 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { -64347.2, -169281, 0.139293 },
  { -64404.2, -145915, 0.199367 }
};
static const section_align_s cspad2_q3_align[] = { 
  { -108425, -129401, 0.18157 },
  { -108480, -106028, 0.178244 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { -164802, -117556, 0.0580212 },
  { -141479, -117504, 0.362906 }
};
#else
#if 0
//  Anton Barty's Run3 alignment
static const section_align_s cspad2_q0_align[] = { 
  { -36819, 21212, 0 },
  { -13926, 20118, 0 },
  { -72896, 32823, 0 },
  { -72690, 9654 , 0 },
  { -59943, 68875, 0 },
  { -83518, 69069, 0 },
  { -24211, 77285, 0 },
  { -24493, 54274, 0 }
};
static const section_align_s cspad2_q1_align[] = { 
  { 20883,  36403, 0 },
  { 20345,  13136, 0 },
  { 33381,  71226, 0 },
  {  9854,  71589, 0 },
  { 67915,  58959, 0 },
  { 68070,  82299, 0 },
  { 77062,  24509, 0 },
  { 53778,  24501, 0 }
};
static const section_align_s cspad2_q2_align[] = { 
  { 37709,  -20925, 0 },
  { 13851,  -21252, 0 },
  { 72591,  -32254, 0 },
  { 72511,  -9874 , 0 },
  { 61387,  -66403, 0 },
  { 82655,  -66183, 0 },
  { 25190,  -78052, 0 },
  { 25092,  -54662, 0 }
};
static const section_align_s cspad2_q3_align[] = { 
  { -20991, -37204, 0 },
  { -21111, -13834, 0 },
  { -32802, -72309, 0 },
  { -9633,  -72402, 0 },
  { -69336, -59277, 0 },
  { -69193, -82613, 0 },
  { -78266, -25060, 0 },
  { -54186, -25374, 0 }
};
#else
//  Post Run3 Metrology
static const section_align_s cspad2_q0_align[] = {
{ -33855.2, 21828.8, -0.279709 },
{ -10469.2, 21768.5, 0.0283592 },
{ -68903.5, 34121, -0.34487 },
{ -68989, 10796, -0.0791325 },
{ -56921.2, 69167.2, -0.154696 },
{ -80301, 69250.5, -0.123441 },
{ -22071.2, 78275, 0.090546 },
{ -22008, 54950.5, 0.389862 },
};
static const section_align_s cspad2_q1_align[] = {
{ 21808, 33894, 0.151922 },
{ 21778.2, 10452, -0.00834655 },
{ 34135.8, 68826.8, 0.133881 },
{ 10737, 68874, 0.120768 },
{ 68853.5, 56396.5, 0.104317 },
{ 68874.2, 79786.2, 0.112048 },
{ 78097.2, 22057.8, -0.00486948 },
{ 54757, 22046.5, 0.0575798 },
};
static const section_align_s cspad2_q2_align[] = {
{ 34033.5, -22048.2, -0.327842 },
{ 10688.2, -21961, -0.0205296 },
{ 68408.8, -34614.5, -0.647003 },
{ 68783, -11369.5, -0.553163 },
{ 56439.2, -69398.8, -0.449492 },
{ 79797.8, -69617.2, -0.396346 },
{ 21960.2, -78510.8, -0.0938215 },
{ 21957.5, -55118.5, -0.110921 },
};
static const section_align_s cspad2_q3_align[] = {
{ -21861.5, -33833.8, -0.348006 },
{ -21783.8, -10451.5, -0.0178942 },
{ -34157.8, -69071.8, -0.0178058 },
{ -10786.2, -69077, 0.0412966 },
{ 0, 0, 0 },
{ -69184.2, -80319, 0.346376 },
{ 0, 0, 0 },
{ -54959, -22309.8, 0.0940429 },
};
#endif
#endif

static const section_align_s* cspad2_align[] = { cspad2_q0_align,
                                                 cspad2_q1_align,
                                                 cspad2_q2_align,
                                                 cspad2_q3_align };


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


CspadGeometry::CspadGeometry(Pds::DetInfo::Detector detinfo) :
  _sections(new SectionGeometry[MaxSections])
{
  const double rsect[] = { 0, 90, 180, 90 };

  if (detinfo==Pds::DetInfo::XppGon) {
    // load constants for quadrant 2 
    //
    // until we know something about the alignment of the quadrants,
    //   place them arbitrarily
    //
    double xoff(-35000), yoff(23000);
    for(int q=0; q<4; q++) {
      CsMatrix rq(q*90);

      const double *align_x, *align_y, *align_rot;
      align_x   = cspad1_align_x;
      align_y   = cspad1_align_y;
      align_rot = cspad1_align_rot;
      for(int i=0; i<8; i++) {
        double rs = -rsect[i>>1]+q*90;

        CsMatrix r(align_rot[i]+rs);
        CsVector t; 
        t[0] = xoff-align_x[i]; t[1] = yoff-align_y[i];

        _sections[q*8+i].translation = rq*t;
        _sections[q*8+i].rotation = r;
      }
    }
  }
  else {
    //    double xoff(-88776), yoff(-93039);
    double xoff(0), yoff(0);
    for(int q=0; q<4; q++) {

      const section_align_s* align = cspad2_align[q];

      for(int i=0; i<8; i++) {
        double rs = -rsect[i>>1]-q*90;

        CsMatrix r(align[i].rot+rs);
        CsVector t; 
        t[0] = align[i].x-xoff; t[1] = align[i].y-yoff;

        _sections[q*8+i].translation = t;
        _sections[q*8+i].rotation = r;
      }
    }
  }
}

const SectionGeometry& CspadGeometry::section(unsigned quad,
					      unsigned section) const
{ return _sections[quad*8+section]; }




