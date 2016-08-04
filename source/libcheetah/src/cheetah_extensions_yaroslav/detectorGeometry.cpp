/*
 * detectorGeometry.cpp
 *
 *  Created on: 12.12.2015
 *      Author: Yaro
 */

#include "detectorGeometry.h"
//#include <math.h>
#include <cmath>

#include <boost/foreach.hpp>
#ifdef __CDT_PARSER__
#undef BOOST_FOREACH
#define BOOST_FOREACH(a, b) for(a; ; )
#endif

using namespace std;
using namespace Eigen;

void updateVirtualZeroPosition(detectorPosition_t& detectorPositions);

void computeDetectorPositionsFromDetectorGeometryMatrix(
        vector< vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > >& detectorPositions,
        const detectorRawSize_cheetah_t detectorRawSize_cheetah, const Vector2f* detectorGeometryMatrix_linear)
{
    detectorPositions.resize(detectorRawSize_cheetah.nasics_x);
    typedef vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > element_t;
    BOOST_FOREACH (element_t & element , detectorPositions )
    {
        element.resize(detectorRawSize_cheetah.nasics_y);
    }

    const Vector2f (*detectorGeometryMatrix)[detectorRawSize_cheetah.pix_nx] =
            (const Vector2f (*)[detectorRawSize_cheetah.pix_nx]) detectorGeometryMatrix_linear;

    for (uint16_t asic_y = 0; asic_y < detectorRawSize_cheetah.nasics_y; ++asic_y) {
        for (uint16_t asic_x = 0; asic_x < detectorRawSize_cheetah.nasics_x; ++asic_x) {
            uint16_t min_fs = asic_x * detectorRawSize_cheetah.asic_nx;
            uint16_t min_ss = asic_y * detectorRawSize_cheetah.asic_ny;
            uint16_t max_fs = (asic_x + 1) * detectorRawSize_cheetah.asic_nx - 1;
            uint16_t max_ss = (asic_y + 1) * detectorRawSize_cheetah.asic_ny - 1;

            Vector2f fs = detectorGeometryMatrix[min_ss][min_fs + 1] - detectorGeometryMatrix[min_ss][min_fs];
            Vector2f ss = detectorGeometryMatrix[min_ss + 1][min_fs] - detectorGeometryMatrix[min_ss][min_fs];
            Vector2f corner = detectorGeometryMatrix[min_ss][min_fs] - 0.5 * fs - 0.5 * ss;

            detectorPositions[asic_y][asic_x].min_fs = min_fs;
            detectorPositions[asic_y][asic_x].min_ss = min_ss;
            detectorPositions[asic_y][asic_x].max_fs = max_fs;
            detectorPositions[asic_y][asic_x].max_ss = max_ss;
            detectorPositions[asic_y][asic_x].fs = fs;
            detectorPositions[asic_y][asic_x].ss = ss;
            detectorPositions[asic_y][asic_x].corner = corner;

            detectorPositions[asic_y][asic_x].rawCoordinates_uint16 = ImageRectangle < uint16_t > (Point2D < uint16_t > (min_fs, min_ss),
                    Point2D < uint16_t > (max_fs, max_ss));
            detectorPositions[asic_y][asic_x].rawCoordinates_float = ImageRectangle< float >(Point2D< float >(min_fs, min_ss),
                    Point2D< float >(max_fs, max_ss));

            updateVirtualZeroPosition(detectorPositions[asic_y][asic_x]);
        }
    }
}

void updateVirtualZeroPosition(detectorPosition_t& detectorPositions)
{
    float numerator = detectorPositions.fs.dot(detectorPositions.corner * (-1));
    float denominator = detectorPositions.fs.norm() * detectorPositions.corner.norm();
    float angleSsVectorToZero = acosf(numerator / denominator);

//    rotationMatrix = [ cos(angleSsVectorToZero) -sin(angleSsVectorToZero); sin(angleSsVectorToZero) cos(angleSsVectorToZero) ];
//    virtualZeroPosition = upperLeftCornerRaw + rotationMatrix*[distanceUpperLeftCornerAlignedToZero ; 0];

    detectorPositions.virtualZeroPositionRaw = Map< const Vector2f >(detectorPositions.rawCoordinates_float.getUpperLeftCorner().getData())
            + Vector2f(cos(angleSsVectorToZero), sin(angleSsVectorToZero)) * detectorPositions.corner.norm();
}

