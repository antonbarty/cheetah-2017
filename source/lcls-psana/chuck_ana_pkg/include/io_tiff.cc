#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include <cmath>
#include "tiffio.h"
#include "io.h"
#include "Double_2D.h"

using namespace std;

#define FAILURE 0
#define SUCCESS 1

#define ONE_BYTE 1
#define TWO_BYTE 2
#define FOUR_BYTE 4

/***************************************************************/
//bad code which allows the array type to be 
//decided at run-time.
/***************************************************************/
class tiff_anonymous_array{

  int type_;
  uint8  * a_uint8;
  uint16 * a_uint16;
  uint32 * a_uint32;
  
 public:

  tiff_anonymous_array(int type, int size){
    type_ = type;

     a_uint8=0;
     a_uint16=0;
     a_uint32=0;

    switch( type ){
    case ONE_BYTE:
      a_uint8 = new uint8[size];
      break;
    case TWO_BYTE:
      a_uint16 = new uint16[size];
      break;
    case FOUR_BYTE:
      a_uint32 = new uint32[size];
      break;
    default:
      cout << "Not familiar with type.. exiting.." << endl;
      exit(1);
    }    
  }
  
  ~tiff_anonymous_array(){
    switch( type_ ){
    case ONE_BYTE:
      delete[] a_uint8;
      break;
    case TWO_BYTE:
      delete[] a_uint16;
      break;
    case FOUR_BYTE:
      delete[] a_uint32;
      break;
    default:
      cout << "Not familiar with the tiff type.. exiting.." << endl;
      exit(1);
    }    
  }

  void * return_array(){
    switch( type_ ){
    case ONE_BYTE:
      return a_uint8;
    case TWO_BYTE:
      return a_uint16;
    case FOUR_BYTE:
      return a_uint32;
     default:
      cout << "Not familiar with the tiff type.. exiting.." << endl;
      exit(1);
    }  
  }

  double return_array_value(int i){

    //coverting to double. Not ideal.
    switch( type_ ){
    case ONE_BYTE:
      return a_uint8[i];
    case TWO_BYTE:
      return a_uint16[i];
    case FOUR_BYTE:
      return a_uint32[i];
    default:
      cout << "Not familiar with the tiff type.. exiting.." << endl;
      exit(1);
    }          
  }
};

/*********************************************************/

/*********************************************************/
int read_tiff(string file_name, Double_2D & data){
 
  //open the input file:
  TIFF* tif = TIFFOpen(file_name.c_str(), "r");
 
  if (tif) {
    int dircount = 0;
    for( ; TIFFReadDirectory(tif); dircount++);
    if(dircount > 1)
      cout << "Multiple directories in the file "<<file_name
	   << "... only using the first" <<endl;
  }
  else{
    cout << "Could not open the file "<<file_name<<endl;
    return FAILURE;
  }
  
  uint32 w, h;
  uint16 bits_per_sample;
  uint16 samples_per_pixel;

  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
  TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel);

uint16 sampleformat;
TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleformat);
//cout << "sample format: " << sampleformat << endl;  

//cout << "width: " << w << endl;
//cout << "height: " << h << endl;
//cout << "bits per pixel: " << bits_per_sample << endl;  
//cout << "samples per pixel: " << samples_per_pixel << endl;  
samples_per_pixel = 1;								// NEED TO FIX!!!!!!!!!!!!!

  int strip_size = TIFFStripSize(tif);
  int bytes_per_pixel = bits_per_sample*samples_per_pixel/8;
  int pixels_per_strip = strip_size/bytes_per_pixel;

//cout << "strip_size: " << strip_size << endl;
//cout << "bytes_per_pixel: " << bytes_per_pixel << endl;  
//cout << "pixels_per_strip: " << pixels_per_strip << endl;  
  
  double * grey_image = new double[w*h]; 
  uint32 * colour_image = new uint32[w*h];

  if(samples_per_pixel>1){ //see if the image is colour

    //cout << "Processing colour image" << endl;
    TIFFReadRGBAImage(tif, w, h, colour_image, 1);

  }
  else{ //otherwise if the image is grey scale

    //cout << "Processing grey scale image" << endl;

    tiff_anonymous_array * buffer;

    switch(bytes_per_pixel){ //otherwise set up the buffer for grey scale
    case(ONE_BYTE):
      buffer = new tiff_anonymous_array(ONE_BYTE,pixels_per_strip);
    break;
    case(TWO_BYTE):
      buffer = new tiff_anonymous_array(TWO_BYTE,pixels_per_strip);
    break;
    case(FOUR_BYTE):
      buffer = new tiff_anonymous_array(FOUR_BYTE,pixels_per_strip);
      break;
    default:
      cout << "Confused about the tiff image.." <<endl;
      return FAILURE;
    }
  

    for ( tstrip_t strip = 0; strip < TIFFNumberOfStrips(tif); strip++){
      int bytes_read = TIFFReadEncodedStrip(tif, strip, buffer->return_array(), (tsize_t) - 1);
      for( int i=0; i< bytes_read/bytes_per_pixel; i++ ){
	grey_image[i+strip*pixels_per_strip] = buffer->return_array_value(i);
      }
    }
     
  }

  //copy to the image array

  //make space for the array if it hasn't 
  //already been allocated.
  if(data.get_size_x()==0)
    data.allocate_memory(w,h);

  for(int i=0; i < w; ++i){
    for(int j=0; j< h; ++j){
      if(samples_per_pixel>1){//if the image is colour we take the sum of colour value
	uint32 pixel = colour_image[(h-j-1)*w+i];
	data.set(i,j,TIFFGetR(pixel)+TIFFGetG(pixel)+TIFFGetB(pixel));
      }
      else //grey scale:
	data.set(i,j,grey_image[j*w+i]);
    }
  }
  
  delete[] grey_image;
  delete[] colour_image;

  TIFFClose(tif);
  
  return SUCCESS; //success
    
};

/** write data out to a tiff file **/
int write_tiff(string file_name, const Double_2D & data, bool log_scale,
	       double min, double max){

  TIFF* tif = TIFFOpen(file_name.c_str(), "w");
  if (!tif) {
    cout << "Could not open the file "<<file_name<<endl;
    return FAILURE;
  }

  if(min==0 && max==0){
    max = data.get_max();
    min = data.get_min();
  }

  int max_pixel = 65535; //2^(8bit*2bytes)
  //int max_pixel = 4294967296; //2^(8bit*4bytes)

  //copy the image into an array
  int w = data.get_size_x();
  int h = data.get_size_y();
  uint16 * grey_image = new uint16[w*h];
  for(int i=0; i < w; i++){
    for(int j=0; j< h; j++){ //copy and scale
      grey_image[j*w+i] = io_scale_value(min,max, 
					 max_pixel, 
					 data.get(i,j),
					 log_scale);
    }
  }
  
  // We need to set some values for basic tags before we can add any data
  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, w);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, h);
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8*sizeof(uint16));
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
  TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, h);

  TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS); 
  TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
  

  //write the data out as a single strip
  int bytes_written = TIFFWriteEncodedStrip(tif, 0, 
					    (void*) grey_image, 
					    h*w*sizeof(uint16));
  if(bytes_written!=h*w*sizeof(uint16)){
    cout << "Problem writing to tiff file" << endl;
    exit(1);
  }

  TIFFClose(tif);
  delete[] grey_image;  
  return SUCCESS; //success

};

/** write data out to a tiff file **/
int write_tiff_asIs(string file_name, const Double_2D & data){
  TIFF* tif = TIFFOpen(file_name.c_str(), "w");
  if (!tif) {
    cout << "Could not open the file "<<file_name<<endl;
    return FAILURE;
  }

  //copy the image into an array
  int w = data.get_size_x();
  int h = data.get_size_y();
  uint16 * grey_image = new uint16[w*h];
  for(int i=0; i < w; i++){
    for(int j=0; j< h; j++){ //copy
      grey_image[j*w+i] = data.get(i,j);
    }
  }
  cout << "data:" << data.get(64,64) << endl;
  cout << "grey:" << grey_image[64*w+64] << endl;
  // We need to set some values for basic tags before we can add any data
  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, w);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, h);
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8*sizeof(uint16));
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
  TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, h);

  TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS); 
  TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
  

  //write the data out as a single strip
  int bytes_written = TIFFWriteEncodedStrip(tif, 0, 
					    (void*) grey_image, 
					    h*w*sizeof(uint16));
  if(bytes_written!=h*w*sizeof(uint16)){
    cout << "Problem writing to tiff file" << endl;
    exit(1);
  }

  TIFFClose(tif);
  delete[] grey_image;  
  return SUCCESS; //success

};
