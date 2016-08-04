/*
 * streakfinder.h
 *
 *  Created on: Feb 1, 2016
 *      Author: vmariani
 */

#ifndef INCLUDE_STREAKFINDER_WRAPPER_H_
#define INCLUDE_STREAKFINDER_WRAPPER_H_

#include "streakFinder.h"
#include "pythonWrapperConversions.h"
#include "pnCcdWorkarounds.h"

streakFinder_constantArguments_t *precomputeStreakFinderConstantArguments(uint_fast8_t filterLength, uint_fast8_t minFilterLength, float filterStep,
        float sigmaFactor, uint_fast8_t streakElongationMinStepsCount, float streakElongationRadiusFactor, uint_fast8_t streakPixelMaskRadius,
        uint_fast8_t numLinesToCheck, detectorCategory_t detectorCategory, int background_region_preset, int background_region_dist_from_edge, long asic_nx,
        long asic_ny, long nasics_x, long nasics_y, float *pixel_map_x, float *pixel_map_y, uint8_t *input_mask, char* background_region_mask);

void freePrecomputedStreakFinderConstantArguments(streakFinder_constantArguments_t *streakfinder_constant_arguments);

void streakfinder(float* data, uint8_t* streak_mask, uint8_t* input_mask, streakFinder_constantArguments_t* streakfinder_constant_arguments);

#endif /* INCLUDE_STREAKFINDER_WRAPPER_H_ */

