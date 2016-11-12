/*
 * streakfinder.cpp
 *
 *  Created on: Feb 5, 2016
 *      Author: vmariani
 */

#include "streakfinder_wrapper.h"
#include "cheetahConversion.h"
#include "pythonWrapperConversions.h"
#include "mask.h"
#include <stdio.h>

streakFinder_constantArguments_t *precomputeStreakFinderConstantArguments(uint_fast8_t filterLength, uint_fast8_t minFilterLength, float filterStep,
        float sigmaFactor, uint_fast8_t streakElongationMinStepsCount, float streakElongationRadiusFactor, uint_fast8_t streakPixelMaskRadius,
        uint_fast8_t numLinesToCheck, detectorCategory_t detectorCategory, int background_region_preset, int background_region_dist_from_edge, long asic_nx,
        long asic_ny, long nasics_x, long nasics_y, float *pixel_map_x, float *pixel_map_y, uint8_t *input_mask, char* background_region_mask)
{
    printf("DEBUG: Starting precomputing StreakFinder constants");
    detectorRawSize_cheetah_t *detector_raw_size_cheetah = new detectorRawSize_cheetah_t;
    detector_raw_size_cheetah->asic_nx = asic_nx;
    detector_raw_size_cheetah->asic_ny = asic_ny;
    detector_raw_size_cheetah->nasics_x = nasics_x;
    detector_raw_size_cheetah->nasics_y = nasics_y;

    detector_raw_size_cheetah->pix_nx = asic_nx * nasics_x;
    detector_raw_size_cheetah->pix_ny = asic_ny * nasics_y;
    detector_raw_size_cheetah->pix_nn = asic_nx * nasics_x * asic_ny * nasics_y;

    streakFinder_accuracyConstants_t *streakFinder_accuracyConstants = new streakFinder_accuracyConstants_t;
    streakFinder_accuracyConstants->filterLength = filterLength;
    streakFinder_accuracyConstants->minFilterLength = minFilterLength;
    streakFinder_accuracyConstants->filterStep = filterStep;
    streakFinder_accuracyConstants->sigmaFactor = sigmaFactor;
    streakFinder_accuracyConstants->streakElongationMinStepsCount = streakElongationMinStepsCount;
    streakFinder_accuracyConstants->streakElongationRadiusFactor = streakElongationRadiusFactor;
    streakFinder_accuracyConstants->streakPixelMaskRadius = streakPixelMaskRadius;

    setStreakDetectorIndices(*streakFinder_accuracyConstants, detectorCategory);

    Eigen::Vector2f* dgm;

    std::vector < std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > > *detector_positions =
            new std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >;

    streakFinder_precomputedConstants_t *streakFinder_precomputedConstants = new streakFinder_precomputedConstants_t;

    printf("DEBUG: Streakfinder detector_category is %i\n", detectorCategory);
    if (detectorCategory == detectorCategory_pnCCD) {
        printf("DEBUG Streakfinder detector_category set to pnccd");
        streakFinder_accuracyConstants->linesToCheck.push_back(1);
        streakFinder_accuracyConstants->linesToCheck.push_back(3);
        numLinesToCheck -= 2;
        for (uint_fast8_t line_idx = 8; line_idx <= numLinesToCheck * 7; line_idx += 7) {
            streakFinder_accuracyConstants->linesToCheck.push_back(line_idx);
        }

        streakFinder_accuracyConstants->backgroundEstimationRegionsInDetector.push_back(ImageRectangle < uint16_t > (Point2D < uint16_t > (100, 400), 10, 10));
        streakFinder_accuracyConstants->backgroundEstimationRegionsInDetector.push_back(ImageRectangle < uint16_t > (Point2D < uint16_t > (250, 250), 10, 10));
        streakFinder_accuracyConstants->backgroundEstimationRegionsInDetector.push_back(ImageRectangle < uint16_t > (Point2D < uint16_t > (450, 200), 10, 10));

        std::vector < std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > > detector_positions_tmp;

        cheetahGetDetectorGeometryMatrix(pixel_map_x, pixel_map_y, *detector_raw_size_cheetah, &dgm);
        computeDetectorPositionsFromDetectorGeometryMatrix(detector_positions_tmp, *detector_raw_size_cheetah, dgm);
        cheetahDeleteDetectorGeometryMatrix(dgm);

        rearrangePnCcdGeometryForStreakFinder(*detector_positions, detector_positions_tmp);

        uint8_t *mask_rearranged = new uint8_t[detector_raw_size_cheetah->pix_nn];
        rearrangePnCcdMaskForStreakFinder(mask_rearranged, input_mask);
        precomputeStreakFinderConstants(*streakFinder_accuracyConstants, *detector_raw_size_cheetah, *detector_positions, mask_rearranged,
                *streakFinder_precomputedConstants);
        delete[] mask_rearranged;
    } else {
        for (uint_fast8_t line_idx = 1; line_idx <= numLinesToCheck; ++line_idx) {
            streakFinder_accuracyConstants->linesToCheck.push_back(line_idx);
        }

        cheetahGetDetectorGeometryMatrix(pixel_map_x, pixel_map_y, *detector_raw_size_cheetah, &dgm);
        computeDetectorPositionsFromDetectorGeometryMatrix(*detector_positions, *detector_raw_size_cheetah, dgm);
        cheetahDeleteDetectorGeometryMatrix(dgm);

        setUserSelection_backgroundEstimationRegionInDetector(*streakFinder_accuracyConstants, *detector_raw_size_cheetah,
                background_region_preset,
                background_region_dist_from_edge,
                background_region_mask);

        precomputeStreakFinderConstants(*streakFinder_accuracyConstants, *detector_raw_size_cheetah, *detector_positions, input_mask,
                *streakFinder_precomputedConstants);
    }
    streakFinder_constantArguments_t *streakFinderConstantArguments = new streakFinder_constantArguments_t();

    detectorCategory_t *detectorCategory_heap = new detectorCategory_t;
    *detectorCategory_heap = detectorCategory;

    streakFinderConstantArguments->accuracyConstants = (void*) streakFinder_accuracyConstants;
    streakFinderConstantArguments->detectorRawSize_cheetah = (void*) detector_raw_size_cheetah;
    streakFinderConstantArguments->detectorPositions = (void*) detector_positions;
    streakFinderConstantArguments->detectorCategory = (void*) detectorCategory_heap;
    streakFinderConstantArguments->streakFinder_precomputedConstant = (void*) streakFinder_precomputedConstants;

    return streakFinderConstantArguments;

}

void freePrecomputedStreakFinderConstantArguments(streakFinder_constantArguments_t *streakfinder_constant_arguments)
{
    delete (streakFinder_accuracyConstants_t*) streakfinder_constant_arguments->accuracyConstants;
    delete (detectorRawSize_cheetah_t*) streakfinder_constant_arguments->detectorRawSize_cheetah;
    delete (detectorCategory_t*) streakfinder_constant_arguments->detectorCategory;
    delete (std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > > *) streakfinder_constant_arguments->detectorPositions;
    freePrecomputedStreakFinderConstants(*(streakFinder_precomputedConstants_t *) streakfinder_constant_arguments->streakFinder_precomputedConstant);
}

void streakfinder(float* data, uint8_t* streak_mask, uint8_t* input_mask, streakFinder_constantArguments_t* streakFinder_constantArguments)
{
    mergeMaskIntoData(data, (const uint8_t *) input_mask, *(const detectorRawSize_cheetah_t*) streakFinder_constantArguments->detectorRawSize_cheetah);

    if (*((detectorCategory_t*) streakFinder_constantArguments->detectorCategory) == detectorCategory_pnCCD) {
        float* data_rearranged = new float[((detectorRawSize_cheetah_t*) streakFinder_constantArguments->detectorRawSize_cheetah)->pix_nn];

        rearrangePnCcdDataForStreakFinder(data_rearranged, data);
        pythonWrapper_streakFinder(data_rearranged, streakFinder_constantArguments);
        reRearrangePnCcdDataForStreakFinder(data, data_rearranged);

        delete[] data_rearranged;
    } else {
        pythonWrapper_streakFinder(data, streakFinder_constantArguments);
    }

    getMaskFromMergedMaskInData(data, streak_mask, *(detectorRawSize_cheetah_t*) streakFinder_constantArguments->detectorRawSize_cheetah);
}

