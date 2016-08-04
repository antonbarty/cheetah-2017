/*
 * radialBackgroundSubtraction.cpp
 *
 *  Created on: 26.06.2016
 *      Author: Yaro
 */

#include "radialBackgroundSubtraction.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include "matlabLikeFunctions.h"
#include "sortingByOtherValues.h"

using namespace Eigen;

static void gatherAvailableRadii(std::vector< float >& availableRadii, std::vector< Point2D< uint16_t > >& radiiMatrixIndices,
        const radialRankFilter_accuracyConstants_t& accuracyConstants, const uint8_t* mask_linear,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah, const float* detectorGeometryRadiusMatrix_linear);
static void fillBins(std::vector< std::vector< Point2D< uint16_t > > >& binsWithIndices, std::vector< std::vector< float > >& binsWithRadii,
        std::vector< float > &availableRadii, std::vector< Point2D< uint16_t > >& radiiMatrixIndices,
        const radialRankFilter_accuracyConstants_t& accuracyConstants);
static void thinOutBins(std::vector< std::vector< float > > &binsWithRadii, std::vector< std::vector< Point2D< uint16_t > > >& binsWithIndices,
        const radialRankFilter_accuracyConstants_t& accuracyConstants, const Vector2f* detectorGeometryMatrix_linear,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah);
static void computeAngles(std::vector< float > &angles, const std::vector< Point2D< uint16_t > >& radiiMatrixIndices,
        const Vector2f* detectorGeometryMatrix_linear, const detectorRawSize_cheetah_t& detectorRawSize_cheetah);
static void computeBinsWithLinearindicesFromBinsWithMatrixIndices(std::vector< std::vector< uint32_t > >& binsWithLinearIndices,
        const std::vector< std::vector< Point2D< uint16_t > > >& binsWithIndices, const detectorRawSize_cheetah_t& detectorRawSize_cheetah);
static void computeSparsePrecomputedConstants(radialRankFilter_precomputedConstants_t& precomputedConstants,
        const std::vector< std::vector< uint32_t > >& binsWithLinearIndices);
static void computeBinRadii(radialRankFilter_precomputedConstants_t& precomputedConstants, const std::vector< std::vector< float > >& binsWithRadii);
static void computeDataCountPerBin(radialRankFilter_precomputedConstants_t& precomputedConstants, const std::vector< std::vector< float > >& binsWithRadii);
static void computeIntraBinIndices(radialRankFilter_precomputedConstants_t& precomputedConstants, const uint8_t* mask_linear,
        const std::vector< std::vector< uint32_t > >& binsWithLinearIndices, const float* detectorGeometryRadiusMatrix_linear,
        const radialRankFilter_accuracyConstants_t& accuracyConstants, const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions);
static void computeIntraBinInterpolationConstant(radialRankFilter_precomputedConstants_t& precomputedConstants, const uint8_t* mask_linear,
        const float* detectorGeometryRadiusMatrix_linear, const radialRankFilter_accuracyConstants_t& accuracyConstants,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions);

static void gatherBinsData(std::vector< std::vector< float > >& binsWithData, const float* data_linear,
        const radialRankFilter_precomputedConstants_t& precomputedConstants);
static void computeBinValues(std::vector< float >& binValues, std::vector< std::vector< float > >& binsWithData,
        const radialRankFilter_precomputedConstants_t& precomputedConstants, const radialRankFilter_accuracyConstants_t& accuracyConstants);

void precomputeRadialRankFilterConstants(radialRankFilter_precomputedConstants_t& precomputedConstants, const uint8_t* mask_linear,
        const float* detectorGeometryRadiusMatrix_linear,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah, const radialRankFilter_accuracyConstants_t& accuracyConstants,
        const Eigen::Vector2f* detectorGeometryMatrix_linear)
{

    std::vector< float > availableRadii;
    std::vector < Point2D< uint16_t > > radiiMatrixIndices;

    gatherAvailableRadii(availableRadii, radiiMatrixIndices, accuracyConstants, mask_linear, detectorPositions, detectorRawSize_cheetah,
            detectorGeometryRadiusMatrix_linear);

    std::vector < std::vector< Point2D< uint16_t > > > binsWithIndices;
    std::vector < std::vector< float > > binsWithRadii;

    fillBins(binsWithIndices, binsWithRadii, availableRadii, radiiMatrixIndices, accuracyConstants);

    std::vector < std::vector< Point2D< uint16_t > > > binsWithIndices_thinedOut(binsWithIndices);
    std::vector < std::vector< float > > binsWithRadii_thinedOut(binsWithRadii);

    thinOutBins(binsWithRadii_thinedOut, binsWithIndices_thinedOut, accuracyConstants, detectorGeometryMatrix_linear, detectorRawSize_cheetah);

    std::vector < std::vector< uint32_t > > binsWithLinearIndices_thinedOut;

    computeBinsWithLinearindicesFromBinsWithMatrixIndices(binsWithLinearIndices_thinedOut, binsWithIndices_thinedOut, detectorRawSize_cheetah);
    computeSparsePrecomputedConstants(precomputedConstants, binsWithLinearIndices_thinedOut);

    computeBinRadii(precomputedConstants, binsWithRadii_thinedOut);
    precomputedConstants.binCount = binsWithRadii.size() + 2;

    computeDataCountPerBin(precomputedConstants, binsWithRadii_thinedOut);

    std::vector < std::vector< uint32_t > > binsWithLinearIndices;

    computeBinsWithLinearindicesFromBinsWithMatrixIndices(binsWithLinearIndices, binsWithIndices, detectorRawSize_cheetah);
    computeIntraBinIndices(precomputedConstants, mask_linear, binsWithLinearIndices, detectorGeometryRadiusMatrix_linear, accuracyConstants,
            detectorRawSize_cheetah, detectorPositions);

    computeIntraBinInterpolationConstant(precomputedConstants, mask_linear, detectorGeometryRadiusMatrix_linear, accuracyConstants, detectorRawSize_cheetah,
            detectorPositions);
}

void applyRadialRankFilter(float* data_linear, const radialRankFilter_accuracyConstants_t& accuracyConstants,
        const radialRankFilter_precomputedConstants_t& precomputedConstants, const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions)
{
    std::vector < std::vector< float > > binsWithData;
    gatherBinsData(binsWithData, data_linear, precomputedConstants);

    std::vector< float > binValues;
    computeBinValues(binValues, binsWithData, precomputedConstants, accuracyConstants);

    //*******debug
//    for (uint32_t i=0; i < detectorRawSize_cheetah.pix_nn; ++i) {
//        data_linear[i] = 0;
//    }
    //*******debug

    for (uint8_t detektorToCrrectNumber = 0; detektorToCrrectNumber < accuracyConstants.detektorsToCorrectIndices.size(); ++detektorToCrrectNumber) {
        const Point2D< uint_fast8_t >& detektorToCorrectIndex = accuracyConstants.detektorsToCorrectIndices[detektorToCrrectNumber];

        const detectorPosition_t detectorPosition = detectorPositions[detektorToCorrectIndex.getY()][detektorToCorrectIndex.getX()];

        for (uint16_t y = detectorPosition.rawCoordinates_uint16.getUpperLeftCorner().getY() + 1;
                y <= detectorPosition.rawCoordinates_uint16.getLowerRightCorner().getY() - 1; ++y) {
            for (uint16_t x = detectorPosition.rawCoordinates_uint16.getUpperLeftCorner().getX() + 1;
                    x <= detectorPosition.rawCoordinates_uint16.getLowerRightCorner().getX() - 1; ++x) {
                uint32_t linearIndex = getLinearIndexFromMatrixIndex(x, y, detectorRawSize_cheetah);
                if (data_linear[linearIndex] != INFINITY) {
                    data_linear[linearIndex] -= binValues[precomputedConstants.intraBinIndices[linearIndex]]
                            + precomputedConstants.intraBinInterpolationConstant[linearIndex]
                                    * (binValues[precomputedConstants.intraBinIndices[linearIndex] + 1]
                                            - binValues[precomputedConstants.intraBinIndices[linearIndex]]);
                }
            }
        }
    }
}

static void gatherBinsData(std::vector< std::vector< float > >& binsWithData, const float* data_linear,
        const radialRankFilter_precomputedConstants_t& precomputedConstants)
{
    binsWithData.resize(precomputedConstants.binCount);
    for (uint32_t i = 1; i < binsWithData.size() - 1; ++i) {
        binsWithData[i].reserve(precomputedConstants.dataCountPerBin[i]);
    }

    for (uint32_t i = 0; i < precomputedConstants.sparseLinearDataToConsiderIndices.size(); ++i) {
        binsWithData[precomputedConstants.sparseBinIndices[i]].push_back(data_linear[precomputedConstants.sparseLinearDataToConsiderIndices[i]]);
    }
}

static void computeBinValues(std::vector< float >& binValues, std::vector< std::vector< float > >& binsWithData,
        const radialRankFilter_precomputedConstants_t& precomputedConstants, const radialRankFilter_accuracyConstants_t& accuracyConstants)
{
    binValues.resize(binsWithData.size());
    for (uint32_t i = 1; i < binsWithData.size() - 1; ++i) {
        uint32_t intRank = std::max((uint32_t)(accuracyConstants.rank * binsWithData[i].size()), (uint32_t) 1) - 1;
        std::nth_element(binsWithData[i].begin(), binsWithData[i].begin() + intRank, binsWithData[i].end());
        binValues[i] = binsWithData[i][intRank];
    }

    binValues[0] = binValues[1]
            + (binValues[1] - binValues[2]) /
                    (precomputedConstants.binRadii[2] - precomputedConstants.binRadii[1]) *
                    (precomputedConstants.binRadii[1] - precomputedConstants.binRadii[0]);

    uint32_t lastIndex = binValues.size() - 1;
    binValues[lastIndex] = binValues[lastIndex - 1] +
            (binValues[lastIndex - 1] - binValues[lastIndex - 2]) /
                    (precomputedConstants.binRadii[lastIndex - 1] - precomputedConstants.binRadii[lastIndex - 2]) *
                    (precomputedConstants.binRadii[lastIndex] - precomputedConstants.binRadii[lastIndex - 1]);
}

static void gatherAvailableRadii(std::vector< float > &availableRadii, std::vector< Point2D< uint16_t > > &radiiMatrixIndices,
        const radialRankFilter_accuracyConstants_t& accuracyConstants, const uint8_t* mask_linear,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah, const float* detectorGeometryRadiusMatrix_linear)
{
    const uint8_t (*mask)[detectorRawSize_cheetah.pix_nx] = (uint8_t (*)[detectorRawSize_cheetah.pix_nx]) mask_linear;
    const float (*detectorGeometryRadiusMatrix)[detectorRawSize_cheetah.pix_nx] =
            (float (*)[detectorRawSize_cheetah.pix_nx]) detectorGeometryRadiusMatrix_linear;

    for (uint8_t detektorToConsiderNumber = 0; detektorToConsiderNumber < accuracyConstants.detektorsToConsiderIndices.size(); ++detektorToConsiderNumber) {
        const Point2D< uint_fast8_t >& detektorToConsiderIndex = accuracyConstants.detektorsToConsiderIndices[detektorToConsiderNumber];

        const detectorPosition_t detectorPosition = detectorPositions[detektorToConsiderIndex.getY()][detektorToConsiderIndex.getX()];

        for (uint16_t y = detectorPosition.rawCoordinates_uint16.getUpperLeftCorner().getY() + 1;
                y <= detectorPosition.rawCoordinates_uint16.getLowerRightCorner().getY() - 1; ++y) {
            for (uint16_t x = detectorPosition.rawCoordinates_uint16.getUpperLeftCorner().getX() + 1;
                    x <= detectorPosition.rawCoordinates_uint16.getLowerRightCorner().getX() - 1; ++x) {
                if (mask[y][x] == 0) {
                    availableRadii.push_back(detectorGeometryRadiusMatrix[y][x]);
                    radiiMatrixIndices.push_back(Point2D < uint16_t > (x, y));
                }
            }
        }
    }
}

static void fillBins(std::vector< std::vector< Point2D< uint16_t > > >& binsWithIndices, std::vector< std::vector< float > >& binsWithRadii,
        std::vector< float >& availableRadii, std::vector< Point2D< uint16_t > >& radiiMatrixIndices,
        const radialRankFilter_accuracyConstants_t& accuracyConstants)
{
    sortTwoVectorsByFirstVector(availableRadii, radiiMatrixIndices);

    binsWithIndices.clear();
    binsWithIndices.resize(1);
    binsWithRadii.clear();
    binsWithRadii.resize(1);

    for (uint32_t i = 0; i < availableRadii.size(); ++i) {
        if (binsWithIndices.back().size() < accuracyConstants.minValuesPerBin ||
                binsWithRadii.back().back() - binsWithRadii.back().front() < accuracyConstants.minBinWidth) {
            binsWithIndices.back().push_back(radiiMatrixIndices[i]);
            binsWithRadii.back().push_back(availableRadii[i]);
        } else {
            binsWithIndices.push_back(std::vector < Point2D< uint16_t > > (1, radiiMatrixIndices[i]));
            binsWithRadii.push_back(std::vector< float >(1, availableRadii[i]));
        }
    }
}

static void thinOutBins(std::vector< std::vector< float > >& binsWithRadii, std::vector< std::vector< Point2D< uint16_t > > >& binsWithIndices,
        const radialRankFilter_accuracyConstants_t& accuracyConstants, const Vector2f* detectorGeometryMatrix_linear,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah)
{
    if (accuracyConstants.maxConsideredValuesPerBin == 0) {
        return;
    }

    for (uint32_t i = 0; i < binsWithRadii.size(); ++i) {
        std::vector< float > &currentRadii = binsWithRadii[i];
        std::vector < Point2D< uint16_t > > &currentIndices = binsWithIndices[i];

        std::vector< float > angles;
        computeAngles(angles, currentIndices, detectorGeometryMatrix_linear, detectorRawSize_cheetah);
        sortThreeVectorsByFirstVector(angles, currentRadii, currentIndices);

        uint32_t N = currentRadii.size();

        if (N > accuracyConstants.maxConsideredValuesPerBin) {
            std::vector< float > floatLinspace;
            linspace(0, N, accuracyConstants.maxConsideredValuesPerBin + 1).swap(floatLinspace);

            std::vector < uint32_t > vectorIndicesToConsider(floatLinspace.begin(), floatLinspace.end() - 1);

            std::vector< float > tempRadii(vectorIndicesToConsider.size());
            std::vector < Point2D< uint16_t > > tempIndices(vectorIndicesToConsider.size());

            for (uint32_t i = 0; i < vectorIndicesToConsider.size(); ++i) {
                tempRadii[i] = currentRadii[vectorIndicesToConsider[i]];
                tempIndices[i] = currentIndices[vectorIndicesToConsider[i]];
            }
            tempRadii.swap(currentRadii);
            tempIndices.swap(currentIndices);
        }
    }
}

static void computeAngles(std::vector< float >& angles, const std::vector< Point2D< uint16_t > >& radiiMatrixIndices,
        const Vector2f* detectorGeometryMatrix_linear, const detectorRawSize_cheetah_t& detectorRawSize_cheetah)
{
    const Vector2f (*detectorGeometryMatrix)[detectorRawSize_cheetah.pix_nx] =
            (const Eigen::Vector2f (*)[detectorRawSize_cheetah.pix_nx]) detectorGeometryMatrix_linear;

    angles.resize(radiiMatrixIndices.size());

    for (uint32_t i = 0; i < radiiMatrixIndices.size(); ++i) {
        Point2D < uint16_t > matrixIndex = radiiMatrixIndices[i];
        const Vector2f position = detectorGeometryMatrix[matrixIndex.getY()][matrixIndex.getX()];
        float angle = atan2(position.y(), position.x());
        angles[i] = angle;
    }
}

static void computeBinsWithLinearindicesFromBinsWithMatrixIndices(std::vector< std::vector< uint32_t > >& binsWithLinearIndices,
        const std::vector< std::vector< Point2D< uint16_t > > >& binsWithIndices, const detectorRawSize_cheetah_t& detectorRawSize_cheetah)
{
    binsWithLinearIndices.resize(binsWithIndices.size());
    for (uint32_t i = 0; i < binsWithIndices.size(); ++i) {
        binsWithLinearIndices[i].resize(binsWithIndices[i].size());
        for (uint32_t j = 0; j < binsWithIndices[i].size(); ++j) {
            const Point2D< uint16_t > matrixIndex = binsWithIndices[i][j];
            binsWithLinearIndices[i][j] = getLinearIndexFromMatrixIndex(matrixIndex, detectorRawSize_cheetah);
        }
    }
}

static void computeSparsePrecomputedConstants(radialRankFilter_precomputedConstants_t& precomputedConstants,
        const std::vector< std::vector< uint32_t > >& binsWithLinearIndices)
{
    precomputedConstants.sparseBinIndices.clear();
    precomputedConstants.sparseLinearDataToConsiderIndices.clear();

    for (uint16_t i = 0; i < binsWithLinearIndices.size(); ++i) {
        for (uint32_t j = 0; j < binsWithLinearIndices[i].size(); ++j) {
            precomputedConstants.sparseLinearDataToConsiderIndices.push_back(binsWithLinearIndices[i][j]);
            precomputedConstants.sparseBinIndices.push_back(i + 1);   //+1 because a first and last bin are added for interpolation
        }
    }

    sortTwoVectorsByFirstVector(precomputedConstants.sparseLinearDataToConsiderIndices, precomputedConstants.sparseBinIndices);
}

static void computeBinRadii(radialRankFilter_precomputedConstants_t& precomputedConstants, const std::vector< std::vector< float > >& binsWithRadii)
{
    precomputedConstants.binRadii.resize(binsWithRadii.size() + 2); //+2 because a first and last bin are added for interpolation
    float minRadius = INFINITY;
    float maxRadius = 0;
    for (uint32_t i = 0; i < binsWithRadii.size(); ++i) {
        double sum = 0;
        for (uint32_t j = 0; j < binsWithRadii[i].size(); ++j) {
            sum += binsWithRadii[i][j];

            if (binsWithRadii[i][j] < minRadius) {
                minRadius = binsWithRadii[i][j];
            }
            if (binsWithRadii[i][j] > maxRadius) {
                maxRadius = binsWithRadii[i][j];
            }
        }
        precomputedConstants.binRadii[i + 1] = (float) (sum / binsWithRadii[i].size());
    }
    precomputedConstants.binRadii[0] = minRadius;
    precomputedConstants.binRadii.back() = maxRadius;
}

static void computeDataCountPerBin(radialRankFilter_precomputedConstants_t& precomputedConstants, const std::vector< std::vector< float > >& binsWithRadii)
{
    precomputedConstants.dataCountPerBin.resize(precomputedConstants.binCount);
    for (uint32_t i = 0; i < binsWithRadii.size(); ++i) {
        precomputedConstants.dataCountPerBin[i + 1] = binsWithRadii[i].size();
    }
    precomputedConstants.dataCountPerBin[0] = 0;
    precomputedConstants.dataCountPerBin.back() = 0;
}

static void computeIntraBinIndices(radialRankFilter_precomputedConstants_t& precomputedConstants, const uint8_t* mask_linear,
        const std::vector< std::vector< uint32_t > >& binsWithLinearIndices, const float* detectorGeometryRadiusMatrix_linear,
        const radialRankFilter_accuracyConstants_t& accuracyConstants, const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions)
{
    std::vector < uint16_t > binIndices(detectorRawSize_cheetah.pix_nn);
    for (uint16_t i = 0; i < binsWithLinearIndices.size(); ++i) {
        for (uint32_t j = 0; j < binsWithLinearIndices[i].size(); ++j) {
            binIndices[binsWithLinearIndices[i][j]] = i + 1; //+1 because a first and last bin are added for interpolation
        }
    }

    sortTwoVectorsByFirstVector(precomputedConstants.sparseLinearDataToConsiderIndices, precomputedConstants.sparseBinIndices);

    precomputedConstants.intraBinIndices.resize(detectorRawSize_cheetah.pix_nn);

    for (uint8_t detektorToCrrectNumber = 0; detektorToCrrectNumber < accuracyConstants.detektorsToCorrectIndices.size(); ++detektorToCrrectNumber) {
        const Point2D< uint_fast8_t >& detektorToCrrectIndex = accuracyConstants.detektorsToCorrectIndices[detektorToCrrectNumber];

        const detectorPosition_t detectorPosition = detectorPositions[detektorToCrrectIndex.getY()][detektorToCrrectIndex.getX()];

        for (uint16_t y = detectorPosition.rawCoordinates_uint16.getUpperLeftCorner().getY() + 1;
                y <= detectorPosition.rawCoordinates_uint16.getLowerRightCorner().getY() - 1; ++y) {
            for (uint16_t x = detectorPosition.rawCoordinates_uint16.getUpperLeftCorner().getX() + 1;
                    x <= detectorPosition.rawCoordinates_uint16.getLowerRightCorner().getX() - 1; ++x) {
                uint32_t linearIndex = getLinearIndexFromMatrixIndex(x, y, detectorRawSize_cheetah);
                if (mask_linear[linearIndex] == 0) {
                    if (detectorGeometryRadiusMatrix_linear[linearIndex] < precomputedConstants.binRadii[binIndices[linearIndex]]) {
                        precomputedConstants.intraBinIndices[linearIndex] = binIndices[linearIndex] - 1;
                    } else {
                        precomputedConstants.intraBinIndices[linearIndex] = binIndices[linearIndex];
                    }
                }
            }
        }
    }
}

static void computeIntraBinInterpolationConstant(radialRankFilter_precomputedConstants_t& precomputedConstants, const uint8_t* mask_linear,
        const float* detectorGeometryRadiusMatrix_linear, const radialRankFilter_accuracyConstants_t& accuracyConstants,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions)
{
    precomputedConstants.intraBinInterpolationConstant.resize(detectorRawSize_cheetah.pix_nn);

    for (uint8_t detektorToCrrectNumber = 0; detektorToCrrectNumber < accuracyConstants.detektorsToCorrectIndices.size(); ++detektorToCrrectNumber) {
        const Point2D< uint_fast8_t >& detektorToCrrectIndex = accuracyConstants.detektorsToCorrectIndices[detektorToCrrectNumber];

        const detectorPosition_t detectorPosition = detectorPositions[detektorToCrrectIndex.getY()][detektorToCrrectIndex.getX()];

        for (uint16_t y = detectorPosition.rawCoordinates_uint16.getUpperLeftCorner().getY() + 1;
                y <= detectorPosition.rawCoordinates_uint16.getLowerRightCorner().getY() - 1; ++y) {
            for (uint16_t x = detectorPosition.rawCoordinates_uint16.getUpperLeftCorner().getX() + 1;
                    x <= detectorPosition.rawCoordinates_uint16.getLowerRightCorner().getX() - 1; ++x) {
                uint32_t linearIndex = getLinearIndexFromMatrixIndex(x, y, detectorRawSize_cheetah);
                if (mask_linear[linearIndex] == 0) {
                    precomputedConstants.intraBinInterpolationConstant[linearIndex] = (detectorGeometryRadiusMatrix_linear[linearIndex]
                            - precomputedConstants.binRadii[precomputedConstants.intraBinIndices[linearIndex]])
                            / (precomputedConstants.binRadii[precomputedConstants.intraBinIndices[linearIndex] + 1]
                                    - precomputedConstants.binRadii[precomputedConstants.intraBinIndices[linearIndex]]);
                }
            }
        }
    }
}
