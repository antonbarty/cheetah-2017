/*
 * mask.h
 *
 *  Created on: 12.12.2015
 *      Author: Yaro
 */

#ifndef INCLUDE_MASK_H_
#define INCLUDE_MASK_H_

#include <stdint.h>
#include <vector>
#include "detectorGeometry.h"

void mergeMaskIntoData(float * data, const uint8_t * mask, const detectorRawSize_cheetah_t& detectorRawSize_cheetah);
void mergeMaskIntoData(float * data, const std::vector< uint32_t >& sparseMask);

void mergeMaskAndDataIntoDataCopy(const float * data, float * dataCopy, const uint8_t * mask, const detectorRawSize_cheetah_t& detectorRawSize_cheetah);
void mergeMaskAndDataIntoDataCopy(const float * data, float * dataCopy, const std::vector< uint32_t >& sparseMask,
        const detectorRawSize_cheetah_t& detectorRawSize_cheetah);

void mergeInvertedMaskIntoData(float * data, const uint8_t * mask, const detectorRawSize_cheetah_t& detectorRawSize_cheetah);
void mergeInvertedMaskAndDataIntoDataCopy(const float * data, float * dataCopy, const uint8_t * mask, const detectorRawSize_cheetah_t& detectorRawSize_cheetah);

void getMaskFromMergedMaskInData(const float * data, uint8_t * mask, const detectorRawSize_cheetah_t& detectorRawSize_cheetah);

void createSparseMask(const uint8_t * mask, const detectorRawSize_cheetah_t& detectorRawSize_cheetah, std::vector< uint32_t >& sparseMask);

#endif /* INCLUDE_MASK_H_ */
