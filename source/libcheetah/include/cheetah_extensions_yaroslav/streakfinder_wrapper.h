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


streakFinderConstantArguments_t *precompute_streakfinder_constant_arguments(uint_fast8_t filter_length, uint_fast8_t min_filter_length, float filter_step,
                                        float sigma_factor, uint_fast8_t streak_elongation_min_steps_count,
                                        float streak_elongation_radius_factor,
                                        uint_fast8_t streak_pixel_mask_radius, uint_fast8_t num_lines_to_check,
                                        detectorCathegory_t detector_type, int background_region_preset,
                                        int background_region_dist_from_edge,
                                        long asic_nx, long asic_ny, long nasics_x, long nasics_y,
                                        float *pixel_map_x, float *pixel_map_y, uint8_t *mask, char* background_region_mask);

void free_precomputed_streak_finder_constant_arguments(streakFinderConstantArguments_t *streakfinder_constant_arguments);

void streakfinder(float* data, uint8_t* streak_mask, streakFinderConstantArguments_t* streakfinder_constant_arguments);

#endif /* INCLUDE_STREAKFINDER_WRAPPER_H_ */

