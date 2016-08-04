/*
 * pnCcdWorkarounds.h
 *
 *  Created on: 20.07.2016
 *      Author: Yaro
 */

#ifndef PNCCDWORKAROUNDS_H_
#define PNCCDWORKAROUNDS_H_

#include "streakFinder.h"
#include "detectorGeometry.h"
#include <vector>
#include <Eigen/StdVector>

void rearrangePnCcdDataForStreakFinder(float* data_rearranged_linear, const float* data_linear);
void reRearrangePnCcdDataForStreakFinder(float* data_linear, const float* data_rearranged_linear);
void rearrangePnCcdMaskForStreakFinder(uint8_t* mask_rearranged_linear, const uint8_t* mask_linear);
void rearrangePnCcdGeometryForStreakFinder(
        std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions_rearranged,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions);

#endif /* PNCCDWORKAROUNDS_H_ */
