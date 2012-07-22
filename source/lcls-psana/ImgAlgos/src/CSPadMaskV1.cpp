//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class CSPadMaskV1...
//
// Author List:
//      Mikhail S. Dubrovin
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "ImgAlgos/CSPadMaskV1.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <fstream>
#include <stdexcept>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"

using namespace std;

namespace ImgAlgos {

//----------------
// Constructors --
//----------------

CSPadMaskV1::CSPadMaskV1 ()
{
  // Fill the mask array by zeros
  std::fill_n(&m_mask[0][0][0][0], int(SIZE_OF_ARRAY), mask_t(0));
}

//----------------

CSPadMaskV1::CSPadMaskV1( const std::string& fname )
{
  // Open file
  std::ifstream in(fname.c_str());
  if (not in.good()) {
    const std::string msg = "Failed to open the mask file: "+fname;
    MsgLogRoot(error, msg);
    throw std::runtime_error(msg);
  }

  // Read the entire file content in vector
  std::vector<mask_t> v_pars;
  std::string str;  
  do{ 
      in >> str; 
      if   ( in.good() ) v_pars.push_back(mask_t(std::atof(str.c_str()))); 
    } while( in.good() );

  // Close file
  in.close();

  // Check and copy the vector in array 
  fillArrFromVector(v_pars);
}

//----------------

void CSPadMaskV1::fillArrFromVector( const std::vector<mask_t> v_parameters )
{
  cout << "\nCSPadMaskV1:\n";
    if (v_parameters.size() != SIZE_OF_ARRAY) {
        WithMsgLog("CSPadMaskV1", error, str) {
        str << "Expected number of parameters is " << SIZE_OF_ARRAY ;
        str << ", read from file " << v_parameters.size() ;
        str << ": check the file.\n" ;
        }       
        const std::string msg = "The data size available in file for CSPad mask is wrong.";
        MsgLogRoot(error, msg);
        throw std::runtime_error(msg);
        //abort();
    }

    size_t arr_size = sizeof( mask_t ) * v_parameters.size();
    memcpy( &m_mask, &v_parameters[0], arr_size );
    //this->print();
}

//----------------

void CSPadMaskV1::print()
{
  MsgLog("CSPadMaskV1::print()",  info, "Print part of the data for test purpose only.");
    for (int iq = 0; iq != Quads; ++ iq) {
      cout << "Quad: " << iq << "\n"; 
      for (int is = 0; is != Sectors; ++ is) {
      cout << "Segment: " << is << "\n"; 
          //for (int ic = 0; ic != Columns; ++ ic) {
	  //for (int ir = 0; ir != Rows; ++ ir) {
	    for (int ic = 0; ic < 4; ++ ic) {
	    for (int ir = 0; ir < 10; ++ ir) {
	      //cout << "  " << iq << " " << is << " " << ic << " " << ir << " "; 
              cout << m_mask[iq][is][ic][ir] << " "; 
          }
              cout << endl;
        }
      }
    }
}

//----------------
//--------------
// Destructor --
//--------------
CSPadMaskV1::~CSPadMaskV1 ()
{
}

} // namespace ImgAlgos
