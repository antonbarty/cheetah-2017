/*
 * radialBackgroundSubtraction.h
 *
 *  Created on: 26.06.2016
 *      Author: Yaro
 */

#ifndef RADIALBACKGROUNDSUBTRACTION_H_
#define RADIALBACKGROUNDSUBTRACTION_H_

#include <Point2D.h>
#include <stdint.h>
#include <vector>
#include "detectorGeometry.h"
#include <Eigen/Dense>
#include <Eigen/StdVector>

typedef struct {
    uint_fast32_t minValuesPerBin;
    uint_fast32_t minBinWidth;

    uint_fast32_t maxConsideredValuesPerBin;    //0 for infinite

    std::vector< Point2D< uint_fast8_t > > detektorsToConsiderIndices; //Point (x,y) in the way the detector is positioned in the rawImage (top left detector is (0,0), it's right neighbor is (1,0) )
    std::vector< Point2D< uint_fast8_t > > detektorsToCorrectIndices; //must be a subset of detektorsToConsiderIndices

    float rank; //between 0 and 1
} radialRankFilter_accuracyConstants_t;

typedef struct {
    std::vector< uint32_t > sparseLinearDataToConsiderIndices;
    std::vector< uint16_t > sparseBinIndices;

    uint16_t binCount;
    std::vector< uint32_t > dataCountPerBin;

    std::vector< uint16_t > intraBinIndices;

    std::vector< float > binRadii;

    std::vector< float > intraBinInterpolationConstant;
} radialRankFilter_precomputedConstants_t;

void precomputeRadialRankFilterConstants(radialRankFilter_precomputedConstants_t& precomputedConstants, const uint8_t* mask_linear,
        const float* detectorGeometryRadiusMatrix_linear,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah, const radialRankFilter_accuracyConstants_t& accuracyConstants,
        const Eigen::Vector2f* detectorGeometryMatrix_linear);

void applyRadialRankFilter(float* data_linear, const radialRankFilter_accuracyConstants_t& accuracyConstants,
        const radialRankFilter_precomputedConstants_t& precomputedConstants, const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions);

#endif /* RADIALBACKGROUNDSUBTRACTION_H_ */
