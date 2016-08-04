/*
 * pythonWrapperConversions.h
 *
 *  Created on: 05.02.2016
 *      Author: Yaro
 */

#ifndef PYTHONWRAPPERCONVERSIONS_H_
#define PYTHONWRAPPERCONVERSIONS_H_

#include <vector>
#include <stddef.h>
#include "streakFinder.h"
#include "detectorGeometry.h"

enum detectorCategory_t {
    detectorCategory_CSPAD,
    detectorCategory_pnCCD,
    detectorCategory_UNDEFINED
};

typedef struct {
    void* accuracyConstants;
    void* detectorRawSize_cheetah;
    void* detectorPositions;
    void* detectorCategory;
    void* streakFinder_precomputedConstant;
} streakFinder_constantArguments_t;

void setUserSelection_backgroundEstimationRegionInDetector(streakFinder_accuracyConstants_t& streakFinder_accuracyConstants,
        detectorRawSize_cheetah_t detectorRawSize_cheetah, int presetNumber, int distanceFromDetectorBottom,
        char* backgroundRegionMask_forVisualization = NULL);

void setStreakDetectorIndices(streakFinder_accuracyConstants_t& streakFinder_accuracyConstants, detectorCategory_t detectorCategory);

void setStreakFinderConstantArguments(streakFinder_constantArguments_t* streakFinderConstantArguments,
        const streakFinder_accuracyConstants_t& accuracyConstants,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions,
        const streakFinder_precomputedConstants_t& streakFinder_precomputedConstants);

void pythonWrapper_streakFinder(float* data_linear, streakFinder_constantArguments_t* streakFinderConstantArguments);

#endif /* PYTHONWRAPPERCONVERSIONS_H_ */
