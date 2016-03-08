/*
 * streakFinder.h
 *
 *  Created on: 16.12.2015
 *      Author: Yaro
 */

#ifndef INCLUDE_STREAKFINDER_H_
#define INCLUDE_STREAKFINDER_H_

#include <stdint.h>
#include <vector>
#include "detectorGeometry.h"
#include "Point.h"
#include "ImageRectangle.h"
#include <Eigen/Dense>
#include <Eigen/StdVector>

typedef struct {
    uint_fast8_t filterLength;
    uint_fast8_t minFilterLength;
    float filterStep;

    float sigmaFactor;
    uint_fast8_t streakElongationMinStepsCount;
    float streakElongationRadiusFactor;
    uint_fast8_t streakPixelMaskRadius;
    std::vector< uint_fast8_t > linesToCheck; //slow scan lines to check
    std::vector< Point2D< uint_fast8_t > > streakDetektorsIndices; //Point (x,y) in the way the detector is positioned in the rawImage (top left detector is (0,0), it's right neighbor is (1,0) )
    std::vector< ImageRectangle< uint16_t > > backgroundEstimationRegionsInDetector;
} streakFinder_accuracyConstants_t;

typedef struct {
    std::vector< uint32_t > pixelsToMaskIndices;
    std::vector< uint32_t > numberOfPixelsToMaskForStreakLength;
} streakPixels_t;
typedef struct {
    std::vector< std::vector< std::vector< Eigen::Vector2f, Eigen::aligned_allocator< Eigen::Vector2f > > > > filterDirectionVectors; // [streakDetektorNumber][lineToCheckNumber][posOnLineToCheck]

    int32_t* radialFilterContributors; // [y][x][contributor]

    std::vector< std::vector< std::vector< streakPixels_t > > > streaksPixels; // [streakDetektorNumber][lineToCheckNumber][posOnLineToCheck]

    float detectorPositionsHash; //used to check, whether the precomputed constants are computed with the same detector positions as they are used with (especially important, if they are loaded from a file)

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} streakFinder_precomputedConstants_t;

void precomputeStreakFinderConstants(const streakFinder_accuracyConstants_t& streakFinder_accuracyConstants,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions,
        const uint8_t* mask_linear,
        streakFinder_precomputedConstants_t& streakFinder_precomputedConstants);
void freePrecomputeStreakFinderConstants(streakFinder_precomputedConstants_t& streakFinder_precomputedConstants);

void streakFinder(float* data_linear, const streakFinder_accuracyConstants_t& accuracyConstants, const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions,
        const streakFinder_precomputedConstants_t& streakFinder_precomputedConstants);

#endif /* INCLUDE_STREAKFINDER_H_ */
