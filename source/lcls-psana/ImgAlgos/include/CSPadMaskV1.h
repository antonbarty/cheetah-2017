#ifndef PDSCALIBDATA_CSPADMASKV1_H
#define PDSCALIBDATA_CSPADMASKV1_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class CSPadMaskV1.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <vector>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "psddl_psana/cspad.ddl.h" // for Psana::CsPad::MaxQuadsPerSensor etc.
#include "ndarray/ndarray.h"

namespace ImgAlgos {

/**
 *  Gets, holds, and provides an access to the ( (4 quad x 8 sect x 185) x 388) 
 *  pixel mask array of the CSPad
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @see AdditionalClass
 *
 *  @version $Id$
 *
 *  @author Mikhail S. Dubrovin
 */

class CSPadMaskV1  {
public:

    enum { Quads         = Psana::CsPad::MaxQuadsPerSensor }; // 4
    enum { Sectors       = Psana::CsPad::SectorsPerQuad    }; // 8
    enum { Columns       = Psana::CsPad::ColumnsPerASIC    }; // 185 THERE IS A MESS IN ONLINE COLS<->ROWS
    enum { Rows          = Psana::CsPad::MaxRowsPerASIC*2  }; // 388 THERE IS A MESS IN ONLINE COLS<->ROWS 
    enum { SectorSize    = Columns * Rows                  }; // 185 * 388
    enum { SIZE_OF_ARRAY = Quads * Sectors * SectorSize    };

    typedef uint16_t mask_t; 

  // Constructors
  CSPadMaskV1 () ;

  CSPadMaskV1( const std::string& fname );

  // Destructor
  virtual ~CSPadMaskV1 () ;

  // Access methods
  mask_t  getMask(size_t quad, size_t sect, size_t col, size_t row){ return m_mask[quad][sect][col][row]; };

  mask_t* getMask(){ return &m_mask[0][0][0][0]; };

  mask_t* getMask(size_t quad, size_t sect){ return &m_mask[quad][sect][0][0]; };

  ndarray<mask_t, 4> getMask() const {
    return make_ndarray(&m_mask[0][0][0][0], Quads, Sectors, Columns, Rows);
  }

  void  print();


protected:
  void fillArrFromVector( const std::vector<mask_t> v_pars );

private:

  mask_t m_mask[Quads][Sectors][Columns][Rows];  

  // Copy constructor and assignment are disabled by default
  CSPadMaskV1 ( const CSPadMaskV1& ) ;
  CSPadMaskV1& operator = ( const CSPadMaskV1& ) ;

};

} // namespace ImgAlgos

#endif // PDSCALIBDATA_CSPADMASKV1_H
