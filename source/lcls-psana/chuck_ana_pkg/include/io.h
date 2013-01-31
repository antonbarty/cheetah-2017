#ifndef IO_H
#define IO_H

#include <stdlib.h>
#include <cstring>
#include <cmath>
#include "Double_2D.h"

class Complex_2D;
//class Double_2D;

#define FAILURE 0

using namespace std;

/**
 * Read a ppm file. Returns a 2D of the data. 
 *
 * @param file_name The name of the file to read from
 * @param data The array to be filled with data
 */
//int read_ppm(string file_name, Double_2D & data);

/**
 * Read a tiff file. Returns a 2D of the data.
 * 
 * @param file_name The name of the file to read from
 * @param data The array to be filled with data
 */
int read_tiff(string file_name, Double_2D & data);

/**
 * Read a HDF4 file. Returns a 2D of the data. 
 *
 * @param file_name The name of the file to read from
 * @param data The array to be filled with data
 * @param data_name The name of the block in the HDF4 where the data is.
 * By default it looks for the "data" block.
 */
//int read_hdf4(string file_name, Double_2D & data, 
//	      char * data_name="data");



//int read_dbin(string file_name, int nx, int ny, Double_2D & data);

//int read_cplx(string file_name, Complex_2D & complex);



/** 
 * Write a 2D array to a ppm file. The data will be saved as a 16 bit
 * grey-scale image. Please note that the data will be scaled to fit
 * within the range 0 - 2^16. Hence this method is only useful for
 * viewing the final output and should not be used if you plan to
 * reopen the file at a later date and pass the data back to the
 * reconstruction algorithm.
 *
 * @param file_name The name of the file to write to
 * @param data The array to be written to file
 * @param log_scale Output on log scale? true/false. Default is false.
 */ 
//int write_ppm(string file_name, const Double_2D & data, 
//	      bool log_scale=false,
//	      double min=0, double max=0);

/** 
 * Write a 2D array into a binary file. The data will be saved in 64
 * bit format. Because of their high precision and because they are
 * not scaled, files produces in this way can used to save data from
 * the reconstruction which you plan to reload into the algorithm at a
 * later date.
 *
 * @param file_name The name of the file to write to
 * @param data The array to be written to file
 */ 
//int write_dbin(string file_name, const Double_2D & data);

/** 
 * Write a 2D complex array into a binary file (the array of type
 * fftw3 is written out). This method is useful for saving data from
 * the reconstruction which you plan to reload into the algorithm at a
 * later date. For example saving the current estimate of the object
 * exist surface wave for later use, saving the white field
 * reconstructed from 3-plane propogation for use in Fresnel
 * reconsturction, etc.
 *
 * @param file_name The name of the file to write to 
 * @param data The complex array to be written to file
 */ 
//int write_cplx(string file_name, const Complex_2D & complex);

/** 
 * Write a 2D array to a tiff file. The data will be saved as a 16 bit
 * grey-scale image. Please note that the data will be scaled to fit
 * within the range 0 - 2^16. Hence this method is only useful for
 * viewing the final output and should not be used if you plan to
 * reopen the file at a later date and pass the data back to the
 * reconstruction algorithm.
 *
 * @param file_name The name of the file to write to
 * @param data The array to be written to file
 * @param log_scale Output on log scale? true/false. Default is false.
 */ 
int write_tiff(string file_name, const Double_2D & data, 
	       bool log_scale=false,
	       double min=0, double max=0);

int write_tiff_asIs(string file_name, const Double_2D & data);

//used to transform an array of doubles in the range -x_min .... x_max
//to fall between 0... pixel_max (which is usually 2^16).
inline unsigned int io_scale_value(double min, double max, 
			  int pixel_max, 
			  double value, bool log_scale){

  if(min==max)
    return max;

  double grad;

  if(log_scale){

    //first scale to be between 1 and some large number (no. of pixels?):
    grad = (double)(pixel_max-1)/(double)(max-min);
    value = grad*(value-min)+1;
    
    //adjust to log scale
    min = 0;
    max = log10(pixel_max);
    value = log10(value);
  }
  
  grad = (double)pixel_max/(double)(max-min);

  value = grad*(value-min);

  if(value<0)
    value = min;
  if(value>pixel_max)
    value = pixel_max;

  return value;
  
}

//generic read and write methods

/**
 * Read a ppm, tiff, dbin or hdf file. This method will try to guess
 * the file type from the file name. Fills a 2D array with the data.
 * Error checks are performed and the program is exitied if an
 * error is encounted.
 *
 * @param file_name The name of the file to read from 
 * @param data The array to be filled with data
 * @param nx, ny Dimensions used when reading a ppm file.
 * @param data_name The name of the data branch if a HDF 
 * file is to be read.
 */
inline void read_image(string file_name, Double_2D & data,
		int nx=0, int ny=0, char * data_name="data"){

  int status = FAILURE;

  const char * file = file_name.c_str();
  
  if(strstr(file,".tiff\0")!=0 || strstr(file,".tif\0")!=0)
    status = read_tiff(file_name,data);
/*  
  if(strstr(file,".ppm\0")!=0)
    status = read_ppm(file_name,data);    
  
  if(strstr(file,".dbin\0")!=0){
    if(nx==0 || ny==0)
      cout << "Please pass the dimensions of the dbin file you"
	   << " wish to read to the read_image method." << endl; 
    else
      status = read_dbin(file_name,nx,ny,data);
  } 
  
  if(strstr(file,".hdf\0")!=0)
    status = read_hdf4(file_name,data,data_name); 
*/  

  if(status==FAILURE){
    cout << "Failed to read the file: " << file_name
	 << ". Exiting now.."<<endl;
    exit(0);
  }

};

/**
 * Write a ppm, tiff or dbin file. This method will try to guess the
 * file type from the file name. It writes out a 2D array of the data.
 * Error checks are performed and the program is exitied if an error
 * is encounted.
 *
 * @param file_name The name of the file to write to 
 * @param data The data array to write out
 * @param log_scale Only used for writing tiff and ppm files. By default
 *        images are not written out on a log scale.
 */
inline void write_image(string file_name, Double_2D & data, bool log_scale=false, double min=0, double max=0){
  
  int status = FAILURE;

  const char * file = file_name.c_str();
  
  if(strstr(file,".tiff\0")!=0 || strstr(file,".tif\0")!=0)
    status = write_tiff(file_name,data, log_scale, min, max);
/*
  if(strstr(file,".ppm\0")!=0)
    status = write_ppm(file_name,data, log_scale, min, max);    

  if(strstr(file,".dbin\0")!=0)
    status = write_dbin(file_name,data);   
*/
  if(status==FAILURE){
    cout << "Failed to write to the file: " << file_name
	 << ". Exiting now.."<<endl;
    exit(0);
  }

};


#endif
