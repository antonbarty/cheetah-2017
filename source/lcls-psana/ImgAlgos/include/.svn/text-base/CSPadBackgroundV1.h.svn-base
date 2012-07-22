#ifndef PDSCALIBDATA_CSPADBACKGROUNDV1_H
#define PDSCALIBDATA_CSPADBACKGROUNDV1_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class CSPadBackgroundV1.
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
 *  Gets, holds, and provides an access to the (4 quad x 8 sect x 185 x 388) 
 *  pixel background amplitudes of the CSPad
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

class CSPadBackgroundV1  {
public:

    enum { Quads         = Psana::CsPad::MaxQuadsPerSensor }; // 4
    enum { Sectors       = Psana::CsPad::SectorsPerQuad    }; // 8
    enum { Columns       = Psana::CsPad::ColumnsPerASIC    }; // 185 THERE IS A MESS IN ONLINE COLS<->ROWS
    enum { Rows          = Psana::CsPad::MaxRowsPerASIC*2  }; // 388 THERE IS A MESS IN ONLINE COLS<->ROWS 
    enum { SectorSize    = Columns * Rows                  }; // 185 * 388
    enum { SIZE_OF_ARRAY = Quads * Sectors * SectorSize    };

    typedef double background_t; 

  // Constructors
  CSPadBackgroundV1 () ;

  CSPadBackgroundV1( const std::string& fname );

  // Destructor
  virtual ~CSPadBackgroundV1 () ;


  // Access methods
  background_t getBackground(size_t quad, size_t sect, size_t col, size_t row){ return m_bkgd[quad][sect][col][row]; };

  background_t* getBackground(){ return &m_bkgd[0][0][0][0]; };

  background_t* getBackground(size_t quad, size_t sect){ return &m_bkgd[quad][sect][0][0]; };

  ndarray<background_t, 4> getBackground() const {
    return make_ndarray(&m_bkgd[0][0][0][0], Quads, Sectors, Columns, Rows);
  }

  void  print();


protected:
  void fillArrFromVector( const std::vector<background_t> v_pars );

private:

  background_t m_bkgd[Quads][Sectors][Columns][Rows];  

  // Copy constructor and assignment are disabled by default
  CSPadBackgroundV1 ( const CSPadBackgroundV1& ) ;
  CSPadBackgroundV1& operator = ( const CSPadBackgroundV1& ) ;

};

} // namespace ImgAlgos

#endif // PDSCALIBDATA_CSPADBACKGROUNDV1_H
