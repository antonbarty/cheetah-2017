/**
 * @file Double_2D.h
 * @class Double_2D
 * @author  Nadia Davidson 
 * @date Last modified on 6/1/2011
 *
 * @brief A 2-dimensional array of doubles
 *
 * This class represents a 2D field of doubles. Setter and getter
 * methods are provided along with some other useful functions. This
 * is a convenient form to store and pass image data in the
 * reconstruction software.  A better implementation of this class
 * would use templates, but restricting the type to double is
 * sufficient our needs.
 */

#ifndef DOUBLE_2D_H
#define DOUBLE_2D_H

#include <iostream>
#include "string.h"

#include <math.h>

#define DOUBLE_PRECISION 1
template <class T>
class Real_2D{

 protected:
  
  /** the size in x */
  int nx;

  /** the size in y */
  int ny;

  /** number of pixels */
  int nn;

 public:

  friend class Complex_2D ; 

  /** the underlying 2-D array */
  T * array;

  /**
   * A constructor which creates an empty array (of no size).  Note
   * that memory has not been allocated if this method is used.
   */
  Real_2D():nx(0),ny(0){
	//std::cout << "Real_2D constructor" << std::endl;
  };
  
  /**
   * Constructor that creates a 2D object with the given dimensions.
   * 
   * @param x_size The number of samplings in the horizontal direction
   * @param y_size The number of samplings in the vertical direction
   */
  Real_2D(int x_size, int y_size){
    allocate_memory(x_size,y_size);
	//std::cout << "alloc mem" << std::endl;
  };
  
  /**
   * Destructor. Memory is deallocated here.
   */
  ~Real_2D(){
    if(nx > 0 ){
      delete [] array;
    }  
    
  };
     
  /**
   * Allocate memory for the array. This should only be used if
   * the constructor was called with no parameters!
   * 
   * @param x_size The number of samplings in the horizontal direction
   * @param y_size The number of samplings in the vertical direction
   */ 
  void allocate_memory(int x_size, int y_size){
    nx = x_size;
    ny = y_size;
	nn = nx * ny;
    array = new T[nx*ny];
    
    for(int i=0; i<nn; i++) {
		array[i]=0; // doesn't matter j+i*ny
	}
  };
  

  /**
   * Copy the contents of another Real_2D array to this one.  Note
   * that this is a quick copy method and no bounds checking is done.
   * 
   * @param double_array The array to copy from
   */
  void copy(const Real_2D<T> & other_array){
    memcpy(array,other_array.array, sizeof(T)*nn);
  };
  
  /**
   * Set the value at positions (x,y) WARNING: no bound checking is
   * done!
   *
   * @param x The horizontal position 
   * @param y The vertical position
   * @param value The value to set
   */
  inline void set(int x, int y, T value){
    array[y+x*ny]=value;
  };


  /**
   * Get the value at positions (x,y) WARNING: no bound checking is
   * done!
   *
   * @param x The horizontal position 
   * @param y The vertical position
   * @return The value at (x,y)
   */
  inline T get(int x, int y) const {
    return array[y+x*ny];
  };

  /**
   * Get the size in x;
   * 
   * @return The number of horizontal points.
   *  
   */
  inline int get_size_x() const {
    return nx;
  };

  /**
   * Get the size in y;
   * 
   * @return The number of vertical points.
   *  
   */
  inline int get_size_y() const {
    return ny;
  };


  /**
   * Get the sum of all values in the array. This is useful to
   * determine normalisation values.
   * 
   * @return The sum of all values in the array
   *  
   */
  T get_sum() const{
    T total = 0;
    for(int i=0; i<nn; i++)
	total+=array[i]; // doesn't matter j+i*ny
    return total;
  };


  /**
   * Get the maximum of all values in the array. 
   * 
   * @return The maximum value in the array
   *  
   */
  T get_max() const{
    if(nx==0||ny==0)
      return 0;
    
    T max = array[0];
    for(int i=0; i<nn; i++){
		if(array[i]>max) 	// doesn't matter j+i*ny
			max = array[i]; 	// doesn't matter j+i*ny
    }
    return max;
    
  };

  /**
   * Get the minimum of all values in the array. 
   * 
   * @return The minimum value in the array 
   */
  T get_min() const {
    if(nx==0||ny==0)
      return 0;

    T min = array[0];
    for(int i=0; i<nn; i++){
		if(array[i]<min)	// doesn't matter j+i*ny
			min = array[i];	// doesn't matter j+i*ny
    }
    return min;
    
  };


  void add(const Real_2D<T> & other_array, double norm=1.0){
    for(int i=0; i< nn; i++)
		array[i]+=norm*other_array.array[i]; // i+j*nx?
  }
  
  void scale(double scale_by){
    for(int i=0; i< nn; i++)
		array[i]*=scale_by;			// doesn't matter j+i*ny
  }


//invert (and scale if we want to).
void invert(bool scale){

  int middle_x = nx/2;
  int middle_y = ny/2;

  double scale_factor = 1;
  if(scale)
    scale_factor = 1.0/(sqrt(nx*ny));
  
  if(nx%2==1 || ny%2==1)
    std::cout << "WARNING: The array dimensions are odd "
	 << "but we have assumed they are even when inverting an "
	 << "array after FFT. This will probably cause you issues..."
	 << std::endl;
  
  for(int i=0; i < nx; ++i){
    for(int j=0; j < middle_y; ++j){
      
	int j_new = j+middle_y; 
	int i_new = i+middle_x; 

	if(i >=  middle_x)
	  i_new = i_new - 2*middle_x;

	double temp_rl = get(i_new,j_new);
	//double temp_im = get_imag(i_new,j_new);

	set(i_new,j_new,get(i,j)*scale_factor);
	//set_imag(i_new,j_new,get_imag(i,j)*scale_factor);

	set(i,j,temp_rl*scale_factor);
	//set_imag(i,j,temp_im*scale_factor);
    }
  }
}

};

#ifndef DOUBLE_PRECISION
typedef Real_2D<float> Double_2D;
#else
typedef Real_2D<double> Double_2D;
#endif


#endif
