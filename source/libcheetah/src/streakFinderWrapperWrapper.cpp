/*
 * streakfinder.cpp
 *
 *  Created on: Feb 5, 2016
 *      Author: vmariani
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>

#include <mmintrin.h>
#include <emmintrin.h>

#include "cheetah_extensions_yaroslav/streakFinder.h"
#include "cheetah_extensions_yaroslav/streakfinder_wrapper.h"
#include "cheetah_extensions_yaroslav/pythonWrapperConversions.h"
#include "cheetah_extensions_yaroslav/cheetahConversion.h"

#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "cheetahmodules.h"

#ifdef __CDT_PARSER__
#undef DETECTOR_LOOP
#define DETECTOR_LOOP for(;;)
#endif

/*
 *	Initialise streak finder
 */
void initStreakFinder(cGlobal *global)
{
    DETECTOR_LOOP
    {
        if (global->detector[detIndex].useStreakFinder) {
            /*
             *	Set up streak finder pre-calculations
             */
            printf("Setting up streak finder (detectorID=%ld)\n", global->detector[detIndex].detectorID);

            long pix_nn = global->detector[detIndex].pix_nn;
            float *pix_r = global->detector[detIndex].pix_r;
            float *pix_x = global->detector[detIndex].pix_x;
            float *pix_y = global->detector[detIndex].pix_y;

            long asic_nx = global->detector[detIndex].asic_nx;
            long asic_ny = global->detector[detIndex].asic_ny;
            long nasics_x = global->detector[detIndex].nasics_x;
            long nasics_y = global->detector[detIndex].nasics_y;

            int streak_background_region_preset = global->detector[detIndex].streak_background_region_preset;
            int streak_background_region_dist_from_edge = global->detector[detIndex].streak_background_region_dist_from_edge;
            float streak_filter_step = global->detector[detIndex].streak_filter_step;
            float streak_sigma_factor = global->detector[detIndex].streak_sigma_factor;
            float streak_elongation_radius_factor = global->detector[detIndex].streak_elongation_radius_factor;
            uint_fast8_t streak_filter_length = global->detector[detIndex].streak_filter_length;
            uint_fast8_t streak_min_filter_length = global->detector[detIndex].streak_min_filter_length;
            uint_fast8_t streak_elongation_min_steps_count = global->detector[detIndex].streak_elongation_min_steps_count;
            uint_fast8_t streak_pixel_mask_radius = global->detector[detIndex].streak_pixel_mask_radius;
            uint_fast8_t streak_num_lines_to_check = global->detector[detIndex].streak_num_lines_to_check;

            detectorCategory_t streak_detector_type = detectorCategory_UNDEFINED;
            if (strcmp(global->detector[detIndex].detectorType, "cspad") == 0) {
                streak_detector_type = detectorCategory_CSPAD;
            } else if (strcmp(global->detector[detIndex].detectorType, "pnccd") == 0) {
                streak_detector_type = detectorCategory_pnCCD;
            }

            char *streak_background_region_mask = NULL;

            //	Masks for bad regions  (mask=0 to ignore regions)
            //	All these mask are supposed to use this convention: 0 means passthrough, !=0 means that the pixel is masked
            uint8_t *mask = (uint8_t*) calloc(pix_nn, sizeof(uint8_t));
            uint16_t combined_pixel_options = PIXEL_IS_HOT | PIXEL_IS_BAD;
            for (long i = 0; i < pix_nn; i++) {
                mask[i] = isAnyOfBitOptionsSet(global->detector[detIndex].pixelmask_shared[i], combined_pixel_options);
            }

            global->detector[detIndex].streakfinderConstants = precomputeStreakFinderConstantArguments(streak_filter_length, streak_min_filter_length,
                    streak_filter_step, streak_sigma_factor, streak_elongation_min_steps_count, streak_elongation_radius_factor, streak_pixel_mask_radius,
                    streak_num_lines_to_check, streak_detector_type, streak_background_region_preset, streak_background_region_dist_from_edge, asic_nx, asic_ny,
                    nasics_x, nasics_y, pix_x, pix_y, mask, streak_background_region_mask);

            //	Cleanup memory
            free(mask);
        }
    }
}

/*
 streakFinder_constantArguments_t *precomputeStreakFinderConstantArguments(
 uint_fast8_t	filter_length,
 uint_fast8_t	min_filter_length,
 float			filter_step,
 float			sigma_factor,
 uint_fast8_t	streak_elongation_min_steps_count,
 float			streak_elongation_radius_factor,
 uint_fast8_t	streak_pixel_mask_radius,
 uint_fast8_t	num_lines_to_check,
 detectorCategory_t detector_type,
 int				background_region_preset,
 int				background_region_dist_from_edge,
 long			asic_nx,
 long			asic_ny,
 long			nasics_x,
 long			nasics_y,
 float			*pixel_map_x,
 float			*pixel_map_y,
 uint8_t			*input_mask,
 char*			background_region_mask)
 */

/*
 *	Destroy streak finder
 *	The structure is freed by the following function:
 *		void freePrecomputedStreakFinderConstantArguments(streakFinder_constantArguments_t *streakfinder_constant_arguments);
 */
void destroyStreakFinder(cGlobal *global)
{
    DETECTOR_LOOP
    {
        if (global->detector[detIndex].useStreakFinder) {
            freePrecomputedStreakFinderConstantArguments(global->detector[detIndex].streakfinderConstants);
        }
    }
}

/*
 *	Perform streak finding
 *
 *	void streakfinder(float* data, uint8_t* streak_mask, uint8_t* input_mask, streakFinder_constantArguments_t* streakfinder_constant_arguments);
 *
 *	After the function returns, the "streak_mask" contains the streak mask.
 *	Some mask nomenclature:
 *		input_mask: this is a mask to be applied to the data before the streak finding takes place
 *		streak_mask: this is the mask that contains the streak mask after the function returns
 *		background_region_mask: this is a mask that contains, after the function retuns,  
 *			the regions used by the algorithm to estimate the background level. 
 *			It is used by the parameter tweaker. Just pass a NULL pointer if you don't use it
 *
 *	All these mask are supposed to use this convention: 0 means passthrough, !=0 means that the pixel is masked
 */
void streakFinder(cEventData *eventData, cGlobal *global)
{
    DETECTOR_LOOP
    {
        if (global->detector[detIndex].useStreakFinder) {

            long pix_nn = global->detector[detIndex].pix_nn;
            float *data = eventData->detector[detIndex].data_detPhotCorr;
            uint16_t *pixelmask = eventData->detector[detIndex].pixelmask;

            streakFinder_constantArguments_t *streakfinderConstants = global->detector[detIndex].streakfinderConstants;

            /*
             *	The data gets corrupted by the streak finder.
             *	So we must make a copy and pass it to the algorithm.
             */
            float *tempdata = (float*) malloc(pix_nn * sizeof(float));
            memcpy(tempdata, data, pix_nn * sizeof(float));

//	Masks pre-existing bad regions  (mask=0 to ignore regions)
//	The streak finder expects this mask to be
//		mask = 1(or other): ignore,
//		mask = 0: look at the data in the pixels
            uint8_t *input_mask = (uint8_t*) malloc(pix_nn * sizeof(uint8_t));
            uint8_t *streak_mask = (uint8_t*) malloc(pix_nn * sizeof(uint8_t));
            uint16_t combined_pixel_options = PIXEL_IS_HOT | PIXEL_IS_BAD | PIXEL_IS_NOISY;
            for (long i = 0; i < pix_nn; i++) {
                input_mask[i] = isAnyOfBitOptionsSet(pixelmask[i], combined_pixel_options);
            }

//	Streakfinder
//	The data gets corrupted by the streak finder, so we must pass a copy to the algorithm.
            streakfinder(tempdata, streak_mask, input_mask, streakfinderConstants);

// Set frame mask
// In the output mask, 1 means there is a streak, 0 means that there is data
// So !=0 pixel is in jet, ==0 pixel is not in jet
            for (long i = 0; i < pix_nn; i++) {
                if (streak_mask[i] != 0) {
                    eventData->detector[detIndex].pixelmask[i] |= PIXEL_IS_IN_JET;
                } else
                    eventData->detector[detIndex].pixelmask[i] &= ~PIXEL_IS_IN_JET;
            }

//	Cleanup memory
            free(tempdata);
            free(input_mask);
            free(streak_mask);
        }
    }
}

