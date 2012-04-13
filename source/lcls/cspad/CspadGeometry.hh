#ifndef CspadGeometry_hh
#define CspadGeometry_hh

#include "CspadCorrector.hh"

//
//  Class for 2-D vector operations
//
class CsVector {
public:
  double& operator[](unsigned);
  double  operator[](unsigned) const;
public:
  CsVector  operator+(const CsVector&) const;
  CsVector& operator=(const CsVector&);
  CsVector& operator*=(double);
private:
  double _v[2];
};

//
//  Class for 2-D matrix operations
//
class CsMatrix {
public:
  CsMatrix();
  CsMatrix(double theta);
public:
  CsVector&        operator[](unsigned);
  const CsVector&  operator[](unsigned) const;
public:
  CsVector operator*(const CsVector&) const;
  CsMatrix operator+(const CsMatrix&) const;
  CsMatrix& operator=(const CsMatrix&);
private:
  CsVector _v[2];
};

//
//  Location and orientation of a single section ("2 x 1")
//
class SectionGeometry {
public:
  SectionGeometry();
public:
  //  Centroid [um] {x,y} 
  CsVector          pixel_centroid  (unsigned col, 
				     unsigned row) const;
public:
  //
  //  Project/interpolate section data onto a square grid.
  //  "grid_spacing" is the step size of the grid in [um]
  //  "grid_colstep" is the number of doubles in one row of the grid array
  //  "grid_origin"  is the location of the detector origin on the grid
  //
  void map(const CspadSection& data,
	   double*             grid_val,
	   double              grid_spacing,
	   unsigned            grid_colstep,
	   const CsVector&     grid_origin) const;
  
private:
  void _overlay(float d, const CsVector& p,
		const CsVector& dxy,
		double* grid, unsigned gcol) const;

public:
  CsVector translation;  // center of the section
  CsMatrix rotation;     // global orientation
};

//
//  Collection of section geometries
//
class CspadGeometry {
public:
  CspadGeometry();
public:
  //
  //  Return the geometry of an individual section ("2 x 1")
  //
  const SectionGeometry& section(unsigned quad,
				 unsigned section) const;
private:
  SectionGeometry* _sections;
};

#endif
