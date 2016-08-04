/*
 * peakFinder.cpp
 *
 *  Created on: 12.12.2015
 *      Author: Yaro
 */

#include "peakFinder.h"
//#include <math.h>
#include <cmath>

typedef struct {
    float totalMass;
    float weightedCoordinatesSummed_x, weightedCoordinatesSummed_y;
    float biggestPixelMass;
    uint_fast8_t pixelsCount;
} peakFinder9_intermediatePeakStatistics_t;

static inline bool isPixelCandidateForPeak(const float* data_linear, const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const peakFinder9_accuracyConstants_t& accuracyConstants, uint_fast16_t x, uint_fast16_t y);
static inline void computeNormalDistributionParameters(const float* data_linear, const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const peakFinder9_accuracyConstants_t& accuracyConstants, uint_fast16_t x, uint_fast16_t y, float* mean, float* sigma);
static inline void analysePeak(uint_fast16_t x, uint_fast16_t y, float thresholdNeighbourPixel, const float* data_linear,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah, const peakFinder9_accuracyConstants_t& accuracyConstants,
        peakFinder9_intermediatePeakStatistics_t& intermediatePeakStatistics);
static inline void analyseRingAroundPixel(uint_fast8_t radius, float thresholdNeighbourPixel, uint_fast16_t x, uint_fast16_t y, const float* data_linear,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah, peakFinder9_intermediatePeakStatistics_t& intermediatePeakStatistics, bool* newPixelFound);
static inline void addPixelTointermediatePeakStatistics(peakFinder9_intermediatePeakStatistics_t& intermediatePeakStatistics, uint_fast16_t x,
        uint_fast16_t y, float pixelValue);
static inline void savePeak(float sigmaBackground, float meanBackground, const peakFinder9_intermediatePeakStatistics_t& intermediatePeakStatistics,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah, tPeakList& peakList);

uint32_t peakFinder9(const float* data_linear, const peakFinder9_accuracyConstants_t& accuracyConstants,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah, tPeakList& peakList)
{
    uint32_t peakCount = 0;

    for (uint32_t asic_y = 0; asic_y < detectorRawSize_cheetah.nasics_y; ++asic_y) {
        for (uint32_t asic_x = 0; asic_x < detectorRawSize_cheetah.nasics_x; ++asic_x) {
            peakCount += peakFinder9_oneDetector(data_linear, asic_x, asic_y, accuracyConstants, detectorRawSize_cheetah, peakList);
        }
    }

    return peakCount;
}

//returns number of peaks found
uint32_t peakFinder9_oneDetector(const float* data_linear, uint32_t asic_x, uint32_t asic_y, const peakFinder9_accuracyConstants_t& accuracyConstants,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah, tPeakList& peakList)
{
    uint_fast16_t x_asicStart = asic_x * detectorRawSize_cheetah.asic_nx;
    uint_fast16_t y_asicStart = asic_y * detectorRawSize_cheetah.asic_ny;

    const float (*data)[detectorRawSize_cheetah.pix_nx] = (const float (*)[detectorRawSize_cheetah.pix_nx]) data_linear; //using #define instead of pix_nx gives a tiny performance boost

    uint_fast16_t windowRadius = accuracyConstants.windowRadius;
    uint32_t peakCount = 0;

    for (uint_fast16_t y = y_asicStart + windowRadius; y <= y_asicStart + detectorRawSize_cheetah.asic_ny - 1 - windowRadius; ++y) {
        for (uint_fast16_t x = x_asicStart + windowRadius; x <= x_asicStart + detectorRawSize_cheetah.asic_nx - 1 - windowRadius; ++x) {
            if (isPixelCandidateForPeak(data_linear, detectorRawSize_cheetah, accuracyConstants, x, y)) {
                float meanBackground, sigmaBackground;
                computeNormalDistributionParameters(data_linear, detectorRawSize_cheetah, accuracyConstants, x, y, &meanBackground, &sigmaBackground);

                float thresholdSinglePixel = meanBackground + accuracyConstants.sigmaFactorBiggestPixel * sigmaBackground;
                if (data[y][x] > thresholdSinglePixel) {
                    float thresholdNeighbourPixel = meanBackground + accuracyConstants.sigmaFactorPeakPixel * sigmaBackground;
                    peakFinder9_intermediatePeakStatistics_t intermediatePeakStatistics;
                    analysePeak(x, y, thresholdNeighbourPixel, data_linear, detectorRawSize_cheetah, accuracyConstants, intermediatePeakStatistics);

                    float thresholdWholePeak = meanBackground + accuracyConstants.sigmaFactorWholePeak * sigmaBackground;
                    if (intermediatePeakStatistics.totalMass > thresholdWholePeak) {
                        savePeak(sigmaBackground, meanBackground, intermediatePeakStatistics, detectorRawSize_cheetah, peakList);
                        peakCount++;
                    }
                }
            }
        }
    }

    return peakCount;
}

//writing this function directly into peakFinder9_oneDetector saves some milliseconds
static inline bool isPixelCandidateForPeak(const float* data_linear, const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const peakFinder9_accuracyConstants_t& accuracyConstants, uint_fast16_t x, uint_fast16_t y)
{

    const float (*data)[detectorRawSize_cheetah.pix_nx] = (float (*)[detectorRawSize_cheetah.pix_nx]) data_linear;

    if (data[y][x] == -INFINITY) {
        return false;
    }

    uint_fast16_t windowRadius = accuracyConstants.windowRadius;

    float adjustedPixel = data[y][x] - accuracyConstants.minimumPeakOversizeOverNeighbours;

    if (adjustedPixel > data[y][x - windowRadius] &&
            adjustedPixel > data[y][x + windowRadius] &&
            adjustedPixel > data[y - 1][x - windowRadius] &&
            adjustedPixel > data[y - 1][x + windowRadius] &&
            adjustedPixel > data[y - windowRadius][x - 1] &&
            adjustedPixel > data[y - windowRadius][x] &&
            adjustedPixel > data[y - windowRadius][x + 1] &&
            adjustedPixel > data[y + 1][x - windowRadius] &&
            adjustedPixel > data[y + 1][x + windowRadius] &&
            adjustedPixel > data[y + windowRadius][x - 1] &&
            adjustedPixel > data[y + windowRadius][x] &&
            adjustedPixel > data[y + windowRadius][x + 1] &&
            data[y][x] > data[y - 1][x - 1] &&
            data[y][x] > data[y - 1][x] &&
            data[y][x] > data[y - 1][x + 1] &&
            data[y][x] > data[y][x - 1] &&
            data[y][x] > data[y][x + 1] &&
            data[y][x] > data[y + 1][x - 1] &&
            data[y][x] > data[y + 1][x] &&
            data[y][x] > data[y + 1][x + 1]) {
        return true;
    } else {
        return false;
    }
}

//theoretically best true true, but need to test!
#define COMPUTE_ON_THE_FLY              false
#define ONE_PASS_COMPUTATION_METHOD      true

static inline void computeNormalDistributionParameters(const float* data_linear, const detectorRawSize_cheetah_t& detectorRawSize_cheetah,
        const peakFinder9_accuracyConstants_t& accuracyConstants, uint_fast16_t x, uint_fast16_t y, float* mean, float* sigma)
{

    const float (*data)[detectorRawSize_cheetah.pix_nx] = (float (*)[detectorRawSize_cheetah.pix_nx]) data_linear;
    uint_fast16_t windowRadius = accuracyConstants.windowRadius;

#if COMPUTE_ON_THE_FLY && !ONE_PASS_COMPUTATION_METHOD
#warning "Computing on the fly makes no sense for naive computation method!"
#endif

#if COMPUTE_ON_THE_FLY && ONE_PASS_COMPUTATION_METHOD
//compute on the fly (only for changed computation method)
    double sum = 0;
    double sumOfSquares = 0;
    uint_fast8_t validPixelCount = 0;

//compute mean and sigma from border
//upper border
    for (const float* currentPixel = &data[y - windowRadius][x - 2]; currentPixel <= &data[y - windowRadius][x + 2]; ++currentPixel) {
        if (*currentPixel != -INFINITY) {
            float pixelValue = *currentPixel;
            sumOfSquares += pixelValue * pixelValue;
            sum += pixelValue;
            validPixelCount++;
        }
    }

//left and right border
    for (int_fast8_t i = -2; i <= 2; ++i) {
        if (data[y + i][x - windowRadius] != -INFINITY) {
            float pixelValue = data[y + i][x - windowRadius];
            sumOfSquares += pixelValue * pixelValue;
            sum += pixelValue;
            validPixelCount++;
        }

        if (data[y + i][x + windowRadius] != -INFINITY) {
            float pixelValue = data[y + i][x + windowRadius];
            sumOfSquares += pixelValue * pixelValue;
            sum += pixelValue;
            validPixelCount++;
        }
    }

//lower border
    for (const float* currentPixel = &data[y + windowRadius][x - 2]; currentPixel <= &data[y + windowRadius][x + 2]; ++currentPixel) {
        if (*currentPixel != -INFINITY) {
            int_fast32_t pixelValue = *currentPixel;
            sumOfSquares += pixelValue * pixelValue;
            sum += pixelValue;
            validPixelCount++;
        }
    }

    if (validPixelCount < 2) {
        *mean = INFINITY;
        *sigma = INFINITY;
    } else {
        *mean = (float) sum / validPixelCount;
        float computedSigma = sqrt((float) sumOfSquares / (validPixelCount - 1) - (*mean) * (*mean) * validPixelCount / (float) (validPixelCount - 1));
        *sigma = fmax(computedSigma, accuracyConstants.minimumSigma);
    }
#else
//first save everything, then compute
    float background[20];
    uint_fast8_t validPixelCount = 0;

//compute mean and sigma from border
//upper border
    for (const float* currentPixel = &data[y - windowRadius][x - 2]; currentPixel <= &data[y - windowRadius][x + 2]; ++currentPixel) {
        if (*currentPixel != -INFINITY) {
            background[validPixelCount++] = *currentPixel;
        }
    }

//left and right border
    for (int_fast8_t i = -2; i <= 2; ++i) {
        if (data[y + i][x - windowRadius] != -INFINITY) {
            background[validPixelCount++] = data[y + i][x - windowRadius];
        }

        if (data[y + i][x + windowRadius] != -INFINITY) {
            background[validPixelCount++] = data[y + i][x + windowRadius];
        }
    }

//lower border
    for (const float* currentPixel = &data[y + windowRadius][x - 2]; currentPixel <= &data[y + windowRadius][x + 2]; ++currentPixel) {
        if (*currentPixel != -INFINITY) {
            background[validPixelCount++] = *currentPixel;
        }
    }
#endif

    if (validPixelCount < 2) {
        *mean = INFINITY;
        *sigma = INFINITY;
    } else {
#if ONE_PASS_COMPUTATION_METHOD
#if !COMPUTE_ON_THE_FLY
        double sum = 0;  //can be float, since it can easily be stored in 24bit!
        double sumOfSquares = 0;
        for (uint_fast8_t i = 0; i < validPixelCount; ++i) {
            sum += background[i];
            sumOfSquares += background[i] * background[i];
        }
#endif
        *mean = (float) sum / validPixelCount;
        float computedSigma = sqrt(((float) sumOfSquares - (*mean) * (*mean) * validPixelCount) / (float) (validPixelCount - 1));
        *sigma = std::max(computedSigma, accuracyConstants.minimumSigma);
#else
        //naive computation method
        double sum = 0;
        for (uint_fast8_t i = 0; i < validPixelCount; ++i) {
            sum += background[i];
        }
        *mean = sum / validPixelCount;

        double squaredDeviationSum = 0;
        for (uint_fast8_t i = 0; i < validPixelCount; ++i) {
            double deviation = *mean - background[i];
            squaredDeviationSum += deviation * deviation;
        }
        *sigma = std::max(sqrtf((float) squaredDeviationSum / (validPixelCount - 1)), accuracyConstants.minimumSigma);
#endif
    }
}

static inline void analysePeak(uint_fast16_t x, uint_fast16_t y, float thresholdNeighbourPixel, const float* data_linear,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah, const peakFinder9_accuracyConstants_t& accuracyConstants,
        peakFinder9_intermediatePeakStatistics_t& intermediatePeakStatistics)
{

    const float (*data)[detectorRawSize_cheetah.pix_nx] = (float (*)[detectorRawSize_cheetah.pix_nx]) data_linear;

    intermediatePeakStatistics.totalMass = data[y][x];
    intermediatePeakStatistics.weightedCoordinatesSummed_x = data[y][x] * x;
    intermediatePeakStatistics.weightedCoordinatesSummed_y = data[y][x] * y;
    intermediatePeakStatistics.biggestPixelMass = data[y][x];
    intermediatePeakStatistics.pixelsCount = 1;

    bool newPixelFound = true;
    for (uint_fast8_t radius = 1; newPixelFound && radius < accuracyConstants.windowRadius; ++radius) {
        analyseRingAroundPixel(radius, thresholdNeighbourPixel, x, y, data_linear, detectorRawSize_cheetah, intermediatePeakStatistics, &newPixelFound);
    }

}

static inline void analyseRingAroundPixel(uint_fast8_t radius, float thresholdNeighbourPixel, uint_fast16_t x, uint_fast16_t y, const float* data_linear,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah, peakFinder9_intermediatePeakStatistics_t& intermediatePeakStatistics, bool* newPixelFound)
{

    const float (*data)[detectorRawSize_cheetah.pix_nx] = (float (*)[detectorRawSize_cheetah.pix_nx]) data_linear;

    *newPixelFound = false;
    uint_fast8_t pixelsCount_old = intermediatePeakStatistics.pixelsCount;

//upper border
    uint_fast16_t currentY = y - radius;
    for (int_fast8_t i = -radius; i <= radius; ++i) {
        uint_fast16_t currentX = x + i;
        float currentPixelValue = data[currentY][currentX];
        if (currentPixelValue > thresholdNeighbourPixel) {
            addPixelTointermediatePeakStatistics(intermediatePeakStatistics, currentX, currentY, currentPixelValue);
        }
    }

//left and right border
    for (int_fast8_t i = -(radius - 1); i <= (radius - 1); ++i) {
        uint_fast16_t currentX = x - radius;
        uint_fast16_t currentY = y + i;
        float currentPixelValue = data[currentY][currentX];
        if (currentPixelValue > thresholdNeighbourPixel) {
            addPixelTointermediatePeakStatistics(intermediatePeakStatistics, currentX, currentY, currentPixelValue);
        }

        currentX = x + radius;
        currentY = y + i;
        currentPixelValue = data[currentY][currentX];
        if (currentPixelValue > thresholdNeighbourPixel) {
            addPixelTointermediatePeakStatistics(intermediatePeakStatistics, currentX, currentY, currentPixelValue);
        }
    }

//lower border
    currentY = y + radius;
    for (int_fast8_t i = -radius; i <= radius; ++i) {
        uint_fast16_t currentX = x + i;
        float currentPixelValue = data[currentY][currentX];
        if (currentPixelValue > thresholdNeighbourPixel) {
            addPixelTointermediatePeakStatistics(intermediatePeakStatistics, currentX, currentY, currentPixelValue);
        }
    }

    if (pixelsCount_old != intermediatePeakStatistics.pixelsCount) {
        *newPixelFound = true;
    }
}

static inline void addPixelTointermediatePeakStatistics(peakFinder9_intermediatePeakStatistics_t& intermediatePeakStatistics, uint_fast16_t x,
        uint_fast16_t y, float pixelValue)
{
    intermediatePeakStatistics.totalMass += pixelValue;
    intermediatePeakStatistics.weightedCoordinatesSummed_x += pixelValue * x;
    intermediatePeakStatistics.weightedCoordinatesSummed_y += pixelValue * y;
    ++intermediatePeakStatistics.pixelsCount;
}

static inline void savePeak(float sigmaBackground, float meanBackground, const peakFinder9_intermediatePeakStatistics_t& intermediatePeakStatistics,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah, tPeakList& peakList)
{

    float x = intermediatePeakStatistics.weightedCoordinatesSummed_x / intermediatePeakStatistics.totalMass;
    float y = intermediatePeakStatistics.weightedCoordinatesSummed_y / intermediatePeakStatistics.totalMass;
    float peakMass = intermediatePeakStatistics.totalMass - intermediatePeakStatistics.pixelsCount * meanBackground;

    if (peakList.nPeaks < peakList.nPeaks_max) {
        uint32_t peakCountOld = peakList.nPeaks;

        peakList.peakNpix += intermediatePeakStatistics.pixelsCount;
        peakList.peakTotal += peakMass;
        peakList.peak_npix[peakCountOld] = intermediatePeakStatistics.pixelsCount;
        peakList.peak_com_x[peakCountOld] = x;
        peakList.peak_com_y[peakCountOld] = y;
        peakList.peak_com_index[peakCountOld] = roundf(y) * (detectorRawSize_cheetah.pix_nx) + roundf(x);
        peakList.peak_totalintensity[peakCountOld] = peakMass;
        peakList.peak_maxintensity[peakCountOld] = intermediatePeakStatistics.biggestPixelMass;
        peakList.peak_sigma[peakCountOld] = sigmaBackground;
        peakList.peak_snr[peakCountOld] = peakMass / sigmaBackground;

        ++peakList.nPeaks;
    }
}

