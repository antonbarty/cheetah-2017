/*
 * point.h
 *
 *  Created on: 19.12.2015
 *      Author: Yaro
 *      Taken from: http://stackoverflow.com/questions/11891192/c-templates-simple-point-class and extended
 */

#ifndef INCLUDE_POINT_H_
#define INCLUDE_POINT_H_

#include <ostream>
//#include <math.h>
#include <cmath>
#include <stdint.h>
#include <boost/type_traits/common_type.hpp>
#include <Eigen/Dense>
#include <boost/math/special_functions/round.hpp>

#define STATIC_ASSERT( e ) static_assert( e, "!(" #e ")" )

template< typename T >
class Point
{
private:
    T data[2];

public:

    Point()
    {
        data[0] = 0;
        data[1] = 0;
    }

    Point(T x, T y)
    {
        this->data[0] = x;
        this->data[1] = y;
    }

    template< typename Derived >
    Point(const Eigen::EigenBase< Derived >& matrix)
    {
        Eigen::Matrix< typename Derived::Scalar, 2, 1 > tmp = matrix;
        data[0] = tmp(0);
        data[1] = tmp(1);
    }

    Point(Point const& copy)
    {
        data[0] = static_cast< T >(copy.data[0]);
        data[1] = static_cast< T >(copy.data[1]);
    }

    template< typename newTypeT >
    Point& operator=(const Point< newTypeT > &rhs)
    {
        data[0] = static_cast< newTypeT >(rhs.getX());
        data[1] = static_cast< newTypeT >(rhs.getY());

        return *this;
    }

    //casting
    template< typename newTypeT >
    operator Point< newTypeT >() const
    {
        return Point< newTypeT >(static_cast< T >(data[0]), static_cast< T >(data[1]));
    }

    const T& getX(void) const
            {
        return data[0];
    }
    const T& getY(void) const
            {
        return data[1];
    }

    void setX(T value)
    {
        data[0] = value;
    }
    void setY(T value)
    {
        data[1] = value;
    }

    template< typename RHS >
    void operator+=(Point< RHS > const& rhs)
    {
        data[0] += rhs.getX();
        data[1] += rhs.getY();
    }

    template< typename RHS >
    friend Point< typename boost::common_type< T, RHS >::type > operator+(Point< T > const&a, Point< RHS > const& b)
    {
        typedef typename boost::common_type< T, RHS >::type currentCommonType;

        Point< currentCommonType > ret(a);
        ret += b;
        return ret;
    }

    template< typename RHS >
    void operator-=(Point< RHS > const& rhs)
    {
        data[0] -= rhs.getX();
        data[1] -= rhs.getY();
    }

    template< typename RHS >
    friend Point< typename boost::common_type< T, RHS >::type > operator-(Point< T > const&a, Point< RHS > const& b)
    {
        typedef typename boost::common_type< T, RHS >::type currentCommonType;

        Point< currentCommonType > ret(a);
        ret -= b;
        return ret;
    }
//should not be used, is slow! *= is better (not yet implemented)
    friend Point operator*(const T constant, const Point & myPoint)
    {
        Point ret(myPoint);

        ret.data[0] *= constant;
        ret.data[1] *= constant;

        return ret;
    }

    friend Point operator*(const Point & myPoint, const T constant)
    {
        return constant * myPoint;
    }

    friend bool operator>(const Point & a, const Point &b)
    {
        if (a.getX() > b.getX() && a.getY() > b.getY()) {
            return true;
        } else {
            return false;
        }
    }

    friend bool operator>=(const Point & a, const Point &b)
    {
        if (a.getX() >= b.getX() && a.getY() >= b.getY()) {
            return true;
        } else {
            return false;
        }
    }

    friend bool operator<(const Point & a, const Point &b)
    {
        if (a.getX() < b.getX() && a.getY() < b.getY()) {
            return true;
        } else {
            return false;
        }
    }
    friend bool operator<=(const Point & a, const Point &b)
    {
        if (a.getX() <= b.getX() && a.getY() <= b.getY()) {
            return true;
        } else {
            return false;
        }
    }

    void round()
    {
        data[0] = boost::math::round(data[0]);
        data[1] = boost::math::round(data[1]);
    }

    Point getRounded() const
    {
        Point ret(*this);
        ret.round();
        return ret;
    }

    T* getData() const
    {
        return (T*) data;
    }

    friend std::ostream& operator<<(std::ostream &out, const Point &myPoint)
    {
        out << "(" << myPoint.data[0] << "," << myPoint.data[1] << ")";
        return out;
    }

};

#endif /* INCLUDE_POINT_H_ */
