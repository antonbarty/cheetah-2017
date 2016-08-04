/*
 * pythonWrapperConversions.cpp
 *
 *  Created on: 05.02.2016
 *      Author: Yaro
 */

#include "pythonWrapperConversions.h"
#include <string.h>
#include "ImageRectangle.h"
#include <stdint.h>

#include <boost/foreach.hpp>
#ifdef __CDT_PARSER__
#undef BOOST_FOREACH
#define BOOST_FOREACH(a, b) for(a; ; )
#endif

using namespace std;

void setUserSelection_backgroundEstimationRegionInDetector(streakFinder_accuracyConstants_t& streakFinder_accuracyConstants,
        detectorRawSize_cheetah_t detectorRawSize_cheetah, int presetNumber, int distanceFromDetectorBottom, char* backgroundRegionMask_forVisualization)
{

    vector < ImageRectangle< uint16_t > > &backgroundEstimationRegionsInDetector = streakFinder_accuracyConstants.backgroundEstimationRegionsInDetector;

    int height, width;
    switch (presetNumber) {
        case 0:
            height = 6;
            width = 6;
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (44, (detectorRawSize_cheetah.asic_ny - 1) - 45 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (124, (detectorRawSize_cheetah.asic_ny - 1) - 0 - height - distanceFromDetectorBottom), width, height));
            break;
        case 1:
            height = 10;
            width = 10;
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (44, (detectorRawSize_cheetah.asic_ny - 1) - 45 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (124, (detectorRawSize_cheetah.asic_ny - 1) - 0 - height - distanceFromDetectorBottom), width, height));
            break;
        case 2:
            height = 6;
            width = 6;
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (44, (detectorRawSize_cheetah.asic_ny - 1) - 35 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (124, (detectorRawSize_cheetah.asic_ny - 1) - 0 - height - distanceFromDetectorBottom), width, height));
            break;
        case 3:
            height = 10;
            width = 10;
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (44, (detectorRawSize_cheetah.asic_ny - 1) - 35 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (124, (detectorRawSize_cheetah.asic_ny - 1) - 0 - height - distanceFromDetectorBottom), width, height));
            break;
        case 4:
            height = 6;
            width = 6;
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (44, (detectorRawSize_cheetah.asic_ny - 1) - 25 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (124, (detectorRawSize_cheetah.asic_ny - 1) - 0 - height - distanceFromDetectorBottom), width, height));
            break;
        case 5:
            height = 10;
            width = 10;
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (44, (detectorRawSize_cheetah.asic_ny - 1) - 25 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (124, (detectorRawSize_cheetah.asic_ny - 1) - 0 - height - distanceFromDetectorBottom), width, height));
            break;

        case 6:
            height = 6;
            width = 6;
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (22, (detectorRawSize_cheetah.asic_ny - 1) - 45 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (89, (detectorRawSize_cheetah.asic_ny - 1) - 40 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (144, (detectorRawSize_cheetah.asic_ny - 1) - 0 - height - distanceFromDetectorBottom), width, height));
            break;
        case 7:
            height = 10;
            width = 10;
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (22, (detectorRawSize_cheetah.asic_ny - 1) - 45 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (89, (detectorRawSize_cheetah.asic_ny - 1) - 40 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (144, (detectorRawSize_cheetah.asic_ny - 1) - 0 - height - distanceFromDetectorBottom), width, height));
            break;
        case 8:
            height = 6;
            width = 6;
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (22, (detectorRawSize_cheetah.asic_ny - 1) - 35 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (89, (detectorRawSize_cheetah.asic_ny - 1) - 30 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (144, (detectorRawSize_cheetah.asic_ny - 1) - 0 - height - distanceFromDetectorBottom), width, height));
            break;
        case 9:
            height = 10;
            width = 10;
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (22, (detectorRawSize_cheetah.asic_ny - 1) - 35 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (89, (detectorRawSize_cheetah.asic_ny - 1) - 30 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (144, (detectorRawSize_cheetah.asic_ny - 1) - 0 - height - distanceFromDetectorBottom), width, height));
            break;
        case 10:
            height = 6;
            width = 6;
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (22, (detectorRawSize_cheetah.asic_ny - 1) - 25 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (89, (detectorRawSize_cheetah.asic_ny - 1) - 20 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (144, (detectorRawSize_cheetah.asic_ny - 1) - 0 - height - distanceFromDetectorBottom), width, height));
            break;
        case 11:
            height = 10;
            width = 10;
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (22, (detectorRawSize_cheetah.asic_ny - 1) - 25 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (89, (detectorRawSize_cheetah.asic_ny - 1) - 20 - height - distanceFromDetectorBottom), width, height));
            backgroundEstimationRegionsInDetector.push_back(
                    ImageRectangle < uint16_t >
                            (Point2D < uint16_t > (144, (detectorRawSize_cheetah.asic_ny - 1) - 0 - height - distanceFromDetectorBottom), width, height));
            break;
        default:
            break;
    }

    if (backgroundRegionMask_forVisualization != NULL) {
        char (*backgroundRegionMask)[detectorRawSize_cheetah.pix_nx] = (char (*)[detectorRawSize_cheetah.pix_nx]) backgroundRegionMask_forVisualization;

        memset(backgroundRegionMask, 0, detectorRawSize_cheetah.pix_nx);

        for (int x_offsetFactor = 0; x_offsetFactor < 7; x_offsetFactor = x_offsetFactor + 2) {
            typedef ImageRectangle< uint16_t > backgroundEstimationRegionInDetector_t;
            BOOST_FOREACH (const backgroundEstimationRegionInDetector_t & backgroundEstimationRegionInDetector , backgroundEstimationRegionsInDetector)
            {
                for (int y = backgroundEstimationRegionInDetector.getUpperLeftCorner().getY();
                        y <= backgroundEstimationRegionInDetector.getLowerRightCorner().getY(); ++y) {
                    for (int x = backgroundEstimationRegionInDetector.getUpperLeftCorner().getX();
                            x <= backgroundEstimationRegionInDetector.getLowerRightCorner().getX(); ++x) {
                        backgroundRegionMask[y + detectorRawSize_cheetah.asic_ny][x + x_offsetFactor * detectorRawSize_cheetah.asic_nx] = 1;
                    }
                }
            }
        }
    }
}

void setStreakDetectorIndices(streakFinder_accuracyConstants_t& streakFinder_accuracyConstants, detectorCategory_t detectorCategory)
{
    switch (detectorCategory) {
        case detectorCategory_CSPAD:
            streakFinder_accuracyConstants.streakDetektorsIndices.push_back(Point2D < uint_fast8_t > (0, 1));
            streakFinder_accuracyConstants.streakDetektorsIndices.push_back(Point2D < uint_fast8_t > (2, 1));
            streakFinder_accuracyConstants.streakDetektorsIndices.push_back(Point2D < uint_fast8_t > (4, 1));
            streakFinder_accuracyConstants.streakDetektorsIndices.push_back(Point2D < uint_fast8_t > (6, 1));
            break;
        case detectorCategory_pnCCD:
            streakFinder_accuracyConstants.streakDetektorsIndices.push_back(Point2D < uint_fast8_t > (0, 0));
            streakFinder_accuracyConstants.streakDetektorsIndices.push_back(Point2D < uint_fast8_t > (0, 1));
            streakFinder_accuracyConstants.streakDetektorsIndices.push_back(Point2D < uint_fast8_t > (1, 0));
            streakFinder_accuracyConstants.streakDetektorsIndices.push_back(Point2D < uint_fast8_t > (1, 1));
            break;
    }
}

void setStreakFinderConstantArguments(streakFinder_constantArguments_t* streakFinderConstantArguments, const streakFinder_accuracyConstants_t& accuracyConstants,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions,
        const streakFinder_precomputedConstants_t& streakFinder_precomputedConstants)
{
    streakFinderConstantArguments->accuracyConstants = (void*) &accuracyConstants;
    streakFinderConstantArguments->detectorRawSize_cheetah = (void*) &detectorRawSize_cheetah;
    streakFinderConstantArguments->detectorPositions = (void*) &detectorPositions;
    streakFinderConstantArguments->streakFinder_precomputedConstant = (void*) &streakFinder_precomputedConstants;
}

void pythonWrapper_streakFinder(float* data_linear, streakFinder_constantArguments_t* streakFinderConstantArguments)
{
    const streakFinder_accuracyConstants_t* accuracyConstants =
            (const streakFinder_accuracyConstants_t*) streakFinderConstantArguments->accuracyConstants;
    const detectorRawSize_cheetah_t* detectorRawSize_cheetah =
            (const detectorRawSize_cheetah_t*) streakFinderConstantArguments->detectorRawSize_cheetah;
    const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >* detectorPositions =
            (const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >*) streakFinderConstantArguments->detectorPositions;
    const streakFinder_precomputedConstants_t* streakFinder_precomputedConstants =
            (const streakFinder_precomputedConstants_t*) streakFinderConstantArguments->streakFinder_precomputedConstant;

    streakFinder(data_linear, *accuracyConstants, *detectorRawSize_cheetah, *detectorPositions, *streakFinder_precomputedConstants);
}
