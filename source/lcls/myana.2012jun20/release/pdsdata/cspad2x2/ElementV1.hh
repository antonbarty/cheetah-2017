#ifndef Pds_Element_2x2_hh
#define Pds_Element_2x2_hh

//
//  A class to iterate through the CSPAD 2x2 detector data
//
//
//  Each "Element" is composed of 2 "Section"s arranged as follows:
//
//  +---+---+   (for quadrant 0)
//  |   |   |
//  | 0 | 1 |
//  |   |   |
//  +---+---+
//
//  Each "Section" is composed of 2*194 rows by 185 columns.  The data order
//  (represented by "Element" definition below") is (sector, column, row):
//  s0_c0_r0, s1_c0_r0, s0_c0_r1, s1_c0_r1, s0_c0_r2, s1_c0_r2, ... // first column from both sections
//  s0_c1_r0, s1_c1_r0, s0_c1_r1, s1_c1_r1, s0_c1_r2, s1_c1_r2, ... // second column from both sections
//  ...                                                             // and so on until
//  s0_cN_r0, s1_cN_r0, s0_cN_r1, s1_cN_r1, s0_cN_r2, s1_cN_r2 ....     // last column from both sections
//
//
//  

#include "pdsdata/cspad2x2/ElementHeader.hh"
#include "pdsdata/cspad2x2/Detector.hh"

#include <stdint.h>

namespace Pds {

  namespace CsPad2x2 {

    class ElementV1 : public ElementHeader {
    public:
      enum {Version=1};
      ElementV1();
    public:
      //  location of individual pixel datum
      uint16_t pixel(unsigned asic, unsigned column, unsigned row) const;
    public:
      struct { 
        uint16_t s0;
        uint16_t s1;
      } pair[ColumnsPerASIC][MaxRowsPerASIC*2];
    };
  };
};
#endif
