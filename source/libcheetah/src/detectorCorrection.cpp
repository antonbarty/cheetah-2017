/*
 *  detectorCorrection.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 23/11/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>

#include <mmintrin.h>
#include <emmintrin.h>


#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "cheetahmodules.h"
#include "median.h"

void pnccdOffsetCorrection(float *data,uint16_t *mask);


/*
 *	Subtract pre-loaded darkcal file
 */
void subtractDarkcal(cEventData *eventData, cGlobal *global) {
	DETECTOR_LOOP {
		if(global->detector[detID].useDarkcalSubtraction) {
			/*
			 *	Subtract darkcal (from calibration file)
			 */
			long	pix_nn = global->detector[detID].pix_nn;
			float	*data = eventData->detector[detID].corrected_data;
			float	*darkcal = global->detector[detID].darkcal;
			subtractDarkcal(data, darkcal, pix_nn);
			eventData->detector[detID].pedSubtracted = 1;
		}
	}
}

void subtractDarkcal(float *data, float *darkcal, long pix_nn) {
	for(long i=0; i<pix_nn; i++) {
		data[i] -= darkcal[i]; 
	}
}



/*
 *	Apply gain correction
 *	Assumes the gaincal array is appropriately 'prepared' when loaded so that all we do is a multiplication.
 *	All that checking for division by zero (and inverting when required) needs only be done once, right? 
 */
void applyGainCorrection(cEventData *eventData, cGlobal *global) {
	DETECTOR_LOOP {
		if(global->detector[detID].useGaincal) {
			long	pix_nn = global->detector[detID].pix_nn;
			float	*data = eventData->detector[detID].corrected_data;
			float	*gaincal = global->detector[detID].gaincal;
			
			applyGainCorrection(data, gaincal, pix_nn);
		}
	}
}

void applyGainCorrection(float *data, float *gaincal, long pix_nn) {
	for(long i=0; i<pix_nn; i++) {
		data[i] *= gaincal[i]; 
	}
}



/*
 *	Apply bad pixel mask
 *	Assumes that all we have to do here is a multiplication.
 */

void applyBadPixelMask(cEventData *eventData, cGlobal *global){	
	
	DETECTOR_LOOP {
		if(global->detector[detID].applyBadPixelMask) {
			long	 pix_nn = global->detector[detID].pix_nn;
			float	 *data = eventData->detector[detID].corrected_data;
			uint16_t *mask = eventData->detector[detID].pixelmask;

			applyBadPixelMask(data, mask, pix_nn);
		}
	} 
}

void applyBadPixelMask(float *data, uint16_t *mask, long pix_nn) {
for(long i=0; i<pix_nn; i++) {
data[i] *= isBitOptionUnset(mask[i],PIXEL_IS_BAD); 
}
}



/*
 *	Subtract common mode on each module
 *	Common mode is the kth lowest pixel value in the whole ASIC (similar to a median calculation)
 */
void cspadModuleSubtract(cEventData *eventData, cGlobal *global){
    cspadModuleSubtract(eventData, global, 1);
}

void cspadModuleSubtract2(cEventData *eventData, cGlobal *global){
    cspadModuleSubtract(eventData, global, 2);
}

void cspadModuleSubtract(cEventData *eventData, cGlobal *global, int flag){
	
	DETECTOR_LOOP {
        if(global->detector[detID].cmModule == flag) {
		
			// Dereference datector arrays
			float		threshold = global->detector[detID].cmFloor;
			float		*data = eventData->detector[detID].corrected_data;
			uint16_t	*mask = eventData->detector[detID].pixelmask;
			long		asic_nx = global->detector[detID].asic_nx;
			long		asic_ny = global->detector[detID].asic_ny;
			long		nasics_x = global->detector[detID].nasics_x;
			long		nasics_y = global->detector[detID].nasics_y;
			
			cspadModuleSubtract(data, mask, threshold, asic_nx, asic_ny, nasics_x, nasics_y);
			
		}
	}
}

void cspadModuleSubtract(float *data, uint16_t *mask, float threshold, long asic_nx, long asic_ny, long nasics_x, long nasics_y) {
	
	long		e;
	long		mval;
	long		counter;
	float		median;
					  
	// Create median buffer
	float	*buffer; 
	buffer = (float*) calloc(asic_nx*asic_ny, sizeof(float));
	
	
	// Loop over modules (8x8 array)
	for(long mi=0; mi<nasics_x; mi++){
		for(long mj=0; mj<nasics_y; mj++){
			
			// Zero array
			for(long i=0; i<asic_nx*asic_ny; i++)
				buffer[i] = 0;
			
			// Loop over pixels within a module
            counter = 0;
			for(long j=0; j<asic_ny; j++){
				for(long i=0; i<asic_nx; i++){
					e = (j + mj*asic_ny) * (asic_nx*nasics_x);
					e += i + mi*asic_nx;
					if( isBitOptionUnset(mask[e],PIXEL_IS_BAD) ) {
						buffer[counter++] = data[e];
					}
				}
			}
			
			
            // Calculate background using median value 
			//median = kth_smallest(buffer, global->asic_nx*global->asic_ny, mval);
			if(counter>0) {
				mval = lrint(counter*threshold);
                if(mval < 0) 
                    mval = 1;
				median = kth_smallest(buffer, counter, mval);
			}
			else 
				median = 0;

			// Subtract median value
			for(long j=0; j<asic_ny; j++){
				for(long i=0; i<asic_nx; i++){
					e = (j + mj*asic_ny) * (asic_nx*nasics_x);
					e += i + mi*asic_nx;
					data[e] -= median;
				}
			}
		}
	}
	free(buffer);
}



/*
 *	Subtract offset estimated from unbonded pixels
 *	In the upstream detector, the unbonded pixels are in Q0:0-3 and Q2:4-5 and are at the 
 *	corners of each asic and at row=col (row<194) or row-194==col (row>194) for col%10=0.  
 */
void cspadSubtractUnbondedPixels(cEventData *eventData, cGlobal *global){
	
	DETECTOR_LOOP {
        if(global->detector[detID].cspadSubtractUnbondedPixels) { 
			
			// Dereference datector arrays
			float		*data = eventData->detector[detID].corrected_data;
			uint16_t	*mask = eventData->detector[detID].pixelmask;
			long		asic_nx = global->detector[detID].asic_nx;
			long		asic_ny = global->detector[detID].asic_ny;
			long		nasics_x = global->detector[detID].nasics_x;
			long		nasics_y = global->detector[detID].nasics_y;
			
			cspadSubtractUnbondedPixels(data, mask, asic_nx, asic_ny, nasics_x, nasics_y);
			
		}
	}
}

void cspadSubtractUnbondedPixels(float *data, uint16_t *mask, long asic_nx, long asic_ny, long nasics_x, long nasics_y) {
	
	long		e;
	float		counter;
	float		background;
	
	// Loop over modules (8x8 array)
	for(long mi=0; mi<nasics_x; mi++){
		for(long mj=0; mj<nasics_y; mj++){
			
			
			// Loop over unbonded pixels within each ASIC
			background = 0.0;
			counter = 0.0;
			for(long j=0; j<asic_ny-1; j+=10){
				long i=j;
				e = (j + mj*asic_ny) * (asic_nx*nasics_x);
				e += i + mi*asic_nx;
				background += data[e];
				counter += 1;
			}
			background /= counter;
			
			
			// Subtract background from entire ASIC
			for(long j=0; j<asic_ny; j++){
				for(long i=0; i<asic_nx; i++){
					e = (j + mj*asic_ny) * (asic_nx*nasics_x);
					e += i + mi*asic_nx;
					data[e] -= background;
					
				}
			}
		}
	}
	
}


/*
 *	Subtract common mode estimated from signal behind wires
 *	Common mode is the kth lowest pixel value in the whole ASIC (similar to a median calculation)
 */
void cspadSubtractBehindWires(cEventData *eventData, cGlobal *global){
	
	DETECTOR_LOOP {
        if(global->detector[detID].cspadSubtractBehindWires) {
			float		threshold = global->detector[detID].cmFloor;
			float		*data = eventData->detector[detID].corrected_data;
			uint16_t      	*mask = eventData->detector[detID].pixelmask;
			long		asic_nx = global->detector[detID].asic_nx;
			long		asic_ny = global->detector[detID].asic_ny;
			long		nasics_x = global->detector[detID].nasics_x;
			long		nasics_y = global->detector[detID].nasics_y;

			cspadSubtractBehindWires(data, mask, threshold, asic_nx, asic_ny, nasics_x, nasics_y);

		}
	}
}		

void cspadSubtractBehindWires(float *data, uint16_t *mask, float threshold, long asic_nx, long asic_ny, long nasics_x, long nasics_y) {
	
	long		p;
	long		counter;
	long		mval;
	float		median;
	
	// Create median buffer
	float	*buffer; 
	buffer = (float*) calloc(asic_ny*asic_nx, sizeof(float));
	
	// Loop over modules (8x8 array)
	for(long mi=0; mi<nasics_x; mi++){
		for(long mj=0; mj<nasics_x; mj++){
			
			
			// Loop over pixels within a module, remembering signal behind wires
			counter = 0;
			for(long j=0; j<asic_ny; j++){
				for(long i=0; i<asic_nx; i++){
					p = (j + mj*asic_ny) * (asic_nx*nasics_x);
					p += i + mi*asic_nx;
					if( isBitOptionSet(mask[p],PIXEL_IS_SHADOWED) ){
						buffer[counter] = data[p];
						counter++;
					}
				}
			}
			
			// Median value of pixels behind wires
			if(counter>0) {
				mval = lrint(counter*threshold);
				median = kth_smallest(buffer, counter, mval);
			}
			else 
				median = 0;
			
			
			// Subtract median value
			for(long i=0; i<asic_nx; i++){
				for(long j=0; j<asic_ny; j++){
					p = (j + mj*asic_ny) * (asic_nx*nasics_x);
					p += i + mi*asic_nx;
					data[p] -= median;
				}
			}
		}
	}
	free(buffer);
}



/*
 *	Identify hot pixels
 */
void identifyHotPixels(cEventData *eventData, cGlobal *global){
	
	DETECTOR_LOOP {
		if(global->detector[detID].useAutoHotpixel) {
			
			int		lockThreads = global->detector[detID].useBackgroundBufferMutex;
			long	pix_nn = global->detector[detID].pix_nn;
			long	hotpixADC = global->detector[detID].hotpixADC;
			long	bufferDepth = global->detector[detID].hotpixMemory;
			float	*frameData = eventData->detector[detID].corrected_data;
			int16_t	*frameBuffer = global->detector[detID].hotpix_buffer;
      
			/*
			 *	Update global hot pixel buffer
			 */

			int16_t	*buffer = (int16_t *) calloc(pix_nn,sizeof(int16_t));
			for(long i=0; i<pix_nn; i++){
				buffer[i] = (fabs(frameData[i])>hotpixADC)?(1):(0);
			}
      

			if(lockThreads)
				pthread_mutex_lock(&global->hotpixel_mutex);

			long frameID = eventData->threadNum%bufferDepth;
			memcpy(frameBuffer+pix_nn*frameID, buffer, pix_nn*sizeof(int16_t));
			eventData->nHot = global->detector[detID].nhot;

			if(lockThreads)
				pthread_mutex_unlock(&global->hotpixel_mutex);
      
			free(buffer);

		}
	}		
}



/*
 *	Kill hot pixels
 */
void applyHotPixelMask(cEventData *eventData, cGlobal *global){

	DETECTOR_LOOP {
		if (global->detector[detID].useAutoHotpixel && global->detector[detID].applyAutoHotpixel){
			long	pix_nn = global->detector[detID].pix_nn;
			float	*frameData = eventData->detector[detID].corrected_data;
			uint16_t *mask = eventData->detector[detID].pixelmask;
	
			for(long i=0; i<pix_nn; i++)
				frameData[i] *= isBitOptionUnset(mask[i],PIXEL_IS_HOT);
	
		}
	}    
}	



/* 
 *	Recalculate hot pixel masks using frame buffer
 */
void calculateHotPixelMask(cEventData *eventData,cGlobal *global){

	DETECTOR_LOOP {
		if(global->detector[detID].useAutoHotpixel) {
			float	hotpixFrequency = global->detector[detID].hotpixFreq;
			long	bufferDepth = global->detector[detID].hotpixMemory;
			long	hotpixMemory = global->detector[detID].hotpixMemory;
			long	hotpixRecalc = global->detector[detID].hotpixRecalc;
			long	hotpixCalibrated = global->detector[detID].hotpixCalibrated;
			long	lastUpdate = global->detector[detID].hotpixLastUpdate;
			
			if ( ( (eventData->threadNum == lastUpdate+hotpixRecalc) && hotpixCalibrated ) || ( (eventData->threadNum == (hotpixMemory-1)) && !hotpixCalibrated) ) {
				
				long	nhot;
				int	lockThreads = global->detector[detID].useBackgroundBufferMutex;
				long	threshold = lrint(bufferDepth*hotpixFrequency);
				long	pix_nn = global->detector[detID].pix_nn;
				uint16_t *mask = global->detector[detID].pixelmask_shared;
				int16_t	*frameBuffer = global->detector[detID].hotpix_buffer;

				if(lockThreads)
					pthread_mutex_lock(&global->hotpixel_mutex);

				printf("Detector %li: Calculating hot pixel mask at %li/%li.\n",detID, threshold, bufferDepth);
				nhot = calculateHotPixelMask(mask,frameBuffer,threshold, bufferDepth, pix_nn);
				printf("Detector %li: Identified %li hot pixels.\n",detID,nhot);
				global->detector[detID].nhot = nhot;
				global->detector[detID].hotpixLastUpdate = eventData->threadNum;
				global->detector[detID].hotpixCalibrated = 1;
	
				if(lockThreads)
					pthread_mutex_unlock(&global->hotpixel_mutex);

			}
		}	
	}
}


long calculateHotPixelMask(uint16_t *mask, int16_t *frameBuffer, long threshold, long bufferDepth, long pix_nn){

	// Loop over all pixels 
	long	counter;
	long	nhot = 0;
	for(long i=0; i<pix_nn; i++) {
		
		counter = 0;
		for(long j=0; j< bufferDepth; j++) {
			counter += frameBuffer[j*pix_nn+i]; 
		}
		
		// Apply threshold
		if(counter < threshold) {
			mask[i] &= ~(PIXEL_IS_HOT);
		}
		else {
			mask[i] |= PIXEL_IS_HOT;
			nhot++;				
		}		
	}	
	return nhot;
}



/*
 *	Apply polarization correction
 *	The polarization correction is calculated using classical electrodynamics (expression from Hura et al JCP 2000)
 */
void applyPolarizationCorrection(cEventData *eventData, cGlobal *global) {
	DETECTOR_LOOP {
		if (global->detector[detID].usePolarizationCorrection) {
			float	*data = eventData->detector[detID].corrected_data;
            float   *pix_x = global->detector[detID].pix_x;
            float   *pix_y = global->detector[detID].pix_y;
            float   *pix_dist = global->detector[detID].pix_dist;
			long	pix_nn = global->detector[detID].pix_nn;
            float   pixelSize = global->detector[detID].pixelSize;
            double  horizontalFraction = global->detector[detID].horizontalFractionOfPolarization;
            
			applyPolarizationCorrection(data, pix_x, pix_y, pix_dist, pixelSize, horizontalFraction, pix_nn);
		}
	}
}


void applyPolarizationCorrection(float *data, float *pix_x, float *pix_y, float *pix_dist, float pixelSize, double horizontalFraction, long pix_nn) {
	for(long i=0; i<pix_nn; i++) {
		data[i] /= horizontalFraction*(1 - pix_x[i]*pix_x[i]*pixelSize*pixelSize/(pix_dist[i]*pix_dist[i])) + (1 - horizontalFraction)*(1 - pix_y[i]*pix_y[i]*pixelSize*pixelSize/(pix_dist[i]*pix_dist[i]));
	}
}



/*
 *	Apply solid angle correction, two algorithms are available:
 *  1. Assume pixels are azimuthally symmetric
 *  2. Rigorous correction from solid angle of a plane triangle
 *  Both algorithms divides by the constant term of the solid angle so that
 *  the pixel scale is still comparable to ADU for hitfinding. The constant
 *  term of the solid angle is saved as an individual value in the HDF5 files.
 */
void applySolidAngleCorrection(cEventData *eventData, cGlobal *global) {
	DETECTOR_LOOP {
		if (global->detector[detID].useSolidAngleCorrection) {
            float	*data = eventData->detector[detID].corrected_data;
            float   *pix_z = global->detector[detID].pix_z;
            long	pix_nn = global->detector[detID].pix_nn;
            float   pixelSize = global->detector[detID].pixelSize;
            double  detectorZ = global->detector[detID].detectorZ;
            float   cameraLengthScale = global->detector[detID].cameraLengthScale;
            double  solidAngleConst = global->detector[detID].solidAngleConst;
            
            if (global->detector[detID].solidAngleAlgorithm == 1) {
                float   *pix_dist = global->detector[detID].pix_dist;
                
                applyAzimuthallySymmetricSolidAngleCorrection(data, pix_z, pix_dist, pixelSize, detectorZ, cameraLengthScale, solidAngleConst, pix_nn);
            } else {
                float   *pix_x = global->detector[detID].pix_x;
                float   *pix_y = global->detector[detID].pix_y;
                
                applyRigorousSolidAngleCorrection(data, pix_x, pix_y, pix_z, pixelSize, detectorZ, cameraLengthScale, solidAngleConst, pix_nn);
            }
        }
	}
}


void applyAzimuthallySymmetricSolidAngleCorrection(float *data, float *pix_z, float *pix_dist, float pixelSize, double detectorZ, float detectorZScale, double solidAngleConst, long pix_nn) {
    
    // Azimuthally symmetrical (cos(theta)^3) correction
    for (int i = 0; i < pix_nn; i++) {
        double z = pix_z[i]*pixelSize + detectorZ*detectorZScale;
        data[i] /= (z*pixelSize*pixelSize)/(pix_dist[i]*pix_dist[i]*pix_dist[i])/solidAngleConst; // remove constant term to only get theta/phi dependent part of solid angle correction for 2D pattern
    }
}


void applyRigorousSolidAngleCorrection(float *data, float *pix_x, float *pix_y, float *pix_z, float pixelSize, double detectorZ, float detectorZScale, double solidAngleConst, long pix_nn) {            
    
    // Rigorous correction from solid angle of a plane triangle
    for (int i = 0; i < pix_nn; i++) {
        
        // allocate local arrays
        double corner_coordinates[4][3]; // array of vector coordinates of pixel corners, first index starts from upper left corner and goes around clock-wise, second index determines X=0/Y=1/Z=2 coordinate
        double corner_distances[4]; // array of distances of pixel corners, index starts from upper left corner and goes around clock-wise
        double determinant;
        double denominator;
        double solid_angle[2]; // array of solid angles of the two plane triangles that form the pixel
        double total_solid_angle;
        
        // upper left corner
        corner_coordinates[0][0] = pix_x[i]*pixelSize + pixelSize/2;
        corner_coordinates[0][1] = pix_y[i]*pixelSize + pixelSize/2;
        // upper right corner
        corner_coordinates[1][0] = pix_x[i]*pixelSize - pixelSize/2;
        corner_coordinates[1][1] = pix_y[i]*pixelSize + pixelSize/2;
        // lower right corner
        corner_coordinates[2][0] = pix_x[i]*pixelSize - pixelSize/2;
        corner_coordinates[2][1] = pix_y[i]*pixelSize - pixelSize/2;
        // lower left corner
        corner_coordinates[3][0] = pix_x[i]*pixelSize + pixelSize/2;
        corner_coordinates[3][1] = pix_y[i]*pixelSize - pixelSize/2;
        // assign Z coordinate as detector distance and calculate length of the vectors to the pixel coordinates
        for (int j = 0; j < 4; j++) {
            corner_coordinates[j][2] = pix_z[i]*pixelSize + detectorZ*detectorZScale;
            corner_distances[j] = sqrt(corner_coordinates[j][0]*corner_coordinates[j][0] + corner_coordinates[j][1]*corner_coordinates[j][1] + corner_coordinates[j][2]*corner_coordinates[j][2]);
        }
        
        // first triangle made up of upper left, upper right, and lower right corner
        // nominator in expression for solid angle of a plane triangle - magnitude of triple product of first 3 corners
        determinant = fabs( corner_coordinates[0][0]*(corner_coordinates[1][1]*corner_coordinates[2][2] - corner_coordinates[1][2]*corner_coordinates[2][1])
                           - corner_coordinates[0][1]*(corner_coordinates[1][0]*corner_coordinates[2][2] - corner_coordinates[1][2]*corner_coordinates[2][0])
                           + corner_coordinates[0][2]*(corner_coordinates[1][0]*corner_coordinates[2][1] - corner_coordinates[1][1]*corner_coordinates[2][0]) );
        denominator = corner_distances[0]*corner_distances[1]*corner_distances[2] + corner_distances[2]*(corner_coordinates[0][0]*corner_coordinates[1][0] + corner_coordinates[0][1]*corner_coordinates[1][1] + corner_coordinates[0][2]*corner_coordinates[1][2])
        + corner_distances[1]*(corner_coordinates[0][0]*corner_coordinates[2][0] + corner_coordinates[0][1]*corner_coordinates[2][1] + corner_coordinates[0][2]*corner_coordinates[2][2])
        + corner_distances[0]*(corner_coordinates[1][0]*corner_coordinates[2][0] + corner_coordinates[1][1]*corner_coordinates[2][1] + corner_coordinates[1][2]*corner_coordinates[2][2]);
        solid_angle[0] = atan2(determinant, denominator);
        if (solid_angle[0] < 0)
            solid_angle[0] += M_PI; // If det > 0 and denom < 0 arctan2 returns < 0, so add PI
        
        // second triangle made up of lower right, lower left, and upper left corner
        // nominator in expression for solid angle of a plane triangle - magnitude of triple product of last 3 corners
        determinant = fabs( corner_coordinates[0][0]*(corner_coordinates[3][1]*corner_coordinates[2][2] - corner_coordinates[3][2]*corner_coordinates[2][1])
                           - corner_coordinates[0][1]*(corner_coordinates[3][0]*corner_coordinates[2][2] - corner_coordinates[3][2]*corner_coordinates[2][0])
                           + corner_coordinates[0][2]*(corner_coordinates[3][0]*corner_coordinates[2][1] - corner_coordinates[3][1]*corner_coordinates[2][0]) );
        denominator = corner_distances[2]*corner_distances[3]*corner_distances[0] + corner_distances[2]*(corner_coordinates[0][0]*corner_coordinates[3][0] + corner_coordinates[0][1]*corner_coordinates[3][1] + corner_coordinates[0][2]*corner_coordinates[3][2])
        + corner_distances[3]*(corner_coordinates[0][0]*corner_coordinates[2][0] + corner_coordinates[0][1]*corner_coordinates[2][1] + corner_coordinates[0][2]*corner_coordinates[2][2])
        + corner_distances[0]*(corner_coordinates[3][0]*corner_coordinates[2][0] + corner_coordinates[3][1]*corner_coordinates[2][1] + corner_coordinates[3][2]*corner_coordinates[2][2]);
        solid_angle[1] = atan2(determinant, denominator);
        if (solid_angle[1] < 0)
            solid_angle[1] += M_PI; // If det > 0 and denom < 0 arctan2 returns < 0, so add PI
        
        total_solid_angle = 2*(solid_angle[0] + solid_angle[1]);
        
        data[i] /= total_solid_angle/solidAngleConst; // remove constant term to only get theta/phi dependent part of solid angle correction for 2D pattern
    }
}



// Read out artifact compensation for pnCCD back detector
/*
  Effect: Negative offset in lines orthogonal to the read out direction. Occurs if integrated signal in line is high.
  Correction formula: O_i(x) = M(x) + ( M_i(x) * m_i + c_i ) * x
  O_i(x): offset that is applied to line x in quadrant i
  M_i(x): mean value of insensitive pixels (12 pixels closest to the edge) in line x in quadrant i
  m_a1 = 0.055 1/px ; m_a2 = 0.0050  1/px ; m_b2 = 0.0056 1/px ; m_b1 = 0.0049 1/px
  c_a1 = 0.0047 adu/px ; c_a2 = 0.0078 adu/px ; c_b2 = 0.0007 adu/px ; c_b1 = 0.0043 adu/px
  Apply correction only if integrated signal in line is above certain threshold (50000 ADU).

  This is what the detector map looks like:
 
  ---> x
  |
  v y

 
  insensitive pixels at the edge
  |                 | 
  v                 v 
  --------- ---------
  read out <- |       | |       | -> read-out
  <- |  q=0  | |  q=1  | ->
  <- |       | |       | ->
  <- | - - - |x| - - - | ->
  <- |       | |       | ->
  <- |  q=2  | |  q=3  | ->
  <- |       | |       | ->
  --------- ---------
  ^                 ^
  |                 | 
  insensitive pixels at the edge
	
	
*/
void pnccdOffsetCorrection(cEventData *eventData, cGlobal *global){

	DETECTOR_LOOP {
		if(strcmp(global->detector[detID].detectorType, "pnccd") == 0  && global->detector[detID].usePnccdOffsetCorrection == 1) {
			float	*data = eventData->detector[detID].corrected_data;
			uint16_t *mask = eventData->detector[detID].pixelmask;
			pnccdOffsetCorrection(data,mask);
		}
	}
}		


void pnccdOffsetCorrection(float *data,uint16_t *mask) {
    float sum,m;
    int i,j,x,y,mx,my,x_;
    int q;
    int asic_nx = PNCCD_ASIC_NX;
    int asic_ny = PNCCD_ASIC_NY;
    int nasics_x = PNCCD_nASICS_X;
    int nasics_y = PNCCD_nASICS_Y;
    int x_insens_start[4] = {11,1012,11,1012};
    int x_sens_start[4] = {511,512,511,512};
    int insensitve_border_width = 12;
    int Nxsens = 500;
    float sumThreshold = 50000.;
    float offset_m[4] = {0.0055,0.0056,0.0050,0.0049};
    float offset_c[4] = {0.0047,0.0007,0.0078,0.0043};
    int read_out_direction[4] = {-1,1,-1,1};

    // Loop over quadrants
    for(my=0; my<nasics_y; my++){
        for(mx=0; mx<nasics_x; mx++){
            q = mx+my*nasics_x;
            for(y=0; y<asic_ny; y++){
                // sum up values in line
                sum = 0.;
                for(x=0; x<asic_nx; x++){
                    i = my * (asic_ny*asic_nx*nasics_x) + y * asic_nx*nasics_x + mx*asic_nx + x;
                    sum += data[i];
                }
                
                // only do corrections for line if sum exceeds threshold
                if (sum > sumThreshold){
                    
                    // calculate mean value of insensitve pixels
                    m = 0.;
                    for(x_=0; x_<insensitve_border_width; x_++){
                        x = x_insens_start[q]+x_*read_out_direction[q];
                        j = my * (asic_ny*asic_nx*nasics_x) + y * asic_nx*nasics_x + x;
                        m += data[j];
                    }
                    m /= float(insensitve_border_width);
                    
                    // do offset correction
                    for(x_=0; x_<Nxsens; x_++){
                        x = x_sens_start[q]+x_*read_out_direction[q];
                        j = my * (asic_ny*asic_nx*nasics_x) + y * asic_nx*nasics_x + x;
                        data[j] -= m;
                        data[j] -= (m*offset_m[q]+offset_c[q])*float(500-x_);
						mask[j] |= PIXEL_IS_ARTIFACT_CORRECTED;
                    }
                }
            }
        }
    }
}



void pnccdLineInterpolation(cEventData *eventData,cGlobal *global){
	DETECTOR_LOOP {
		if((strcmp(global->detector[detID].detectorType, "pnccd") == 0) && (global->detector[detID].usePnccdLineInterpolation == 1)) {
			// lines in direction of the slowly changing dimension 
			long nx = PNCCD_ASIC_NX * PNCCD_nASICS_X;
			long ny = PNCCD_ASIC_NY * PNCCD_nASICS_Y;
			long x,y,i,i0,i1;
			long x_min = 1;
			long x_max = nx-1;
			float *data = eventData->detector[detID].corrected_data;
			uint16_t *mask = eventData->detector[detID].pixelmask;
			uint16_t mask_out_bits = PIXEL_IS_INVALID | PIXEL_IS_SATURATED | PIXEL_IS_HOT | PIXEL_IS_DEAD |
				PIXEL_IS_SHADOWED | PIXEL_IS_TO_BE_IGNORED | PIXEL_IS_BAD  | PIXEL_IS_MISSING;
			for(y=0; y<ny; y++){
				for(x=x_min;x<=x_max;x=x+2){
					i = nx*y+x;
					i0 = nx*y+x-1;
					i1 = nx*y+x+1;
					if (isNoneOfBitOptionsSet(mask[i0],mask_out_bits) && isNoneOfBitOptionsSet(mask[i1],mask_out_bits)){
						data[i] = (data[i0]+data[i1])/2.;
					}
				}
			}	    
		}
	}
}

void pnccdLineMasking(cEventData *eventData,cGlobal *global){
	DETECTOR_LOOP {
		if((strcmp(global->detector[detID].detectorType, "pnccd") == 0) && (global->detector[detID].usePnccdLineMasking == 1)) {
			// lines in direction of the slowly changing dimension 
			long nx = PNCCD_ASIC_NX * PNCCD_nASICS_X;
			long ny = PNCCD_ASIC_NY * PNCCD_nASICS_Y;
			long x,y,i,i0,i1;
			long x_min = 1;
			long x_max = nx-1;
			uint16_t *mask = eventData->detector[detID].pixelmask;
			for(y=0; y<ny; y++){
				for(x=x_min;x<=x_max;x=x+2){
					i = nx*y+x;
					i0 = nx*y+x-1;
					i1 = nx*y+x+1;
					mask[i] |= PIXEL_IS_BAD;
					if (global->detector[detID].usePnccdLineInterpolation == 1){
						mask[i] |= PIXEL_IS_ARTIFACT_CORRECTED;
					}
				}
			}	    
		}
	}
}


// The following can be fixed also with an adequate geometry as well.
/*
 * Fix pnccd wiring error
 *
 * For each CCD, there are two CAMEX channels which may require special attention for scientific analysis. 
 * When numbering the 1024 CAMEX channels of a CCD from 1 to 1024, these are channels 513 and 1024. 
 * CAMEX channel 513 is not bonded to a physical detector channel. It herefore does not contain any photon data, 
 * but only CAMEX noise, and should be excluded from scientific analysis. The data of the physical detector channel n 
 * for 512 ≤ n ≤ 1022 is in CAMEX channel n + 1. CAMEX channel 1024 is bonded to both detector channels 1023 and 1024. 
 * This channel therefore contains the summed signal of detector channels 1023 and 1024. Depending on the analysis goal, 
 * this channel may be excluded from further analysis, treated as the sum that it actually is, or may be even split into two 
 * channels under some assumptions.
 *	
 *	In IDL and using the 1st version of CASS this becomes:
 *	;; Re-align top right
 *	data[512:1023, 512:1023] = shift(data[512:1023,512:1023], 0, -1)
 *	data[512:1023, 1022:1023] = 0
 *
 *	;; Re-align bottom left
 *	data[0:511, 0:511] = shift(data[0:511, 0:511], 0, 1)
 *	data[0:511, 0:1] = 0
 *
 *  Everything in cheetah is rotated 90 degrees CW compared to CASS 
 *  (and this change is reflected in the code below)
 */

void pnccdFixWiringError(cEventData *eventData, cGlobal *global) {
    DETECTOR_LOOP {
        if(strcmp(global->detector[detID].detectorType, "pnccd") == 0 ) {
            if(global->detector[detID].usePnccdFixWiringError == 1) {
                float	*data = eventData->detector[detID].corrected_data;
                pnccdFixWiringError(data);
            }
        }
    }
}


void pnccdFixWiringError(float *data) {
	long	i,j;
	long    nx = PNCCD_ASIC_NX * PNCCD_nASICS_X;
    

	// Fix top left quadrant 
	// (shift all pixels right by one and zero first column)
	for(j=512; j<1023; j++)		
		for(i=511; i>0; i--)
			data[i+j*nx] = data[(i-1)+j*nx];            
	for(j=512; j<1024; j++) 
		data[0+j*nx] = 0;
    

	// Fix bottom right quadrant 
	//  (shift all pixels left by one and zero last column)
	for(j=0; j<512; j++)
		for(i=512; i<1024; i++)
			data[i+j*nx] =  data[(i+1)+j*nx];            
	for(j=0; j<512; j++) 
		data[1023+j*nx] = 0;
    
}

