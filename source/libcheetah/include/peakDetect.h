/*
 *  peakDetect.h
 *	------------
 *	Created by Jonas Sellberg on 2014-10-27.
 *	Copyright 2014 Uppsala University. All rights reserved.
 *
 *  Object-oriented C++ version of the functionality available 
 *	in the MATLAB script at http://billauer.co.il/peakdet.html
 *	----------------------------------------------------------
 *	This interface defines the PeakDetect class, which implements
 *	peak detection for C-arrays. It relies on the Point and PointVector
 *  classes, whose interfaces are also included, that are simple classes
 *  that combines the coordinates of a point into a single object and
 *  implements a dynamic array of points made of integers, respectively.
 *  The Point and PointVector classes are adapted from lecture notes of
 *  the course CS106B at Stanford University.
 */

#ifndef _peakdetect_h
#define _peakdetect_h

#include <stdint.h>

/*
 *	---------------
 *	| Point Class |
 *	---------------
 */

class Point {
  
public:
  
  /*
   * Constructor: Point(x, y)
   * ------------------------
   * Creates a new Point object with the indicated values.
   */
  
  Point(int x, int y);
  
  
  /*
   * Destructor: ~Point()
   * --------------------
   * Called when this Point is deleted or goes out of scope.
   */
  
  ~Point();
  
  
  /*
   * Methods: getX(), getY()
   * -----------------------
   * These methods return the appropriate component.
   */
  
  int getX();
  int getY();
		
private:
  
  /* Data required to implement a point */
  int x, y;
  
};


/*
 *	---------------------
 *	| PointVector Class |
 *	---------------------
 */

/*
 * Class: PointVector
 * -----------------
 * This interface defines a class that models a vector of points.
 * The points are implemented by the Point class.
 * The fundamental point operations are add and get.
 */

class PointVector {
  
public:
  
  /*
   * Constructor: PointVector
   * Usage: PointVector pa;
   * ----------------------
   * Initializes a new empty vector that can contain points.
   */
  
  PointVector();
  
  
  /*
   * Destructor: ~PointVector
   * Usage: (usually implicit)
   * -------------------------
   * Deallocates storage associated with this pa.  This method is
   * called whenever a PointVector instance variable is deallocated.
   */
  
  ~PointVector();
  
  
  /*
   * Method: size
   * Usage: unsigned nElems = pa.size();
   * --------------------------------
   * Returns the number of points in this vector.
   */
  
  unsigned size();
  
  
  /*
   * Method: isEmpty
   * Usage: if (pa.isEmpty()) . . .
   * --------------------------------
   * Returns true if this vector contains no points, and false otherwise.
   */
  
  bool isEmpty();
  
  
  /*
   * Method: clear
   * Usage: pa.clear();
   * --------------------
   * This method removes all points from this vector.
   */
  
  void clear();
  
  
  /*
   * Method: add
   * Usage: pa.add(x, y);
   * ---------------------
   * Adds the point defined by x and y onto this vector.
   * ---------------------------------------------------
   * Usage: pa.add(mypoint);
   * ------------------------
   * Adds the point defined by the pointer mypoint onto this vector.
   */
  
  void add(int x, int y);
  void add(Point *point);
  
  
  /*
   * Method: get
   * Usage: Point *p = pa.get();
   * -----------------------------
   * Returns the pointer to the last point from this vector.
   * -------------------------------------------------------
   * Usage: Point *p = pa.get(index);
   * ---------------------------------
   * Returns the pointer to the point specified by index from this vector.
   */
  
  Point *get();
  Point *get(unsigned index);
  
private:
  
  /* Data required to implement a vector of points */
  
  Point **points;		/* Dynamic array of pointers to the points */
  unsigned capacity;  /* Allocated size of the array */
  unsigned count;     /* Number of points in the vector */
  
  
  /* Private method prototypes */
  
  void expandCapacity();
  
};


/*
 *	--------------------
 *	| PeakDetect Class |
 *	--------------------
 */

/*
 * Class: PeakDetect
 * -----------------
 * This interface defines a class that detects peaks in a C-array.
 * Characters are added and removed only from the top of the stack.
 * The fundamental stack operations are push (add to top) and pop
 * (remove from top).
 */

class PeakDetect {
	
public:
	
	/*
	 * Constructor: PeakDetect
	 * -----------------------
	 * Initializes a new PeakDetect from a C-array of unsigned 16-bit integers and its length.
	 * The C-array contains the Y-values for which the peak detection is performed.
	 * Assumes the corresponding X-value of each Y-value is determined by its index in the array.
	 */
	
	PeakDetect(uint16_t *Yarray, unsigned length);
	
  
	/*
	 * Constructor: PeakDetect
	 * -----------------------
	 * Initializes a new PeakDetect from two C-arrays of unsigned 16-bit integers and their length.
	 * The first C-array contains X-values.
	 * The second C-array contains Y-values.
	 * Prerequisites: Both C-arrays must have the same length!
	 */
	
	PeakDetect(int *Xarray, uint16_t *Yarray, unsigned length);
	
  
	/*
	 * Destructor: ~PeakDetect
	 * -----------------------
	 * Deallocates storage associated with this instance of PeakDetect.  This method is
	 * called whenever a PeakDetect instance variable is deallocated.
	 */
	
	~PeakDetect();
	
  
	/*
	 * Method: clear
	 * --------------------
	 * This method removes all maxima and minima from this PeakDetect and resets all instance variables.
	 */
	
	void clear();
	
  
	/*
	 * Method: findAll
	 * --------------------------------
	 * Finds all the peaks contained in the allocated Y-values whose heights are larger than delta.
	 */
	
	void findAll(float delta);
	
  
	/*
	 * Method: findNext
	 * --------------------------------
	 * Finds the next peak contained in the allocated Y-values whose height is larger than delta.
	 */
	
	void findNext(float delta);
	
  
	/* Public objects */
	
	PointVector *maxima;    // Pointer to a dynamic array of points holding the maxima
	PointVector *minima;    // Pointer to a dynamic array of points holding the minima
	
private:
	
	/* Data required to implement the PeakDetect class*/
	
	int *x;					// Pointer to array of X-values
	uint16_t *y;			// Pointer to array of Y-values
	unsigned length;		// Length of arrays of X- and Y-values
	unsigned index;			// Index for arrays of X- and Y-values
	
  
	/* Private method prototypes */
		
};

#endif
