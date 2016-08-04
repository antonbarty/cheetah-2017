/*
 * matlabLikeFunctions.cpp
 *
 *  Created on: 28.06.2016
 *      Author: Yaro
 */

#include "matlabLikeFunctions.h"

std::vector< float > linspace(float a, float b, uint32_t n)
{
    std::vector< float > result(n);
    float step = (b - a) / (float) (n - 1);

    for (uint32_t i = 0; i <= n - 2; i++) {
        result[i] = a + (float) i * step;
    }
    result.back() = b;

    return result;
}

