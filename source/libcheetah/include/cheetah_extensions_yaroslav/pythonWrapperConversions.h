/*
 * pythonWrapperConversions.h
 *
 *  Created on: 05.02.2016
 *      Author: Yaro
 */

#ifndef PYTHONWRAPPERCONVERSIONS_H_
#define PYTHONWRAPPERCONVERSIONS_H_

#include <vector>
#include "streakFinder.h"
#include "detectorGeometry.h"

enum detectorCathegory_t {
    detectorCathegory_CSPAD
};

typedef struct {
    void* accuracyConstants;
    void* detectorRawSize_cheetah;
    void* detectorPositions;
    void* streakFinder_precomputedConstant;
} streakFinderConstantArguments_t;

void setUserSelection_backgroundEstimationRegionInDetector(streakFinder_accuracyConstants_t streakFinder_accuracyConstants,
        detectorRawSize_cheetah_t detectorRawSize_cheetah, int presetNumber, int distanceFromDetectorBottom, char* backgroundRegionMask_forVisualization);

void setStreakDetectorIndices(streakFinder_accuracyConstants_t streakFinder_accuracyConstants, detectorCathegory_t detectorCathegory);

void setStreakFinderConstantArguments(streakFinderConstantArguments_t* streakFinderConstantArguments, const streakFinder_accuracyConstants_t& accuracyConstants,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions,
        const streakFinder_precomputedConstants_t& streakFinder_precomputedConstants);

void pythonWrapper_streakFinder(float* data_linear, streakFinderConstantArguments_t* streakFinderConstantArguments);

#endif /* PYTHONWRAPPERCONVERSIONS_H_ */
