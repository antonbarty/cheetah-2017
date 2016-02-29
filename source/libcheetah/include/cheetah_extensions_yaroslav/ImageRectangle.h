/*
 * ImageRectangle.h
 *
 *  Created on: 25.12.2015
 *      Author: Yaro
 */

#ifndef INCLUDE_IMAGERECTANGLE_H_
#define INCLUDE_IMAGERECTANGLE_H_

#include "Point.h"

template< typename T >
class ImageRectangle {
private:
    Point< T > upperLeftCorner, lowerRightCorner;

public:
    ImageRectangle() :
            upperLeftCorner(), lowerRightCorner()
    {
    }

    ImageRectangle(Point< T > upperLeftCorner, Point< T > lowerRightCorner) :
            upperLeftCorner(upperLeftCorner), lowerRightCorner(lowerRightCorner)
    {
    }

    ImageRectangle(Point< T > upperLeftCorner, T width, T height) :
            upperLeftCorner(upperLeftCorner)
    {
        lowerRightCorner.setX(upperLeftCorner.getX() + width - 1);
        lowerRightCorner.setY(upperLeftCorner.getY() + height - 1);
    }

    const Point< T >& getLowerRightCorner() const
    {
        return lowerRightCorner;
    }

    void setLowerRightCorner(const Point< T >& lowerRightCorner)
    {
        this->lowerRightCorner = lowerRightCorner;
    }

    const Point< T >& getUpperLeftCorner() const
    {
        return upperLeftCorner;
    }

    void setUpperLeftCorner(const Point< T >& upperLeftCorner)
    {
        this->upperLeftCorner = upperLeftCorner;
    }

    template< typename someType >
    bool contains(const Point< someType >& pointToTest) const
            {
        if (pointToTest >= upperLeftCorner && pointToTest <= lowerRightCorner) {
            return true;
        } else {
            return false;
        }
    }

    //casting
    template< typename newTypeT >
    operator ImageRectangle< newTypeT >()
    {
        ImageRectangle< newTypeT > result(upperLeftCorner, lowerRightCorner);
        return result;
    }

    template< typename newTypeT >
    ImageRectangle(const ImageRectangle< newTypeT > &rhs)
    {
        upperLeftCorner = rhs.getUpperLeftCorner();
        lowerRightCorner = rhs.getLowerRightCorner();
    }

    template< typename newTypeT >
    ImageRectangle& operator=(const ImageRectangle< newTypeT > &rhs)
    {
        upperLeftCorner = rhs.getUpperLeftCorner();
        lowerRightCorner = rhs.getLowerRightCorner();
        return *this;
    }
};

#endif /* INCLUDE_IMAGERECTANGLE_H_ */
