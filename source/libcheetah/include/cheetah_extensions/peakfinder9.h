/*
 * peakFinder.h
 *
 *  Created on: 12.12.2015
 *      Author: Yaro
 */

#ifndef INCLUDE_PEAKFINDER9_WRAPPER_H_
#define INCLUDE_PEAKFINDER9_WRAPPER_H_

#include <stdint.h>
#include "peakfinders.h"

int peakfinder9(tPeakList *peaklist, float *data, char *mask, long asic_nx, long asic_ny, long nasics_x, long nasics_y, float sigmaFactorBiggestPixel, float sigmaFactorPeakPixel, float sigmaFactorWholePeak, float minimumSigma, float minimumPeakOversizeOverNeighbours, uint_fast8_t windowRadius);

#endif /* INCLUDE_PEAKFINDER9_WRAPPER_H_ */
