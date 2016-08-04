/*
 * pnCcdWorkarounds.cpp
 *
 *  Created on: 20.07.2016
 *      Author: Yaro
 */

#include "pnCcdWorkarounds.h"

void rearrangePnCcdDataForStreakFinder(float* data_rearranged_linear, const float* data_linear)
{
    const float (*data)[1024] = (float (*)[1024]) data_linear;
    float (*data_rearranged)[1024] = (float (*)[1024]) data_rearranged_linear;

    for (uint16_t y_old = 0, x_new = 0; y_old <= 511; ++y_old, ++x_new) {
        for (uint16_t x_old = 0, y_new = 0; x_old <= 511; ++x_old, ++y_new) {
            data_rearranged[y_new][x_new] = data[y_old][x_old];
        }
    }

    for (uint16_t y_old = 512, x_new = 511; y_old <= 1023; ++y_old, --x_new) {
        for (uint16_t x_old = 0, y_new = 512; x_old <= 511; ++x_old, ++y_new) {
            data_rearranged[y_new][x_new] = data[y_old][x_old];
        }
    }

    for (uint16_t y_old = 0, x_new = 512; y_old <= 511; ++y_old, ++x_new) {
        for (uint16_t x_old = 512, y_new = 511; x_old <= 1023; ++x_old, --y_new) {
            data_rearranged[y_new][x_new] = data[y_old][x_old];
        }
    }

    for (uint16_t y_old = 512, x_new = 1023; y_old <= 1023; ++y_old, --x_new) {
        for (uint16_t x_old = 512, y_new = 1023; x_old <= 1023; ++x_old, --y_new) {
            data_rearranged[y_new][x_new] = data[y_old][x_old];
        }
    }
}

void reRearrangePnCcdDataForStreakFinder(float* data_linear, const float* data_rearranged_linear)
{
    float (*data)[1024] = (float (*)[1024]) data_linear;
    const float (*data_rearranged)[1024] = (float (*)[1024]) data_rearranged_linear;

    for (uint16_t y_old = 0, x_new = 0; y_old <= 511; ++y_old, ++x_new) {
        for (uint16_t x_old = 0, y_new = 0; x_old <= 511; ++x_old, ++y_new) {
            data[y_old][x_old] = data_rearranged[y_new][x_new];
        }
    }

    for (uint16_t y_old = 512, x_new = 511; y_old <= 1023; ++y_old, --x_new) {
        for (uint16_t x_old = 0, y_new = 512; x_old <= 511; ++x_old, ++y_new) {
            data[y_old][x_old] = data_rearranged[y_new][x_new];
        }
    }

    for (uint16_t y_old = 0, x_new = 512; y_old <= 511; ++y_old, ++x_new) {
        for (uint16_t x_old = 512, y_new = 511; x_old <= 1023; ++x_old, --y_new) {
            data[y_old][x_old] = data_rearranged[y_new][x_new];
        }
    }

    for (uint16_t y_old = 512, x_new = 1023; y_old <= 1023; ++y_old, --x_new) {
        for (uint16_t x_old = 512, y_new = 1023; x_old <= 1023; ++x_old, --y_new) {
            data[y_old][x_old] = data_rearranged[y_new][x_new];
        }
    }
}

void rearrangePnCcdMaskForStreakFinder(uint8_t* mask_rearranged_linear, const uint8_t* mask_linear)
{
    const uint8_t (*mask)[1024] = (uint8_t (*)[1024]) mask_linear;
    uint8_t (*mask_rearranged)[1024] = (uint8_t (*)[1024]) mask_rearranged_linear;

    for (uint16_t y_old = 0, x_new = 0; y_old <= 511; ++y_old, ++x_new) {
        for (uint16_t x_old = 0, y_new = 0; x_old <= 511; ++x_old, ++y_new) {
            mask_rearranged[y_new][x_new] = mask[y_old][x_old];
        }
    }

    for (uint16_t y_old = 512, x_new = 511; y_old <= 1023; ++y_old, --x_new) {
        for (uint16_t x_old = 0, y_new = 512; x_old <= 511; ++x_old, ++y_new) {
            mask_rearranged[y_new][x_new] = mask[y_old][x_old];
        }
    }

    for (uint16_t y_old = 0, x_new = 512; y_old <= 511; ++y_old, ++x_new) {
        for (uint16_t x_old = 512, y_new = 511; x_old <= 1023; ++x_old, --y_new) {
            mask_rearranged[y_new][x_new] = mask[y_old][x_old];
        }
    }

    for (uint16_t y_old = 512, x_new = 1023; y_old <= 1023; ++y_old, --x_new) {
        for (uint16_t x_old = 512, y_new = 1023; x_old <= 1023; ++x_old, --y_new) {
            mask_rearranged[y_new][x_new] = mask[y_old][x_old];
        }
    }

    //detector boundaries
//    for (uint16_t y = 0; y <= 1023; ++y) {
//        mask_rearranged[y][0] = 1;
//        mask_rearranged[y][511] = 1;
//        mask_rearranged[y][512] = 1;
//        mask_rearranged[y][1023] = 1;
//    }
//    for (uint16_t x = 0; x <= 1023; ++x) {
//        mask_rearranged[0][x] = 1;
//        mask_rearranged[511][x] = 1;
//        mask_rearranged[512][x] = 1;
//        mask_rearranged[1023][x] = 1;
//    }

}

void rearrangePnCcdGeometryForStreakFinder(
        std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions_rearranged,
        const std::vector< std::vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions)
{
    detectorPositions_rearranged = detectorPositions;

    detectorPositions_rearranged[0][0].fs = detectorPositions[0][0].ss;
    detectorPositions_rearranged[0][0].ss = detectorPositions[0][0].fs;

    detectorPositions_rearranged[1][0].fs = -detectorPositions[1][0].ss;
    detectorPositions_rearranged[1][0].ss = detectorPositions[1][0].fs;
    detectorPositions_rearranged[1][0].corner = detectorPositions[1][0].corner + 512 * detectorPositions[1][0].ss;

    detectorPositions_rearranged[0][1].fs = detectorPositions[0][1].ss;
    detectorPositions_rearranged[0][1].ss = -detectorPositions[0][1].fs;
    detectorPositions_rearranged[0][1].corner = detectorPositions[0][1].corner + 512 * detectorPositions[0][1].fs;

//    detectorPositions_rearranged[0][1].fs = -detectorPositions[0][1].ss;
//    detectorPositions_rearranged[0][1].ss = detectorPositions[0][1].fs;
//    detectorPositions_rearranged[0][1].corner = detectorPositions[0][1].corner + 512 * detectorPositions[0][1].ss;
//
//    detectorPositions_rearranged[1][0].fs = detectorPositions[1][0].ss;
//    detectorPositions_rearranged[1][0].ss = -detectorPositions[1][0].fs;
//    detectorPositions_rearranged[1][0].corner = detectorPositions[1][0].corner + 512 * detectorPositions[1][0].fs;

    detectorPositions_rearranged[1][1].fs = -detectorPositions[1][1].ss;
    detectorPositions_rearranged[1][1].ss = -detectorPositions[1][1].fs;
    detectorPositions_rearranged[1][1].corner = detectorPositions[1][1].corner + 512 * detectorPositions[1][1].ss + 512 * detectorPositions[1][1].fs;

    updateVirtualZeroPosition(detectorPositions_rearranged[0][0]);
    updateVirtualZeroPosition(detectorPositions_rearranged[0][1]);
    updateVirtualZeroPosition(detectorPositions_rearranged[1][0]);
    updateVirtualZeroPosition(detectorPositions_rearranged[1][1]);
}
