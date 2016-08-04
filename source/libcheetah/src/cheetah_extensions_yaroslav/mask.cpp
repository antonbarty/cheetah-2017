/*
 * mask.cpp
 *
 *  Created on: 12.12.2015
 *      Author: Yaro
 */

#include "mask.h"
//#include <math.h>
#include <cmath>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include <boost/foreach.hpp>
#ifdef __CDT_PARSER__
#undef BOOST_FOREACH
#define BOOST_FOREACH(a, b) for(a; ; )
#endif

using namespace std;

void mergeMaskIntoData(float * data, const uint8_t * mask, const detectorRawSize_cheetah_t& detectorRawSize_cheetah)
{
    float* currentDataPixel;
    uint8_t* currentMaskPixel;

    for (currentDataPixel = data, currentMaskPixel = (uint8_t *) mask; currentDataPixel < data + detectorRawSize_cheetah.pix_nn;
            ++currentDataPixel, ++currentMaskPixel) {
        if (*currentMaskPixel != 0) {
            *currentDataPixel = -INFINITY;
        }
    }
}

void mergeInvertedMaskIntoData(float * data, const uint8_t * mask, const detectorRawSize_cheetah_t& detectorRawSize_cheetah)
{
    float* currentDataPixel;
    uint8_t* currentMaskPixel;

    for (currentDataPixel = data, currentMaskPixel = (uint8_t *) mask; currentDataPixel < data + detectorRawSize_cheetah.pix_nn;
            ++currentDataPixel, ++currentMaskPixel) {
        if (*currentMaskPixel == 0) {
            *currentDataPixel = -INFINITY;
        }
    }
}

void mergeMaskIntoData(float * data, const std::vector< uint32_t >& sparseMask)
{
    BOOST_FOREACH (const uint32_t & it , sparseMask)
    {
        data[it] = -INFINITY;
    }
}

void mergeMaskAndDataIntoDataCopy(const float * data, float * dataCopy, const uint8_t * mask, const detectorRawSize_cheetah_t& detectorRawSize_cheetah)
{
    const float *currentDataPixel;
    float *currentDataCopyPixel;
    uint8_t* currentMaskPixel;

    for (currentDataPixel = data, currentMaskPixel = (uint8_t *) mask, currentDataCopyPixel = dataCopy;
            currentDataPixel < data + detectorRawSize_cheetah.pix_nn;
            ++currentDataPixel, ++currentMaskPixel, ++currentDataCopyPixel) {
        if (*currentMaskPixel == 0) {
            *currentDataCopyPixel = *currentDataPixel;
        } else {
            *currentDataCopyPixel = -INFINITY;
        }
    }
}

void mergeMaskAndDataIntoDataCopy(const float * data, float * dataCopy, const std::vector< uint32_t >& sparseMask,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah)
{
    copy(data, data + detectorRawSize_cheetah.pix_nn, dataCopy);
    mergeMaskIntoData(dataCopy, sparseMask);
}

void mergeInvertedMaskAndDataIntoDataCopy(const float * data, float * dataCopy, const uint8_t * mask, const detectorRawSize_cheetah_t& detectorRawSize_cheetah)
{
    const float *currentDataPixel;
    float *currentDataCopyPixel;
    uint8_t* currentMaskPixel;

    for (currentDataPixel = data, currentMaskPixel = (uint8_t *) mask, currentDataCopyPixel = dataCopy;
            currentDataPixel < data + detectorRawSize_cheetah.pix_nn;
            ++currentDataPixel, ++currentMaskPixel, ++currentDataCopyPixel) {
        if (*currentMaskPixel == 0) {
            *currentDataCopyPixel = -INFINITY;
        } else {
            *currentDataCopyPixel = *currentDataPixel;
        }
    }
}

void getMaskFromMergedMaskInData(const float * data, uint8_t * mask, const detectorRawSize_cheetah_t& detectorRawSize_cheetah)
{
    uint32_t pixelCount = detectorRawSize_cheetah.pix_nn;

    for (uint32_t i = 0; i < pixelCount; ++i) {
        if (isfinite(data[i])) {
            mask[i] = 0;
        } else {
            mask[i] = 1;
        }
    }
}

void createSparseMask(const uint8_t * mask, const detectorRawSize_cheetah_t& detectorRawSize_cheetah, std::vector< uint32_t >& sparseMask)
{
    sparseMask.clear();

    for (uint32_t i = 0; i < detectorRawSize_cheetah.pix_nn; ++i) {
        if (mask[i] != 0) {
            sparseMask.push_back(i);
        }
    }
}
