#ifndef PDS_CODEC_CompressorOMP_H
#define PDS_CODEC_CompressorOMP_H

//--------------------------------------------------------------------------
// File and Version Information:
//   $Id: CompressorOMP.hh,v 1.1 2012/04/04 01:18:00 tomytsai Exp $
//
// Description:
//   Class CompressorOMP. An openmp wrapper allowing multithreaded compression
//   for API conforming algorithms.
//
// Author:
//   Chang-Ming (Tomy) Tsai, SLAC National Accelerator Laboratory
//--------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------

#include <stdio.h>
#include <omp.h>
#include <vector>

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//              ---------------------
//              -- Class Interface --
//              ---------------------

namespace Pds 
{

namespace CsPad 
{

template< class COMPRESSOR >
class CompressorOMP 
{

public:

  enum 
  {
      Success = 0,
      ErrInAlgorithm         // failed to process at least least one of the images in a batch
  };

  CompressorOMP(size_t iNumImageInit, size_t iNumThreads=1);

  virtual ~CompressorOMP();

  int compress(const void** image, const typename COMPRESSOR::ImageParams* params,
               void** outData, size_t* outDataSize,
               int* stat, int  iNumImages);

  int decompress(const void** outData, const size_t* outDataSize,
                 void** image, typename COMPRESSOR::ImageParams* params,
                 int* stat, int  iNumImages);

  static const char* err2str(int code);

private:
  int                       m_iNumThreads;
  std::vector<COMPRESSOR*>  m_lCompressors;
}; // class CompressorOMP

} //namespace CsPad
} //namespace Pds

template< class COMPRESSOR >
Pds::CsPad::CompressorOMP<COMPRESSOR>::CompressorOMP(size_t iNumImageInit, size_t iNumThreads) :
    m_iNumThreads(iNumThreads)
{    
    m_lCompressors.resize(iNumImageInit);
    for (int iCompressor = 0; iCompressor < (int) m_lCompressors.size(); ++iCompressor)
      m_lCompressors[iCompressor] = new COMPRESSOR();
}

template< class COMPRESSOR >
Pds::CsPad::CompressorOMP<COMPRESSOR>::~CompressorOMP()
{
    for (int iCompressor = 0; iCompressor < (int) m_lCompressors.size(); ++iCompressor)
      delete m_lCompressors[iCompressor];
    m_lCompressors.clear();
}

template< class COMPRESSOR >
int
Pds::CsPad::CompressorOMP<COMPRESSOR>::compress(
    const void** image, const typename COMPRESSOR::ImageParams* params,
    void** outData, size_t* outDataSize,
    int* stat, int iNumImages)
{
  if (iNumImages > (int) m_lCompressors.size())
  {
    int iOrgSize = m_lCompressors.size();
    m_lCompressors.resize(iNumImages);
    for (int iCompressor = iOrgSize; iCompressor < (int) m_lCompressors.size(); ++iCompressor)
      m_lCompressors[iCompressor] = new COMPRESSOR();
  }
  
  #pragma omp parallel for num_threads(m_iNumThreads) schedule(dynamic,1) 
  for (int iImage = 0; iImage < iNumImages; ++iImage)
  {
    stat[iImage] = m_lCompressors[iImage]->compress(
                    image       [iImage],
                    params      [iImage],
                    outData     [iImage],
                    outDataSize [iImage] ); 
  }  

  for (int iImage = 0; iImage < iNumImages; ++iImage)
  {
    if ( stat[iImage] != Success )
      return ErrInAlgorithm;  // at least one of the images hasn't been successfully processed.                                  // see details in the returned status array.
  }
                                  
  return Success;
}

template< class COMPRESSOR >
int
Pds::CsPad::CompressorOMP<COMPRESSOR>::decompress(
    const void** outData, const size_t* outDataSize,
    void** image, typename COMPRESSOR::ImageParams* params,
    int* stat,
    int iNumImages)
{
  if (iNumImages > (int) m_lCompressors.size())
  {
    int iOrgSize = m_lCompressors.size();
    m_lCompressors.resize(iNumImages);
    for (int iCompressor = iOrgSize; iCompressor < (int) m_lCompressors.size(); ++iCompressor)
      m_lCompressors[iCompressor] = new COMPRESSOR();
  }
  
  #pragma omp parallel for num_threads(m_iNumThreads) schedule(dynamic,1) 
  for (int iImage = 0; iImage < iNumImages; ++iImage)
  {
    stat[iImage] = m_lCompressors[iImage]->decompress(
                    outData     [iImage],
                    outDataSize [iImage],
                    image       [iImage],
                    params      [iImage] );
  }
  
  for (int iImage = 0; iImage < iNumImages; ++iImage)
  {
    if ( stat[iImage] != Success )
      return ErrInAlgorithm;  // at least one of the images hasn't been successfully processed.                                  // see details in the returned status array.
  }
  
  return Success;
}

template< class COMPRESSOR >
const char*
Pds::CsPad::CompressorOMP<COMPRESSOR>::err2str(int code)
{
    switch( code ) {
    case Success:              return "Success";
    case ErrInAlgorithm:       return "Failed to process at least least one of the images in a batch";
    }
}

#endif  // PDS_CODEC_CompressorOMP_H
