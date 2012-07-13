/* $Id: NmipsRecord.cc,v 1.1 2010/12/08 11:45:38 caf Exp $ */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "NmipsRecord.hh"

//
// constructor
//
NmipsRecord::NmipsRecord() :
  _nmips(0),
  _emptyCount(0)
{
}

//
// destructor
//
NmipsRecord::~NmipsRecord()
{
#ifdef DYNAMIC_NMIPS
  if (_nmips > 0) {
    delete[] _smallest;
    delete[] _largest;
  }
#endif
}

int NmipsRecord::init(int nmips)
{
  if ((_nmips == 0) && (nmips > 0)) {
    _nmips = _emptyCount = nmips;
#ifdef DYNAMIC_NMIPS
    _smallest = new float[nmips];
    _largest = new float[nmips];
#endif
  }
  return (0);
}

int NmipsRecord::count()
{
  return (2 * _nmips);
}

void NmipsRecord::insert(float value)
{
  int ii;
  int minpos, maxpos;
  float minval, maxval;

  if (_nmips <= 0) {
    // return if not initialized
    return;
  }
  if (_emptyCount > 0) {
    // if this is one of the first <nmips> values, add it to both
    // the largest and smallest lists.
    _smallest[_nmips - _emptyCount] = value;
    _largest[_nmips - _emptyCount] = value;
    _emptyCount --;
  } else {
    // if value is greater than the smallest of the largest, replace it
    minpos = 0;
    minval = _largest[0];
    for (ii = 1; ii < _nmips; ii++) {
      if (_largest[ii] < minval) {
        minpos = ii;
        minval = _largest[ii];
      }
    }
    if (value > minval) {
      _largest[minpos] = value;
    }
    // if value is less than the largest of the smallest, replace it
    maxpos = 0;
    maxval = _smallest[0];
    for (ii = 1; ii < _nmips; ii++) {
      if (_smallest[ii] > maxval) {
        maxpos = ii;
        maxval = _smallest[ii];
      }
    }
    if (value < maxval) {
      _smallest[maxpos] = value;
    }
  }
}

float NmipsRecord::sum()
{
  int ii;
  float total = 0.0;

  for (ii = 0; ii < _nmips; ii++) {
    total += (_smallest[ii] + _largest[ii]);
  }
  return (total);
}

void NmipsRecord::dump()
{
  int ii;

  if (_nmips <= 0) {
    printf("not initialized\n");
  } else {
    printf("largest: ");
    for (ii = 0; ii < _nmips; ii++) {
      printf("%f ", _largest[ii]);
    }
    printf("\n");
    printf("smallest: ");
    for (ii = 0; ii < _nmips; ii++) {
      printf("%f ", _smallest[ii]);
    }
    printf("\n");
  }
}
