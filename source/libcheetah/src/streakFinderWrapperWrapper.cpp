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


//#include "cheetah_extensions_yaroslav/streakFinder.h"
//#include "cheetah_extensions_yaroslav/streakfinder_wrapper.h"
//#include "cheetah_extensions_yaroslav/pythonWrapperConversions.h"
//#include "cheetah_extensions_yaroslav/cheetahConversion.h"

#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "cheetahmodules.h"


/*
 *	Initialise streak finder
 */
void initStreakFinder(cGlobal *global) {
	DETECTOR_LOOP {
		if(global->detector[detIndex].useStreakFinder) {
			/*
			 *	Set up streak finder pre-calculations
			 */
			printf("Setting up streak finder (detectorID=%ld)\n",global->detector[detIndex].detectorID);
			
			long     pix_nn = global->detector[detIndex].pix_nn;
			float    *pix_r = global->detector[detIndex].pix_r;
			float    *pix_x = global->detector[detIndex].pix_x;
			float    *pix_y = global->detector[detIndex].pix_y;

			long		asic_nx = global->detector[detIndex].asic_nx;
			long		asic_ny = global->detector[detIndex].asic_ny;
			long		nasics_x = global->detector[detIndex].nasics_x;
			long		nasics_y = global->detector[detIndex].nasics_y;
			
			int				streak_background_region_preset = global->detector[detIndex].streak_background_region_preset;
			int				streak_background_region_dist_from_edge = global->detector[detIndex].streak_background_region_dist_from_edge;
			float			streak_filter_step = global->detector[detIndex].streak_filter_step;
			float			streak_sigma_factor = global->detector[detIndex].streak_sigma_factor;
			float			streak_elongation_radius_factor = global->detector[detIndex].streak_elongation_radius_factor;
			uint_fast8_t	streak_filter_length = global->detector[detIndex].streak_filter_length;
			uint_fast8_t	streak_min_filter_length = global->detector[detIndex].streak_min_filter_length;
			uint_fast8_t	streak_elongation_min_steps_count = global->detector[detIndex].streak_elongation_min_steps_count;
			uint_fast8_t	streak_pixel_mask_radius = global->detector[detIndex].streak_pixel_mask_radius;
			uint_fast8_t	streak_num_lines_to_check = global->detector[detIndex].streak_num_lines_to_check;
//			detectorCategory_t	streak_detector_type = global->detector[detIndex].streak_detector_type;
//			detectorCategory_t	streak_detector_type = detectorCategory_CSPAD;
			
			//char			*streak_background_region_mask = global->detector[detIndex].streak_background_region_mask;

			
			//	Masks for bad regions  (mask=0 to ignore regions)
			char	*mask = (char*) calloc(pix_nn, sizeof(char));
			uint16_t	combined_pixel_options = PIXEL_IS_IN_PEAKMASK|PIXEL_IS_HOT|PIXEL_IS_BAD|PIXEL_IS_OUT_OF_RESOLUTION_LIMITS;
			//for(long i=0;i<pix_nn;i++)
			//	mask[i] = isNoneOfBitOptionsSet(eventData->detector[detIndex].pixelmask[i], combined_pixel_options);

			
//			global->detector[detIndex].streakfinderConstants = precompute_streakfinder_constant_arguments(streak_filter_length, streak_min_filter_length, streak_filter_step, streak_sigma_factor, streak_elongation_min_steps_count, streak_elongation_radius_factor, streak_pixel_mask_radius,streak_num_lines_to_check, streak_detector_type, streak_background_region_preset, streak_background_region_dist_from_edge, asic_nx, asic_ny, nasics_x, nasics_y, mask, streak_background_region_mask);
			
			//	Cleanup memory
			free(mask);
		}
	}
}


/*
 *	Destroy streak finder
 *	The structure is freed by the following function:
 *		void free_precomputed_streak_finder_constant_arguments(streakFinderConstantArguments_t *streakfinder_constant_arguments);
 */
void destroyStreakFinder(cGlobal *global) {
	DETECTOR_LOOP {
		if(global->detector[detIndex].useStreakFinder) {
//			free_precomputed_streak_finder_constant_arguments(global->detector[detIndex].streakfinderConstants);
		}
	}
}


/*
 *	Perform streak finding
 *
 *	void streakfinder(float* data, uint8_t* streak_mask, uint8_t* input_mask, streakFinderConstantArguments_t* streakfinder_constant_arguments);
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
void streakFinder(cEventData *eventData, cGlobal *global) {
	DETECTOR_LOOP {
		if(global->detector[detIndex].useStreakFinder) {

			long    pix_nn = global->detector[detIndex].pix_nn;
			float	*data = eventData->detector[detIndex].data_detPhotCorr;
			uint16_t	*pixelmask = eventData->detector[detIndex].pixelmask;

//			streakFinderConstantArguments_t	*streakfinderConstants = global->detector[detIndex].streakfinderConstants;
			
			//	Masks pre-existing bad regions  (mask=0 to ignore regions)
			char	*input_mask = (char*) calloc(pix_nn, sizeof(char));
			uint16_t	combined_pixel_options = PIXEL_IS_IN_PEAKMASK|PIXEL_IS_HOT|PIXEL_IS_BAD|PIXEL_IS_OUT_OF_RESOLUTION_LIMITS;
			for(long i=0;i<pix_nn;i++)
				input_mask[i] = isNoneOfBitOptionsSet(eventData->detector[detIndex].pixelmask[i], combined_pixel_options);

			//	Mask to hold streak info
			char	*streak_mask = (char*) calloc(pix_nn, sizeof(char));
			
			// Streakfinder
//			streakfinder(data, streak_mask, input_mask,  streakfinderConstants);
			
			
			// Set frame mask
			for(long i=0; i<pix_nn; i++) {
				if ( streak_mask[i])
					pixelmask[i] |= PIXEL_IS_IN_JET;
				else
					pixelmask[i] &= ~PIXEL_IS_IN_JET;
			}
		
			
			
			//	Cleanup memory
			free(input_mask);
			free(streak_mask);
		}
	}
}




