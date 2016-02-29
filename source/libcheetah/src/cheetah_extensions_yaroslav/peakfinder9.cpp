/*
 * peakfinder9.cpp
 *
 *  Created on: 06.01.2015
 *      Author: Valerio
 */

#include "peakfinder9.h"
#include "peakFinder.h"
#include "mask.h"

using namespace std;

int peakfinder9(tPeakList *peaklist, float *data, char *mask, long asic_nx, long asic_ny, long nasics_x, long nasics_y, float sigmaFactorBiggestPixel, float sigmaFactorPeakPixel, float sigmaFactorWholePeak, float minimumSigma, float minimumPeakOversizeOverNeighbours, uint_fast8_t windowRadius)
{

    peakFinder9_accuracyConstants_t pf9ac;
    pf9ac.sigmaFactorBiggestPixel = sigmaFactorBiggestPixel;
    pf9ac.sigmaFactorPeakPixel = sigmaFactorPeakPixel;
    pf9ac.sigmaFactorWholePeak = sigmaFactorWholePeak;
    pf9ac.minimumSigma = minimumSigma;
    pf9ac.minimumPeakOversizeOverNeighbours = minimumPeakOversizeOverNeighbours;
    pf9ac.windowRadius = windowRadius;

    detectorRawSize_cheetah_t drsc;
    drsc.asic_nx = asic_nx;
    drsc.asic_ny = asic_ny;
    drsc.nasics_x = nasics_x;
    drsc.nasics_y = nasics_y;

    drsc.pix_nx = asic_nx * nasics_x;
    drsc.pix_ny = asic_ny * nasics_y;
    drsc.pix_nn = asic_nx * nasics_x * asic_ny * nasics_y;

    float *copy = (float*) malloc(drsc.pix_nn*sizeof(float));
    uint8_t * cast_mask = (uint8_t*) (mask);

    mergeInvertedMaskAndDataIntoDataCopy(data, copy, cast_mask, drsc);
    return peakFinder9(copy, pf9ac, drsc, *peaklist);
}
