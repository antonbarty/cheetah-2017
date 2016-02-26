/*
 * streakFinder.cpp
 *
 *  Created on: 16.12.2015
 *      Author: Yaro
 */

#include "streakFinder.h"
#include <assert.h>
#include <algorithm>
#include <cmath>
#include <boost/math/special_functions/round.hpp>
#include <boost/algorithm/cxx11/iota.hpp>
#include <boost/phoenix/phoenix.hpp>

#include <boost/foreach.hpp>
#ifdef __CDT_PARSER__
#undef BOOST_FOREACH
#define BOOST_FOREACH(a, b) for(a; ; )
#endif

using namespace std;
using namespace Eigen;

typedef struct {
    uint32_t* pixelsToMaskIndices;
    uint32_t numberOfPixelsToMask;
} streakPixelsShort_t;

static inline void precomputeFilterDirectionVectors(const streakFinder_accuracyConstants_t& streakFinder_accuracyConstants,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const vector< vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions,
        streakFinder_precomputedConstants_t& streakFinder_precomputedConstants);
static inline void precomputeRadialFilterContributors(const streakFinder_accuracyConstants_t& streakFinder_accuracyConstants,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const vector< vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions, const uint8_t* mask_linear,
        streakFinder_precomputedConstants_t& streakFinder_precomputedConstants);
static inline void precomputeStreakPixels(const streakFinder_accuracyConstants_t& streakFinder_accuracyConstants,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const vector< vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions, const uint8_t* mask_linear,
        streakFinder_precomputedConstants_t& streakFinder_precomputedConstants);
static inline void getAllPixelCoordinatesInRadius(uint16_t x_middle, uint16_t y_middle, uint8_t radius, vector< Point< int16_t > >& pixelCoordinatesInRadius);
static inline void getValidPixelCoordinates(const vector< Point< int16_t > >& pixelCoordinates, const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const detectorPosition_t& detectorPosition, const uint8_t* mask_linear, vector< uint32_t >& linearValidPixelCoordinates);
static inline void getLinearValidCoordinatesInRadius(uint16_t x_middle, uint16_t y_middle, uint8_t radius,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const detectorPosition_t& detectorPosition, const uint8_t* mask_linear, vector< uint32_t >& linearValidPixelCoordinates);
static inline float computeStreakThreshold(const float* data_linear, const streakFinder_precomputedConstants_t& streakFinder_precomputedConstants,
        const streakFinder_accuracyConstants_t& streakFinder_accuracyConstants, const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const vector< vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions);
static inline float computeRadialFilter(uint16_t x, uint16_t y, const float* data_linear, const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const streakFinder_precomputedConstants_t& streakFinder_precomputedConstants, const streakFinder_accuracyConstants_t& streakFinder_accuracyConstants);
static inline float computeDetectorPositionsHash(
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions,
        const streakFinder_accuracyConstants_t& streakFinder_accuracyConstants);

void streakFinder(float* data_linear, const streakFinder_accuracyConstants_t& accuracyConstants, const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const vector< vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions,
        const streakFinder_precomputedConstants_t& streakFinder_precomputedConstants)
{
    assert(streakFinder_precomputedConstants.detectorPositionsHash == computeDetectorPositionsHash(detectorPositions, accuracyConstants));

    streakFinder_precomputedConstants.filterDirectionVectors.size();
    vector< streakPixelsShort_t > streaksPixelsShort;
    streaksPixelsShort.reserve(streakFinder_precomputedConstants.filterDirectionVectors.size()); //does not need to be dynamic! can be preallocated and passed as parameter!

    float threshold = computeStreakThreshold(data_linear, streakFinder_precomputedConstants, accuracyConstants, detectorRawSize_cheetah,
            detectorPositions);

    for (uint8_t streakDetektorNumber = 0; streakDetektorNumber < accuracyConstants.streakDetektorsIndices.size(); ++streakDetektorNumber) {
        Point < uint_fast8_t > detectorToCheckIndex = accuracyConstants.streakDetektorsIndices[streakDetektorNumber];
        detectorPosition_t detectorPosition = detectorPositions[detectorToCheckIndex.getY()][detectorToCheckIndex.getX()];

        for (uint8_t lineToCheckNumber = 0; lineToCheckNumber < accuracyConstants.linesToCheck.size(); ++lineToCheckNumber) {
            float y_streakStart = detectorPosition.rawCoordinates_uint16.getLowerRightCorner().getY() - accuracyConstants.linesToCheck[lineToCheckNumber];

            for (uint16_t x_streakStart = detectorPosition.rawCoordinates_uint16.getUpperLeftCorner().getX() + 1, posOnLineToCheck = 0;
                    x_streakStart <= detectorPosition.rawCoordinates_uint16.getLowerRightCorner().getX() - 1; ++x_streakStart, ++posOnLineToCheck) {
                float filterValue = computeRadialFilter(x_streakStart, y_streakStart, data_linear, detectorRawSize_cheetah, streakFinder_precomputedConstants,
                        accuracyConstants);

                if (filterValue > threshold) {
                    int_fast16_t streakLength = 0;

                    const Vector2f& filterDirectionVector_normalized =
                            streakFinder_precomputedConstants.filterDirectionVectors[streakDetektorNumber][lineToCheckNumber][posOnLineToCheck];

                    Vector2f pointOnStreak = Vector2f(x_streakStart, y_streakStart) + filterDirectionVector_normalized;

                    uint_fast8_t stepsWithoutStreakPixel = 0;
                    float currentRadius = (detectorPosition.virtualZeroPositionRaw - pointOnStreak).norm();
                    float streakElongationStepCount = max((float) accuracyConstants.streakElongationMinStepsCount,
                            accuracyConstants.streakElongationRadiusFactor * currentRadius);
                    while (stepsWithoutStreakPixel < streakElongationStepCount && detectorPosition.rawCoordinates_float.contains(Point< float >(pointOnStreak))) {
                        streakLength++;

                        float filterValue = computeRadialFilter(boost::math::round((float) pointOnStreak(0)), boost::math::round((float) pointOnStreak(1)),
                                data_linear,
                                detectorRawSize_cheetah,
                                streakFinder_precomputedConstants, accuracyConstants);
                        if (filterValue > threshold) {
                            stepsWithoutStreakPixel = 0;
                            currentRadius = (detectorPosition.virtualZeroPositionRaw - pointOnStreak).norm();
                            streakElongationStepCount = max((float) accuracyConstants.streakElongationMinStepsCount,
                                    accuracyConstants.streakElongationRadiusFactor * currentRadius);
                        } else {
                            stepsWithoutStreakPixel++;
                        }

                        pointOnStreak += filterDirectionVector_normalized;
                    }

                    uint32_t numberOfPixelsToMask =
                            streakFinder_precomputedConstants.streaksPixels[streakDetektorNumber][lineToCheckNumber][posOnLineToCheck].numberOfPixelsToMaskForStreakLength[streakLength];
                    const vector< uint32_t > &pixelsToMaskIndices =
                            streakFinder_precomputedConstants.streaksPixels[streakDetektorNumber][lineToCheckNumber][posOnLineToCheck].pixelsToMaskIndices;
                    streakPixelsShort_t tmp;
                    tmp.pixelsToMaskIndices = (uint32_t*) &pixelsToMaskIndices[0];
                    tmp.numberOfPixelsToMask = numberOfPixelsToMask;
                    streaksPixelsShort.push_back(tmp);
                }
            }
        }
    }

    BOOST_FOREACH (const streakPixelsShort_t & streakPixelsShort , streaksPixelsShort)
    {
        for (uint32_t* nextPixelToMaskIndex = streakPixelsShort.pixelsToMaskIndices;
                nextPixelToMaskIndex < streakPixelsShort.pixelsToMaskIndices + streakPixelsShort.numberOfPixelsToMask; nextPixelToMaskIndex++) {
            data_linear[*nextPixelToMaskIndex] = -INFINITY;
        }
    }
}

void precomputeStreakFinderConstants(const streakFinder_accuracyConstants_t& streakFinder_accuracyConstants,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const vector< vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions, const uint8_t* mask_linear,
        streakFinder_precomputedConstants_t& streakFinder_precomputedConstants)
{
    precomputeFilterDirectionVectors(streakFinder_accuracyConstants, detectorRawSize_cheetah, detectorPositions, streakFinder_precomputedConstants);

    precomputeRadialFilterContributors(streakFinder_accuracyConstants, detectorRawSize_cheetah, detectorPositions, mask_linear,
            streakFinder_precomputedConstants);

    precomputeStreakPixels(streakFinder_accuracyConstants, detectorRawSize_cheetah, detectorPositions, mask_linear, streakFinder_precomputedConstants);

    streakFinder_precomputedConstants.detectorPositionsHash = computeDetectorPositionsHash(detectorPositions, streakFinder_accuracyConstants);

}

static inline void precomputeFilterDirectionVectors(const streakFinder_accuracyConstants_t& streakFinder_accuracyConstants,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const vector< vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions,
        streakFinder_precomputedConstants_t& streakFinder_precomputedConstants)
{
    vector < vector< vector< Vector2f, Eigen::aligned_allocator< Vector2f > > > > &filterDirectionVectors =
            streakFinder_precomputedConstants.filterDirectionVectors;

    filterDirectionVectors.resize(streakFinder_accuracyConstants.streakDetektorsIndices.size());
    typedef vector< vector< Vector2f, Eigen::aligned_allocator< Vector2f > > > streakDetektor_t;
    BOOST_FOREACH (streakDetektor_t & streakDetektor , filterDirectionVectors)
    {
        streakDetektor.resize(streakFinder_accuracyConstants.linesToCheck.size());

        typedef vector< Vector2f, Eigen::aligned_allocator< Vector2f > > lineToCheck_t;
        BOOST_FOREACH (lineToCheck_t & lineToCheck , streakDetektor)
        {
            lineToCheck.resize(detectorRawSize_cheetah.asic_nx - 2); //first and last pixel are masked and thus do not have to be considered
        }
    }

    for (uint8_t streakDetektorNumber = 0; streakDetektorNumber < streakFinder_accuracyConstants.streakDetektorsIndices.size(); ++streakDetektorNumber) {
        Point < uint_fast8_t > detectorToCheckIndex = streakFinder_accuracyConstants.streakDetektorsIndices[streakDetektorNumber];
        detectorPosition_t detectorPosition = detectorPositions[detectorToCheckIndex.getY()][detectorToCheckIndex.getX()];

        for (uint8_t lineToCheckNumber = 0; lineToCheckNumber < streakFinder_accuracyConstants.linesToCheck.size(); ++lineToCheckNumber) {
            float y_raw = detectorPosition.rawCoordinates_uint16.getLowerRightCorner().getY() - streakFinder_accuracyConstants.linesToCheck[lineToCheckNumber];

            for (uint16_t x_raw = detectorPosition.rawCoordinates_uint16.getUpperLeftCorner().getX() + 1, posOnLineToCheck = 0;
                    x_raw <= detectorPosition.rawCoordinates_uint16.getLowerRightCorner().getX() - 1;
                    ++x_raw, ++posOnLineToCheck
                    ) {

                Vector2f filterDirectionVector = Vector2f(x_raw, y_raw) - detectorPosition.virtualZeroPositionRaw;
                Vector2f filterDirectionVector_normalized = filterDirectionVector.normalized();

                filterDirectionVectors[streakDetektorNumber][lineToCheckNumber][posOnLineToCheck] = filterDirectionVector_normalized;
            }
        }
    }
}

static inline void precomputeRadialFilterContributors(const streakFinder_accuracyConstants_t& streakFinder_accuracyConstants,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const vector< vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions, const uint8_t* mask_linear,
        streakFinder_precomputedConstants_t& streakFinder_precomputedConstants)
{
    const uint8_t (*mask)[detectorRawSize_cheetah.pix_nx] = (uint8_t (*)[detectorRawSize_cheetah.pix_nx]) mask_linear;

    int32_t* & radialFilterContributors_linear = streakFinder_precomputedConstants.radialFilterContributors;
    radialFilterContributors_linear = new int32_t[detectorRawSize_cheetah.pix_nn * (streakFinder_accuracyConstants.filterLength + 1)];

    fill(radialFilterContributors_linear,
            radialFilterContributors_linear + detectorRawSize_cheetah.pix_nn * (streakFinder_accuracyConstants.filterLength + 1),
            -1);

    int32_t (*radialFilterContributors)[detectorRawSize_cheetah.pix_nx][streakFinder_accuracyConstants.filterLength + 1] =
            (int32_t (*)[detectorRawSize_cheetah.pix_nx][streakFinder_accuracyConstants.filterLength + 1]) radialFilterContributors_linear;

    vector < int32_t > currentFilterContributors;
    currentFilterContributors.reserve(streakFinder_accuracyConstants.filterLength);

    typedef Point< uint_fast8_t > detectorToCheck_t;
    BOOST_FOREACH(const detectorToCheck_t & detectorToCheck, streakFinder_accuracyConstants.streakDetektorsIndices)
    {
        detectorPosition_t detectorPosition = detectorPositions[detectorToCheck.getY()][detectorToCheck.getX()];

        for (uint_fast16_t y = detectorPosition.rawCoordinates_uint16.getUpperLeftCorner().getY();
                y <= detectorPosition.rawCoordinates_uint16.getLowerRightCorner().getY();
                ++y) {
            for (uint_fast16_t x = detectorPosition.rawCoordinates_uint16.getUpperLeftCorner().getX();
                    x <= detectorPosition.rawCoordinates_uint16.getLowerRightCorner().getX(); ++x) {
                Vector2f filterDirectionVector = Vector2f(x, y) - detectorPosition.virtualZeroPositionRaw;
                Vector2f filterDirectionVector_normalized = filterDirectionVector.normalized();
                Vector2f filterDirectionVector_adapted = streakFinder_accuracyConstants.filterStep * filterDirectionVector_normalized;

//                vector<Point<uint16_t, 2>> debug;

                currentFilterContributors.clear();
                for (uint_fast8_t i = 0; i < streakFinder_accuracyConstants.filterLength; ++i) {
                    Vector2f nextFilterPixel_pos = Vector2f(x, y) + i * filterDirectionVector_adapted;

                    Point < uint16_t > nextFilterPixel_posRounded = Point< float >(nextFilterPixel_pos).getRounded(); // was before: Point < uint16_t, 2 > nextFilterPixel_posRounded(round(nextFilterPixel_pos.getX()), round(nextFilterPixel_pos.getY()));
                    if (nextFilterPixel_posRounded > detectorPosition.rawCoordinates_uint16.getUpperLeftCorner()
                            && nextFilterPixel_posRounded < detectorPosition.rawCoordinates_uint16.getLowerRightCorner()
                            && mask[nextFilterPixel_posRounded.getY()][nextFilterPixel_posRounded.getX()] == 0) {

                        currentFilterContributors.push_back(
                                nextFilterPixel_posRounded.getY() * detectorRawSize_cheetah.pix_nx + nextFilterPixel_posRounded.getX());

//                        debug.push_back(nextFilterPixel_posRounded);
                    }
                }

                if (currentFilterContributors.size() >= streakFinder_accuracyConstants.minFilterLength) {
                    copy(currentFilterContributors.begin(), currentFilterContributors.end(), &radialFilterContributors[y][x][0]);
                }
            }
        }
    }
}

static inline void precomputeStreakPixels(const streakFinder_accuracyConstants_t& streakFinder_accuracyConstants,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const vector< vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions, const uint8_t* mask_linear,
        streakFinder_precomputedConstants_t& streakFinder_precomputedConstants)
{
    vector < vector< vector< streakPixels_t > > > &streaksPixels = streakFinder_precomputedConstants.streaksPixels;

    streaksPixels.resize(streakFinder_accuracyConstants.streakDetektorsIndices.size());

    BOOST_FOREACH(vector< vector< streakPixels_t > > & streakDetektor , streaksPixels)
    {
        streakDetektor.resize(streakFinder_accuracyConstants.linesToCheck.size());
        BOOST_FOREACH (vector< streakPixels_t > & lineToCheck , streakDetektor)
        {
            lineToCheck.resize(detectorRawSize_cheetah.asic_nx - 2); //first and last pixel are masked and thus do not have to be considered
        }
    }

    vector< uint32_t > pixelsToMask, newPixelsToMask, pixelsToMask_sorted, newPixelsToMask_cleaned;
    vector < uint32_t > numberOfPixelsToMaskForStreakLength;
    pixelsToMask.reserve(detectorRawSize_cheetah.asic_nx * detectorRawSize_cheetah.asic_ny);
    newPixelsToMask.reserve(detectorRawSize_cheetah.asic_nx * detectorRawSize_cheetah.asic_ny);
    pixelsToMask_sorted.reserve(detectorRawSize_cheetah.asic_nx * detectorRawSize_cheetah.asic_ny);
    newPixelsToMask_cleaned.reserve(detectorRawSize_cheetah.asic_nx * detectorRawSize_cheetah.asic_ny);
    numberOfPixelsToMaskForStreakLength.reserve(detectorRawSize_cheetah.asic_nx + detectorRawSize_cheetah.asic_ny);

    for (uint8_t streakDetektorNumber = 0; streakDetektorNumber < streakFinder_accuracyConstants.streakDetektorsIndices.size(); ++streakDetektorNumber) {
        Point < uint_fast8_t > detectorToCheckIndex = streakFinder_accuracyConstants.streakDetektorsIndices[streakDetektorNumber];
        detectorPosition_t detectorPosition = detectorPositions[detectorToCheckIndex.getY()][detectorToCheckIndex.getX()];

        for (uint8_t lineToCheckNumber = 0; lineToCheckNumber < streakFinder_accuracyConstants.linesToCheck.size(); ++lineToCheckNumber) {
            float y_streakStart = detectorPosition.rawCoordinates_uint16.getLowerRightCorner().getY()
                    - streakFinder_accuracyConstants.linesToCheck[lineToCheckNumber];

            for (uint16_t x_streakStart = detectorPosition.rawCoordinates_uint16.getUpperLeftCorner().getX() + 1, posOnLineToCheck = 0;
                    x_streakStart <= detectorPosition.rawCoordinates_uint16.getLowerRightCorner().getX() - 1; ++x_streakStart, ++posOnLineToCheck) {

                pixelsToMask.clear();
                numberOfPixelsToMaskForStreakLength.clear();
                pixelsToMask_sorted.clear();

                Vector2f filterDirectionVector = Vector2f(x_streakStart, y_streakStart) - detectorPosition.virtualZeroPositionRaw;
                Vector2f filterDirectionVector_normalized = filterDirectionVector.normalized();

                //backtrack streak
                Vector2f currentStreakPos_backtrack(x_streakStart, y_streakStart);
                while (detectorPosition.rawCoordinates_float.contains(Point< float >(currentStreakPos_backtrack))) {
                    getLinearValidCoordinatesInRadius((uint16_t) boost::math::round((float) currentStreakPos_backtrack(0)),
                            (uint16_t) boost::math::round((float) currentStreakPos_backtrack(1)),
                            streakFinder_accuracyConstants.streakPixelMaskRadius, detectorRawSize_cheetah, detectorPosition, mask_linear, newPixelsToMask);

                    pixelsToMask.insert(pixelsToMask.end(), newPixelsToMask.begin(), newPixelsToMask.end());

                    sort(pixelsToMask.begin(), pixelsToMask.end());
                    vector< uint32_t >::iterator it = unique(pixelsToMask.begin(), pixelsToMask.end());
                    pixelsToMask.resize(distance(pixelsToMask.begin(), it));

                    currentStreakPos_backtrack -= filterDirectionVector_normalized;
                }
                numberOfPixelsToMaskForStreakLength.push_back(pixelsToMask.size());

                pixelsToMask_sorted.insert(pixelsToMask_sorted.end(), pixelsToMask.begin(), pixelsToMask.end());
                sort(pixelsToMask_sorted.begin(), pixelsToMask_sorted.end());

                //mask as long as streak is possible
                Point< float > currentStreakPos = Point< float >(Vector2f(x_streakStart, y_streakStart) + filterDirectionVector_normalized);
                while (detectorPosition.rawCoordinates_float.contains(currentStreakPos)) {
                    getLinearValidCoordinatesInRadius(currentStreakPos.getRounded().getX(), currentStreakPos.getRounded().getY(),
                            streakFinder_accuracyConstants.streakPixelMaskRadius, detectorRawSize_cheetah, detectorPosition, mask_linear, newPixelsToMask);

                    newPixelsToMask_cleaned.clear();
                    sort(newPixelsToMask.begin(), newPixelsToMask.end());
                    set_difference(newPixelsToMask.begin(), newPixelsToMask.end(), pixelsToMask_sorted.begin(), pixelsToMask_sorted.end(),
                            inserter(newPixelsToMask_cleaned, newPixelsToMask_cleaned.begin()));

                    pixelsToMask.insert(pixelsToMask.end(), newPixelsToMask_cleaned.begin(), newPixelsToMask_cleaned.end());
                    numberOfPixelsToMaskForStreakLength.push_back(pixelsToMask.size());

                    pixelsToMask_sorted.insert(pixelsToMask_sorted.end(), newPixelsToMask_cleaned.begin(), newPixelsToMask_cleaned.end());
                    sort(pixelsToMask_sorted.begin(), pixelsToMask_sorted.end());

                    currentStreakPos += Point< float >(filterDirectionVector_normalized);
                }

                vector < uint32_t > &pixelsToMask_inPrecomputedArray =
                        streakFinder_precomputedConstants.streaksPixels[streakDetektorNumber][lineToCheckNumber][posOnLineToCheck].pixelsToMaskIndices;
                vector < uint32_t > &numberOfPixelsToMaskForStreakLength_inPrecomputedArray =
                        streakFinder_precomputedConstants.streaksPixels[streakDetektorNumber][lineToCheckNumber][posOnLineToCheck].numberOfPixelsToMaskForStreakLength;

                pixelsToMask_inPrecomputedArray.reserve(pixelsToMask.size());
                pixelsToMask_inPrecomputedArray.insert(pixelsToMask_inPrecomputedArray.end(), pixelsToMask.begin(), pixelsToMask.end());

                numberOfPixelsToMaskForStreakLength_inPrecomputedArray.reserve(numberOfPixelsToMaskForStreakLength.size());
                numberOfPixelsToMaskForStreakLength_inPrecomputedArray.insert(numberOfPixelsToMaskForStreakLength_inPrecomputedArray.end(),
                        numberOfPixelsToMaskForStreakLength.begin(), numberOfPixelsToMaskForStreakLength.end());
            }
        }
    }

}

static inline void getAllPixelCoordinatesInRadius(uint16_t x_middle, uint16_t y_middle, uint8_t radius, vector< Point< int16_t > >& pixelCoordinatesInRadius)
{
    pixelCoordinatesInRadius.clear();
    pixelCoordinatesInRadius.reserve((2 * radius + 1) * (2 * radius + 1));

    for (int16_t x = x_middle - radius; x <= x_middle + radius; ++x) {
        for (int16_t y = y_middle - radius; y <= y_middle + radius; ++y) {
            pixelCoordinatesInRadius.push_back(Point < int16_t > (x, y));
        }
    }
}

static inline void getValidPixelCoordinates(const vector< Point< int16_t > >& pixelCoordinates, const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const detectorPosition_t& detectorPosition, const uint8_t* mask_linear, vector< uint32_t >& linearValidPixelCoordinates)
{
    const uint8_t (*mask)[detectorRawSize_cheetah.pix_nx] = (uint8_t (*)[detectorRawSize_cheetah.pix_nx]) mask_linear;

    linearValidPixelCoordinates.clear();

    vector < Point< int16_t > > validPixelCoordinates_debug;

    typedef Point< int16_t > pixelCoordinate_t;
    BOOST_FOREACH(const pixelCoordinate_t & pixelCoordinate , pixelCoordinates)
    {
        if (pixelCoordinate >= Point < int16_t > (detectorPosition.rawCoordinates_uint16.getUpperLeftCorner())
                && pixelCoordinate <= Point < int16_t > (detectorPosition.rawCoordinates_uint16.getLowerRightCorner())
                && mask[pixelCoordinate.getY()][pixelCoordinate.getX()] == 0) {

            validPixelCoordinates_debug.push_back(pixelCoordinate);
            linearValidPixelCoordinates.push_back(pixelCoordinate.getY() * detectorRawSize_cheetah.pix_nx + pixelCoordinate.getX());
        }
    }
}

static inline void getLinearValidCoordinatesInRadius(uint16_t x_middle, uint16_t y_middle, uint8_t radius,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const detectorPosition_t& detectorPosition, const uint8_t* mask_linear, vector< uint32_t >& linearValidPixelCoordinates)
{
    vector < Point< int16_t > > pixelCoordinatesInRadius;

    getAllPixelCoordinatesInRadius(x_middle, y_middle, radius, pixelCoordinatesInRadius);
    getValidPixelCoordinates(pixelCoordinatesInRadius, detectorRawSize_cheetah, detectorPosition, mask_linear, linearValidPixelCoordinates);
}

static inline float computeStreakThreshold(const float* data_linear, const streakFinder_precomputedConstants_t& streakFinder_precomputedConstants,
        const streakFinder_accuracyConstants_t& streakFinder_accuracyConstants, const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const vector< vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions)
{
    const int maxRegionsCount = 50;

    const size_t regionsCount = streakFinder_accuracyConstants.backgroundEstimationRegionsInDetector.size()
            * streakFinder_accuracyConstants.streakDetektorsIndices.size();
    assert(regionsCount <= maxRegionsCount);

    float means[maxRegionsCount]; //using #define instead of pix_nx gives a tiny performance boost
    float sigmas[maxRegionsCount]; //using #define instead of pix_nx gives a tiny performance boost
    uint_fast8_t validRegionsEstimated = 0;

    typedef Point< uint_fast8_t > streakDetektorIndex_t;
    BOOST_FOREACH (const streakDetektorIndex_t & streakDetektorIndex , streakFinder_accuracyConstants.streakDetektorsIndices)
    {
        const detectorPosition_t &streakDetektorPosition = detectorPositions[streakDetektorIndex.getY()][streakDetektorIndex.getX()];
        typedef ImageRectangle< uint16_t > backgroundEstimationRegion_t;
        BOOST_FOREACH (const backgroundEstimationRegion_t & backgroundEstimationRegion , streakFinder_accuracyConstants.backgroundEstimationRegionsInDetector)
        {
            uint_fast32_t validValuesCount = 0;
            double sum = 0, sumOfSquares = 0;

//            vector< float > debug;

            for (uint16_t y = streakDetektorPosition.rawCoordinates_uint16.getUpperLeftCorner().getY() + backgroundEstimationRegion.getUpperLeftCorner().getY();
                    y <= streakDetektorPosition.rawCoordinates_uint16.getUpperLeftCorner().getY() + backgroundEstimationRegion.getLowerRightCorner().getY();
                    ++y) {
                for (uint16_t x = streakDetektorPosition.rawCoordinates_uint16.getUpperLeftCorner().getX()
                        + backgroundEstimationRegion.getUpperLeftCorner().getX();
                        x <= streakDetektorPosition.rawCoordinates_uint16.getUpperLeftCorner().getX() + backgroundEstimationRegion.getLowerRightCorner().getX();
                        ++x) {

                    float filterValue = computeRadialFilter(x, y, data_linear, detectorRawSize_cheetah, streakFinder_precomputedConstants,
                            streakFinder_accuracyConstants);

//                    debug.push_back(filterValue);

                    if (filterValue != -INFINITY) {
                        sumOfSquares += filterValue * filterValue;
                        sum += filterValue;
                        validValuesCount++;
                    }
                }
            }

            if (validValuesCount > 0) {
                float currentMean = (float) sum / validValuesCount;
                means[validRegionsEstimated] = currentMean;
//                sigmas[validRegionsEstimated] = sqrt(
//                        (float) sumOfSquares / (validValuesCount - 1) - (currentMean) * (currentMean) * validValuesCount / (float) (validValuesCount - 1));
                sigmas[validRegionsEstimated] = sqrt(((float) sumOfSquares - currentMean * currentMean * validValuesCount) / (float) (validValuesCount - 1));
                validRegionsEstimated++;
            }
        }
    }

    using namespace boost::phoenix::placeholders;
    uint8_t indices[regionsCount];
    boost::algorithm::iota(indices, indices + validRegionsEstimated, 0);
    nth_element(indices, indices + 1, indices + validRegionsEstimated, *(sigmas + arg1) < *(sigmas + arg2)); //compute index of second-largest element
//    nth_element(indices, indices + 1, indices + validRegionsEstimated, [&](size_t a, size_t b) {return sigmas[a] < sigmas[b];}); //compute index of second-largest element

    float threshold = means[indices[1]] + streakFinder_accuracyConstants.sigmaFactor * sigmas[indices[1]];
    return threshold;
}

static inline float computeRadialFilter(uint16_t x, uint16_t y, const float* data_linear, const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const streakFinder_precomputedConstants_t& streakFinder_precomputedConstants, const streakFinder_accuracyConstants_t& streakFinder_accuracyConstants)
{

    int32_t (*radialFilterContributors)[detectorRawSize_cheetah.pix_nx][streakFinder_accuracyConstants.filterLength + 1] =
            (int32_t (*)[detectorRawSize_cheetah.pix_nx][streakFinder_accuracyConstants.filterLength + 1]) streakFinder_precomputedConstants.radialFilterContributors;

    float filterContributors[streakFinder_accuracyConstants.filterLength]; //using #define instead of pix_nx gives a tiny performance boost
    float* nextFilterContributor = filterContributors;

    int32_t* nextContributorIndex = &radialFilterContributors[y][x][0];
    if (*nextContributorIndex < 0) {
        return -INFINITY;
    }

    while (*nextContributorIndex >= 0) {
        *nextFilterContributor = data_linear[*nextContributorIndex];
        nextFilterContributor++;
        nextContributorIndex++;
    }

    float* median = filterContributors + (nextFilterContributor - filterContributors) / 2;
    nth_element(filterContributors, median, nextFilterContributor);
    float filterValue = accumulate(filterContributors, median, *median) / (median - filterContributors + 1);

    return filterValue;
}

void freePrecomputeStreakFinderConstants(streakFinder_precomputedConstants_t& streakFinder_precomputedConstants)
{
    delete[] streakFinder_precomputedConstants.radialFilterContributors;
}

static float computeDetectorPositionsHash(
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions,
        const streakFinder_accuracyConstants_t& streakFinder_accuracyConstants)
{
    float hash = 0;

    for (uint8_t streakDetektorNumber = 0; streakDetektorNumber < streakFinder_accuracyConstants.streakDetektorsIndices.size(); ++streakDetektorNumber) {
        Point < uint_fast8_t > detectorToCheckIndex = streakFinder_accuracyConstants.streakDetektorsIndices[streakDetektorNumber];
        detectorPosition_t detectorPosition = detectorPositions[detectorToCheckIndex.getY()][detectorToCheckIndex.getX()];

        hash += (detectorPosition.fs * 128 + detectorPosition.corner).sum();
    }

    return hash;
}

