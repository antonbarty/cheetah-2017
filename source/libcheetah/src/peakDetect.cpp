/*
 *  peakDetect.cpp
 *	--------------
 *	Created by Jonas Sellberg on 2014-10-27.
 *	Copyright 2014 Uppsala University. All rights reserved.
 *
 *  Object-oriented C++ version of the functionality available 
 *	in the MATLAB script at http://billauer.co.il/peakdet.html
 *	----------------------------------------------------------
 *	This file implements the PeakDetect class, which relies on
 *  the Point and PointVector classes, also included, adapted
 *  from lecture notes of the course CS106B at Stanford University.
 */

#include "peakDetect.h"

#include <iostream>

using namespace std;

/*
 *	-------------------------------------
 *	| Point Constructors and Destructor |
 *	-------------------------------------
 */

Point::Point(int x, int y) {
    this->x = x;
    this->y = y;
}

Point::~Point() {
    /* Empty */
}


/*
 *	-----------------
 *	| Point Methods |
 *	-----------------
 */

int Point::getX() {
    return x;
}

int Point::getY() {
    return y;
}


/*
 *	-------------------------
 *	| PointVector Constants |
 *	-------------------------
 */

/*
 * Implementation notes: INITIAL_CAPACITY
 *	-------------------------------------
 * This constant defines the initial allocated size of the points
 * array used by PointVector. If the vector grows beyond its capacity,
 * the implementation doubles the allocated size.
 */

const unsigned INITIAL_CAPACITY = 100;


/*
 *	-------------------------------------------
 *	| PointVector Constructors and Destructor |
 *	-------------------------------------------
 */

/*
 * Implementation notes: PointVector constuctor
 * ------------------------------------------
 * The constructor must allocate the array storage for the vector
 * points and initialize the fields of the object.
 */

PointVector::PointVector() {
    capacity = INITIAL_CAPACITY;
    points = new Point *[capacity];
    for (unsigned i = 0; i < capacity; i++) {
        points[i] = NULL;
    }
    count = 0;
}


/*
 * Implementation notes: ~PointVector
 * --------------------------------
 * The destructor must deallocate every point (which it can do by
 * calling clear) and then free the dynamic array.
 */

PointVector::~PointVector() {
    clear();
    delete[] points;
}


/*
 *	-----------------------
 *	| PointVector Methods |
 *	-----------------------
 */

/*
 * Implementation notes: size, isEmpty
 * -----------------------------------
 * These implementations should be self-explanatory.
 */

unsigned PointVector::size() {
    return count;
}

bool PointVector::isEmpty() {
    return count;
}


/*
 * Implementation notes: clear
 * ---------------------------
 * This method deallocates every point that has been allocated
 * in the dynamic array.
 */

void PointVector::clear() {
    for (unsigned i = 0; i < count; i++) {
        delete points[i];
        points[i] = NULL;
    }
    count = 0;
}


/*
 * Implementation notes: add
 * --------------------------
 * This function must first check to see whether there is
 * enough room for the point and expand the array storage
 * if necessary. If safe, it adds a new Point to the heap
 * or copies the address of an already existing Point.
 */

void PointVector::add(int x, int y) {
    if (count == capacity) expandCapacity();
    points[count++] = new Point(x, y);
}

void PointVector::add(Point *point) {
    if (count == capacity) expandCapacity();
    points[count++] = point;
}


/*
 * Implementation notes: get
 * -------------------------------
 * These functions must check for an empty vector and report
 * NULL if there is no point at the vector index specified.
 */

Point *PointVector::get() {
    if (isEmpty()) return NULL;
    return points[count - 1];
}

Point *PointVector::get(unsigned index) {
    if (index > count - 1) return NULL;
    return points[index];
}


/*
 * Implementation notes: expandCapacity
 * ------------------------------------
 * This private method doubles the capacity of the points array
 * whenever it runs out of space.  To do so, it must allocate a new
 * array, copy all the points from the old array to the new one,
 * free the old storage, and finally replace the points pointer
 * with the new array.
 */

void PointVector::expandCapacity() {
    capacity *= 2;
    Point **array = new Point *[capacity];
    for (unsigned i = 0; i < capacity; i++) {
        if (i < count) array[i] = points[i];
        else array[i] = NULL;
    }
    delete[] points;
    points = array;
}


/*
 *	------------------------------------------
 *	| PeakDetect Constructors and Destructor |
 *	------------------------------------------
 */

/*
 * Implementation notes: PeakDetect constuctors
 * --------------------------------------------
 * The constructor must allocate the storage for the point
 * vectors of the maxima and minima and initialize the pointers
 * to the data values.
 */

PeakDetect::PeakDetect(uint16_t *Yarray, unsigned length) {
    x = NULL;
    y = Yarray;
    maxima = new PointVector();
    minima = new PointVector();
    this->length = length;
    index = 0;
}

PeakDetect::PeakDetect(int *Xarray, uint16_t *Yarray, unsigned length) {
    x = Xarray;
    y = Yarray;
    maxima = new PointVector();
    minima = new PointVector();
    this->length = length;
    index = 0;
}


/*
 * Implementation notes: ~PeakDetect
 * ---------------------------------
 * The destructor must deallocate the two point vectors
 */

PeakDetect::~PeakDetect() {
    delete maxima;
    delete minima;
}


/*
 *	----------------------
 *	| PeakDetect Methods |
 *	----------------------
 */

/*
 * Implementation notes: clear
 * ---------------------------
 * This method clears all maxima and minima that has been found
 * and resets all instance variables.
 */

void PeakDetect::clear() {
    maxima->clear();
    minima->clear();
    index = 0;
}


/*
 * Implementation notes: findAll
 * -----------------------------
 * This method finds all maxima and minima that have a difference larger than delta.
 * It starts looking for a minima and adds an additional minima after the last maxima.
 */

void PeakDetect::findAll(float delta) {
    
    clear();
    
    Point max(-1, -1);
    Point min(-1, 65536);
    bool findMax = false;
    
    for (index = 0; index < length; index++) {
        // check if max/min should be updated
        if (y[index] > max.getY()) max = Point(index, y[index]);
        if (minima->size()) {
            if (y[index] < min.getY()) min = Point(index, y[index]);
        } else {
            if (y[index] <= min.getY()) min = Point(index, y[index]);
        }
        // check if max/min are larger than delta
        if (findMax) {
            if (y[index] < max.getY() - delta) {
                maxima->add(max.getX(), max.getY());
                min = Point(index, y[index]);
                findMax = false;
            }
        } else {
            if (y[index] > min.getY() + delta) {
                minima->add(min.getX(), min.getY());
                max = Point(index, y[index]);
                findMax = true;
            }
        }
    }
    // add last minimum
    minima->add(min.getX(), min.getY());
    
    if (x != NULL) {
        
        PointVector *maxtemp = new PointVector();
        PointVector *mintemp = new PointVector();
        
        for (unsigned i = 0; i < maxima->size(); i++)
            maxtemp->add(x[(maxima->get(i))->getX()], (maxima->get(i))->getY());
        for (unsigned i = 0; i < minima->size(); i++)
            mintemp->add(x[(minima->get(i))->getX()], (minima->get(i))->getY());
        
        delete maxima;
        delete minima;
        maxima = maxtemp;
        minima = mintemp;
        
    }
}


/*
 * Implementation notes: findNext
 * ------------------------------
 * This function
 */

void PeakDetect::findNext(float delta) {
    
    Point max(-1, -1);
    Point min(-1, 65536);
    bool findMax = true;
    int maximaSize = 0;
    int minimaSize = 1;
    
    if (index) {
        max = Point(index, y[index]);
        min = max;
        maximaSize = maxima->size();
        minimaSize = minima->size();
    }
    
    while (maxima->size() - maximaSize < 1 || minima->size() - minimaSize < 1) {
        
        if (index >= length) break;
        // check if max/min should be updated
        if (y[index] > max.getY()) {
            if (x == NULL) max = Point(index, y[index]);
            else max = Point(x[index], y[index]);
        }
        if (y[index] < min.getY()) {
            if (x == NULL) min = Point(index, y[index]);
            else min = Point(x[index], y[index]);
        }
        // check if max/min are larger than delta
        if (findMax) {
            if (y[index] < max.getY() - delta) {
                maxima->add(max.getX(), max.getY());
                // find first minimum and add it
                if (maxima->size() == 1) {
                    int jmax;
                    if (x == NULL) jmax = max.getX();
                    else {
                        int k = 0;
                        while (true) {
                            if (x[k] == max.getX()) {
                                jmax = k;
                                break;
                            }
                            k++;
                        }
                    }
                    min = Point(-1, 65536);
                    for (int j = 0; j < jmax; j++) {
                        if (y[j] <= min.getY()) {
                            if (x == NULL) min = Point(j, y[j]);
                            else min = Point(x[index], y[index]);
                        }
                    }
                    minima->add(min.getX(), min.getY());
                }
                if (x == NULL) min = Point(index, y[index]);
                else min = Point(x[index], y[index]);
                findMax = false;
            }
        } else {
            if (y[index] > min.getY() + delta) {
                minima->add(min.getX(), min.getY());
                if (x == NULL) max = Point(index, y[index]);
                else max = Point(x[index], y[index]);
                findMax = true;
            }
        }
        if (++index == length) minima->add(min.getX(), min.getY());
    }
    
}
