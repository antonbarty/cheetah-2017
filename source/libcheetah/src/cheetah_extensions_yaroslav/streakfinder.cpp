/*
 * streakfinder.cpp
 *
 *  Created on: Feb 5, 2016
 *      Author: vmariani
 */

#include "streakfinder.h"
#include "cheetahConversion.h"
#include "pythonWrapperConversions.h"
#include "mask.h"


streakFinderConstantArguments_t *precompute_streakfinder_constant_arguments(uint_fast8_t filter_length, uint_fast8_t min_filter_length, float filter_step,
                                        float sigma_factor, uint_fast8_t streak_elongation_min_steps_count,
                                        float streak_elongation_radius_factor,
                                        uint_fast8_t streak_pixel_mask_radius, uint_fast8_t num_lines_to_check,
                                        detectorCathegory_t detector_type, int background_region_preset,
                                        int background_region_dist_from_edge,
                                        long asic_nx, long asic_ny, long nasics_x, long nasics_y,
                                        float *pixel_map_x, float *pixel_map_y, uint8_t *mask, char* background_region_mask)
 {
    detectorRawSize_cheetah_t *detector_raw_size_cheetah = new detectorRawSize_cheetah_t;
    detector_raw_size_cheetah->asic_nx = asic_nx;
    detector_raw_size_cheetah->asic_ny = asic_ny;
    detector_raw_size_cheetah->nasics_x = nasics_x;
    detector_raw_size_cheetah->nasics_y = nasics_y;

    detector_raw_size_cheetah->pix_nx = asic_nx * nasics_x;
    detector_raw_size_cheetah->pix_ny = asic_ny * nasics_y;
    detector_raw_size_cheetah->pix_nn = asic_nx * nasics_x * asic_ny * nasics_y;

    streakFinder_accuracyConstants_t *streakfinder_accuracy_constants = new streakFinder_accuracyConstants_t;
    streakfinder_accuracy_constants->filterLength = filter_length;
    streakfinder_accuracy_constants->minFilterLength = min_filter_length;
    streakfinder_accuracy_constants->filterStep = filter_step;
    streakfinder_accuracy_constants->sigmaFactor = sigma_factor;
    streakfinder_accuracy_constants->streakElongationMinStepsCount = streak_elongation_min_steps_count;
    streakfinder_accuracy_constants->streakElongationRadiusFactor = streak_elongation_radius_factor;
    streakfinder_accuracy_constants->streakPixelMaskRadius = streak_pixel_mask_radius;
    for ( uint_fast8_t line_idx=0; line_idx<num_lines_to_check; ++line_idx) {
        streakfinder_accuracy_constants->linesToCheck.push_back(line_idx);
    }

    setStreakDetectorIndices(*streakfinder_accuracy_constants, detector_type);
    setUserSelection_backgroundEstimationRegionInDetector(*streakfinder_accuracy_constants, *detector_raw_size_cheetah,
                                                          background_region_preset,
                                                          background_region_dist_from_edge,
                                                          background_region_mask);
    Eigen::Vector2f* dgm;

    std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > > *detector_positions =
            new std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >;

    cheetahGetDetectorGeometryMatrix(pixel_map_x, pixel_map_y, *detector_raw_size_cheetah, &dgm);
    computeDetectorPositionsFromDetectorGeometryMatrix(*detector_raw_size_cheetah, dgm,
                                                       *detector_positions);
    cheetahDeleteDetectorGeometryMatrix(dgm);

    streakFinder_precomputedConstants_t *streakfinder_precomputed_constants = new streakFinder_precomputedConstants_t;

    precomputeStreakFinderConstants(*streakfinder_accuracy_constants, *detector_raw_size_cheetah, *detector_positions, mask,
                                    *streakfinder_precomputed_constants);

    streakFinderConstantArguments_t *streak_finder_constant_arguments = new streakFinderConstantArguments_t();

    streak_finder_constant_arguments->accuracyConstants = (void*)streakfinder_accuracy_constants;
    streak_finder_constant_arguments->detectorRawSize_cheetah = (void*)detector_raw_size_cheetah;
    streak_finder_constant_arguments->detectorPositions = (void*)detector_positions;
    streak_finder_constant_arguments->streakFinder_precomputedConstant = (void*)streakfinder_precomputed_constants;

    return streak_finder_constant_arguments;

}

void free_precomputed_streak_finder_constant_arguments(streakFinderConstantArguments_t *streakfinder_constant_arguments)
{
    delete((streakFinder_accuracyConstants_t*)streakfinder_constant_arguments->accuracyConstants);
    delete((detectorRawSize_cheetah_t*)streakfinder_constant_arguments->detectorRawSize_cheetah);
    delete((std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > > *)streakfinder_constant_arguments->detectorPositions);
    freePrecomputeStreakFinderConstants(*(streakFinder_precomputedConstants_t *)streakfinder_constant_arguments->streakFinder_precomputedConstant);
}

void streakfinder(float* data, uint8_t* streak_mask, streakFinderConstantArguments_t* streakfinder_constant_arguments)
{
    pythonWrapper_streakFinder(data, streakfinder_constant_arguments);
    getMaskFromMergedMaskInData(data, streak_mask, *(detectorRawSize_cheetah_t*)streakfinder_constant_arguments->detectorRawSize_cheetah);
}


