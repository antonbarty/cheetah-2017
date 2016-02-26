/*
 * cheetahConversion.cpp
 *
 *  Created on: 22.12.2015
 *      Author: Yaro
 */

#include "cheetahConversion.h"

using namespace Eigen;

void cheetahGetDetectorGeometryMatrix(const float* pix_x, const float* pix_y, const detectorRawSize_cheetah_t detectorRawSize_cheetah,
        Vector2f** detectorGeometryMatrix)
{
    uint32_t imageSize_x = detectorRawSize_cheetah.asic_nx * detectorRawSize_cheetah.nasics_x;
    uint32_t imageSize_y = detectorRawSize_cheetah.asic_ny * detectorRawSize_cheetah.nasics_y;

    *detectorGeometryMatrix = new Vector2f[imageSize_x * imageSize_y];
    for (uint32_t i = 0; i < imageSize_x * imageSize_y; ++i) {
        float x = pix_x[i];
        float y = pix_y[i];

        (*detectorGeometryMatrix)[i] = Vector2f(x, y);
    }
}

void cheetahDeleteDetectorGeometryMatrix(Vector2f* detectorGeometryMatrix)
{
    delete[] detectorGeometryMatrix;
}
