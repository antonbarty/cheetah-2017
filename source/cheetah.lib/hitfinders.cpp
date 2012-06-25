
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>

#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "median.h"
#include "hitfinders.h"


// Peakfinders local to this routine
int peakfinder3(cGlobal*, cEventData*, int);
int peakfinder5(cGlobal*, cEventData*, int);
int peakfinder6(cGlobal*, cEventData*, int);



/*
 *	Various flavours of hitfinder
 *		1 - Number of pixels above ADC threshold
 *		2 - Total intensity above ADC threshold
 *		3 - Count Bragg peaks
 *		4 - Use TOF
 *		5 - Like 3, but with extras
 *		6 - Experimental - find peaks by SNR criteria
 *      7 - Laser on event code (usually EVR41)
 */
int  hitfinder(cEventData *eventData, cGlobal *global, int detID){
	
	// Dereference stuff
	long		pix_nn = global->detector[detID].pix_nn;
	long		asic_nx = global->detector[detID].asic_nx;
	long		asic_ny = global->detector[detID].asic_ny;

	//printf("************>>> %li, %li, %li\n", asic_nx, asic_ny, pix_nn);

	
	long	nat;
	long	counter;
	int		hit=0;
	float	total;
	long	*inx = (long *) calloc(pix_nn, sizeof(long));
	long	*iny = (long *) calloc(pix_nn, sizeof(long));
	float	mingrad = global->hitfinderMinGradient*2;
	mingrad *= mingrad;
	
	nat = 0;
	counter = 0;
	total = 0.0;
	
	/*
	 *	Default values for some metrics
	 */
	eventData->peakNpix = 0;
	eventData->peakTotal = 0;
	eventData->peakResolution = 0;
	eventData->peakDensity = 0;
	
	
	/*
	 *	Use a data buffer so we can zero out pixels already counted
	 */
	//printf("************>>> %li, %li, %li\n", asic_nx, asic_ny, pix_nn);
	float *temp = (float*) calloc(pix_nn, sizeof(float));
	if(temp == NULL) 
		printf("temp == NULL\n");
	//for(long i=0;i<pix_nn;i++)
	//	temp[i] = eventData->detector[detID].corrected_data[i]; 
	memcpy(temp, eventData->detector[detID].corrected_data, pix_nn*sizeof(float));
	
	/*
	 *	Apply peak search mask 
	 *	(multiply data by 0 to ignore regions)
	 */
	if(global->hitfinderUsePeakmask) {
		for(long i=0;i<pix_nn;i++){
			temp[i] *= global->detector[detID].peakmask[i]; 
		}
	}
	
	
	// Combined mask
	int* mask = (int *) calloc(pix_nn, sizeof(int) );
	memcpy(mask, global->hitfinderResMask, pix_nn*sizeof(int));
	for (long i=0; i<pix_nn; i++) 
        mask[i] *= global->detector[detID].hotpixelmask[i] * global->detector[detID].badpixelmask[i] * eventData->detector[detID].saturatedPixelMask[i];
	
    
	/*
	 *	Use one of various hitfinder algorithms
	 */
	switch(global->hitfinderAlgorithm) {
			
		case 1 :	// Count the number of pixels above ADC threshold
			for(long i=0;i<pix_nn;i++){
				if(temp[i] > global->hitfinderADC){
					total += temp[i];
					nat++;
				}
			}
			if(nat >= global->hitfinderMinPixCount)
				hit = 1;
			
			eventData->peakNpix = nat;
			eventData->nPeaks = nat;
			eventData->peakTotal = total;
			break;
			
			
		case 2 :	//	integrated intensity above threshold
			for(long i=0;i<pix_nn;i++){
				if(temp[i] > global->hitfinderADC){
					total += temp[i];
					nat++;
				}
			}
			if(total >= global->hitfinderTAT) 
				hit = 1;
			
			eventData->peakNpix = nat;
			eventData->nPeaks = nat;
			eventData->peakTotal = total;
			break;
			
		case 3 : 	// Count number of Bragg peaks
			hit = peakfinder3(global, eventData, detID);			
			break;	

            
		case 4 :	// Use TOF signal to find hits
			if ((global->hitfinderUseTOF==1) && (eventData->TOFPresent==1)){
				double total_tof = 0.;
				for(int i=global->hitfinderTOFMinSample; i<global->hitfinderTOFMaxSample; i++){
					total_tof += eventData->TOFVoltage[i];
				}
				if (total_tof > global->hitfinderTOFThresh)
					hit = 1;
			}
			// Use cspad threshold if TOF is not present 
			else {
				for(long i=0;i<pix_nn;i++){
					if(temp[i] > global->hitfinderADC){
						nat++;
					}
				}
				if(nat >= global->hitfinderMinPixCount)
					hit = 1;
			}
			break;
			
		case 5 : 	// Count number of Bragg peaks
			hit = peakfinder5(global,eventData, detID);
			break;
			
		case 6 : 	// Count number of Bragg peaks
			hit = peakfinder6(global,eventData, detID);
			break;
            
		case 7 : 	// Return laser on event code
            hit = eventData->laserEventCodeOn;
            eventData->nPeaks = eventData->laserEventCodeOn;
			break;
			
		default :
			printf("Unknown hit finding algorithm selected: %i\n", global->hitfinderAlgorithm);
			printf("Stopping in confusion.\n");
			exit(1);
			break;
			
	}
	
	// Statistics on the peaks, for certain hitfinders
	if( eventData->nPeaks > 1 &&
	   ( global->hitfinderAlgorithm == 3 || global->hitfinderAlgorithm == 5 || global->hitfinderAlgorithm == 6 ) ) {
		   
		long	np;
		long  kk;
		float	resolution;
		float	resolutionA;	
		float	cutoff = 0.95;
	   
		np = eventData->nPeaks;
		if(np >= global->hitfinderNpeaksMax) 
		   np = global->hitfinderNpeaksMax; 
		kk = (long) floor(cutoff*np);
	


		// Pixel radius resolution (bigger is better)
		float *buffer1 = (float*) calloc(global->hitfinderNpeaksMax, sizeof(float));
		for(long k=0; k<np; k++) 
			buffer1[k] = eventData->peak_com_r_assembled[k];
		resolution = kth_smallest(buffer1, np, kk);		   
		eventData->peakResolution = resolution;
		free(buffer1);
	
		// Resolution to real space (in Angstrom)
		// Crystallographic resolution d = lambda/sin(theta)
		float z = global->detector[0].detectorZ;
		float dx = global->detector[0].pixelSize;
		double r = sqrt(z*z+dx*dx*resolution*resolution);
		double sintheta = dx*resolution/r;
		resolutionA = eventData->wavelengthA/sintheta;
		eventData->peakResolutionA = resolutionA;

	
		if(resolution > 0) {
			float	area = (3.141*resolution*resolution)/(asic_ny*asic_nx);
			eventData->peakDensity = (cutoff*np)/area;
		}
	   
    } 
	
	// Update central hit counter
	if(hit) {
		pthread_mutex_lock(&global->nhits_mutex);
		global->nhits++;
		global->nrecenthits++;
		pthread_mutex_unlock(&global->nhits_mutex);
	}
	

	free(inx); 			
	free(iny);	
	free(mask);	
	free(temp);
	
	return(hit);
	
}



/*
 *	Peakfinder 3
 *	Search of connected pixels above threshold
 *	Anton Barty
 */
int peakfinder3(cGlobal *global, cEventData *eventData, int detID) {
	
	if(detID > global->nDetectors) {
		printf("peakfinder 3: false detectorID %i\n",detID);
		exit(1);
	}
	
	// Dereference stuff
	long		pix_nx = global->detector[detID].pix_nx;
	long		pix_ny = global->detector[detID].pix_ny;
	long		pix_nn = global->detector[detID].pix_nn;
	long		asic_nx = global->detector[detID].asic_nx;
	long		asic_ny = global->detector[detID].asic_ny;
	long		nasics_x = global->detector[detID].nasics_x;
	long		nasics_y = global->detector[detID].nasics_y;
	
	//printf("************>>> %li, %li, %li\n", pix_nx, pix_ny, pix_nn);

	// Variables for this hitfinder
	long	nat, lastnat;
	long	counter;
	int		hit=0;
	float	total;
	int search_x[] = {-1,0,1,-1,1,-1,0,1};
	int search_y[] = {-1,-1,-1,0,0,1,1,1};
	int	search_n = 8;
	long e;
	long *inx = (long *) calloc(pix_nn, sizeof(long));
	long *iny = (long *) calloc(pix_nn, sizeof(long));
	float totI;
	float peak_com_x;
	float peak_com_y;
	long thisx;
	long thisy;
	long fs, ss;
	//float grad;
	//float lbg, imbg; /* local background nearby peak */
	//float *lbg_buffer;
	//int fsmin, fsmax, ssmin, ssmax;
	//int lbg_ss, lbg_fs, lbg_e;
	//int thisfs, thisss;
	float mingrad = global->hitfinderMinGradient*2;
	mingrad *= mingrad;
	//double dx1, dx2, dy1, dy2;
	//double dxs, dys;
	
	nat = 0;
	counter = 0;
	total = 0.0;
	
	// Combined mask of all areas to ignore
	int *mask = (int *) calloc(pix_nn, sizeof(int) );
	memcpy(mask,global->hitfinderResMask,pix_nn*sizeof(int));
	for (long i=0; i<pix_nn; i++) 
        mask[i] *= global->detector[detID].hotpixelmask[i] * global->detector[detID].badpixelmask[i] * eventData->detector[detID].saturatedPixelMask[i] * global->detector[detID].peakmask[i];
	
	// zero out bad pixels in temporary intensity map
	float* temp = (float *) calloc(pix_nn,sizeof(float));
	for (long i=0; i<pix_nn; i++) 
        temp[i] = eventData->detector[detID].corrected_data[i]*mask[i];
	
    
	// Loop over modules (8x8 array)
	for(long mj=0; mj<nasics_y; mj++){
		for(long mi=0; mi<nasics_x; mi++){
			
			// Loop over pixels within a module
			for(long j=1; j<asic_ny-1; j++){
				for(long i=1; i<asic_nx-1; i++){
					
					
					ss = (j+mj*asic_ny)*pix_nx;
					fs = i+mi*asic_nx;
					e = ss + fs;
					
					if(e >= pix_nn) {
						printf("Array bounds error: e=%i\n");
						exit(1);
					}
					
					if(temp[e] > global->hitfinderADC){
						
                        // Rick's gradient test (if minGradient>0)
                        /*
						if ( global->hitfinderMinGradient > 0 ){
							
							// Get gradients 
							dx1 = temp[e] - temp[e+1];
							dx2 = temp[e-1] - temp[e];
							dy1 = temp[e] - temp[e+global->pix_nx];
							dy2 = temp[e-global->pix_nx] - temp[e];
							
							// Average gradient measurements from both sides 
							dxs = ((dx1*dx1) + (dx2*dx2)) / 2;
							dys = ((dy1*dy1) + (dy2*dy2)) / 2;
							
							// Calculate overall gradient 
							grad = dxs + dys;
							
                            if ( grad < global->hitfinderMinGradient ) 
                               continue;
						}
                        */	
						
						
						// This might be the start of a peak - start searching
						inx[0] = i;
						iny[0] = j;
						nat = 1;
						totI = 0; 
						peak_com_x = 0; 
						peak_com_y = 0; 
						
						// Keep looping until the pixel count within this peak does not change
						do {
							
							lastnat = nat;
							// Loop through points known to be within this peak
							for(long p=0; p<nat; p++){
								// Loop through search pattern
								for(long k=0; k<search_n; k++){
									// Array bounds check
									if((inx[p]+search_x[k]) < 0)
										continue;
									if((inx[p]+search_x[k]) >= asic_nx)
										continue;
									if((iny[p]+search_y[k]) < 0)
										continue;
									if((iny[p]+search_y[k]) >= asic_ny)
										continue;
									
									// Neighbour point 
									thisx = inx[p]+search_x[k]+mi*asic_nx;
									thisy = iny[p]+search_y[k]+mj*asic_ny;
									e = thisx + thisy*pix_nx;
									
									//if(e < 0 || e >= global->pix_nn){
									//	printf("Array bounds error: e=%i\n",e);
									//	continue;
									//}
									
									// Above threshold?
									if(temp[e] > global->hitfinderADC){
										//if(nat < 0 || nat >= global->pix_nn) {
										//	printf("Array bounds error: nat=%i\n",nat);
										//	break
										//}
										totI += temp[e]; // add to integrated intensity
										peak_com_x += temp[e]*( (float) thisx ); // for center of mass x
										peak_com_y += temp[e]*( (float) thisy ); // for center of mass y
										temp[e] = 0; // zero out this intensity so that we don't count it again
										inx[nat] = inx[p]+search_x[k];
										iny[nat] = iny[p]+search_y[k];
										nat++;
									}
								}
							}
						} while(lastnat != nat);
						
                        // Too many or too few pixels means ignore this 'peak'
						if(nat<global->hitfinderMinPixCount || nat>global->hitfinderMaxPixCount) {
                            continue;
                        }

                            
                        // Peak above acceptable SNR ratio? (turn off check by setting hitfinderMinSNR = 0)
                        float   localSigma=0;
                        if(global->hitfinderMinSNR > 0) {
                            long   com_x = lrint(peak_com_x/totI) - mi*asic_nx;
                            long   com_y = lrint(peak_com_y/totI) - mj*asic_ny;
                            
                            // Calculate standard deviation sigma in an annulus around this peak
                            long    ringWidth = global->hitfinderLocalBGRadius+global->hitfinderLocalBGThickness;
                            float   thisr;
                            
                            float   sum = 0;
                            float   sumsquared = 0;
                            long    np_sigma = 0;
                            for(long bj=-ringWidth; bj<ringWidth; bj++){
                                for(long bi=-ringWidth; bi<ringWidth; bi++){
                                    
                                    // Within annulus?
                                    thisr = sqrt( bi*bi + bj*bj );
                                    if(thisr < global->hitfinderLocalBGRadius || thisr > global->hitfinderLocalBGRadius+global->hitfinderLocalBGThickness )
                                        continue;

                                    // Within-ASIC check
                                    if((com_x+bi) < 0)
										continue;
									if((com_x+bi) >= asic_nx)
										continue;
									if((com_y+bj) < 0)
										continue;
									if((com_y+bj) >= asic_ny)
										continue;
                                    
                                    // Position of this point in data stream
									thisx = com_x + bi + mi*asic_nx;
									thisy = com_y + bj + mj*asic_ny;
									e = thisx + thisy*pix_nx;
                                    
                                    // If over ADC threshold, this might be another peak (ignore)
                                    if (temp[e] > global->hitfinderADC)
                                        continue;
                                    
                                    // Use this point to estimate standard deviation of local background signal 
                                    np_sigma++;
                                    sum += temp[e];
                                    sumsquared += temp[e]*temp[e];
                                }
                            }
                            
                            // Standard deviation
                            if (counter == 0)
                                localSigma = 0;
                            else 
                                localSigma = sqrt(sumsquared/np_sigma - (sum*sum/(np_sigma*np_sigma)));
                            
                            
                            // Skip this 'peak' if signal to noise criterion is not met 
                            if( localSigma*global->hitfinderMinSNR > totI)
                                continue;
                        }
                        

                        // This is a peak? If so, add info to peak list
						if(nat>=global->hitfinderMinPixCount && nat<=global->hitfinderMaxPixCount && totI > localSigma*global->hitfinderMinSNR ) {
							
							eventData->peakNpix += nat;
							eventData->peakTotal += totI;
							
							
							// Only space to save the first NpeaksMax peaks
							// Make sure we don't overflow this buffer whilst also retaining peak count
							if ( counter >= global->hitfinderNpeaksMax ) {
								counter++;
								continue;
							}
							
							// Remember peak information
							if (counter < global->hitfinderNpeaksMax) {
								eventData->peak_intensity[counter] = totI;
								eventData->peak_com_x[counter] = peak_com_x/totI;
								eventData->peak_com_y[counter] = peak_com_y/totI;
								eventData->peak_npix[counter] = nat;
								
								e = lrint(peak_com_x/totI) + lrint(peak_com_y/totI)*pix_nx;
								eventData->peak_com_index[counter] = e;
								eventData->peak_com_x_assembled[counter] = global->detector[detID].pix_x[e];
								eventData->peak_com_y_assembled[counter] = global->detector[detID].pix_y[e];
								eventData->peak_com_r_assembled[counter] = global->detector[detID].pix_r[e];
								counter++;
							}
						}
					}
				}
			}
		}
	}	
	

    eventData->nPeaks = counter;
    
	// Now figure out whether this is a hit
	if(counter >= global->hitfinderNpeaks && counter <= global->hitfinderNpeaksMax)
		hit = 1;	
    
	
	free(temp);
	free(mask);
	free(inx);
	free(iny);
    
    return(hit);
	
}


/*	
 *	peakfinder 5
 *	Rick Kirian
 */
int peakfinder5(cGlobal *global, cEventData *eventData, int detID) {

	long  nat;
	long  lastnat;
	long  counter = 0;
	int   hit = 0;
	//float total = 0;
	int   search_x[] = {-1,0,1,-1,1,-1,0,1};
	int   search_y[] = {-1,-1,-1,0,0,1,1,1};
	int   search_n = 8;
	long  e,fs,ss;
	long  *inx = (long *) calloc(global->detector[detID].pix_nn, sizeof(long));
	long  *iny = (long *) calloc(global->detector[detID].pix_nn, sizeof(long));
	float totI;
	float peak_com_x;
	float peak_com_y;
	long  thisx;
	long  thisy;
	float grad;
	float lbg, imbg; /* local background nearby peak */
	float *lbg_buffer;
	int   fsmin, fsmax, ssmin, ssmax;
	int   lbg_ss, lbg_fs, lbg_e;
	int   thisfs, thisss;
	float mingrad = global->hitfinderMinGradient*2;
	mingrad *= mingrad;

	/* Combined mask */
	int * mask = (int *) calloc(global->detector[detID].pix_nn, sizeof(int) );
	memcpy(mask,global->hitfinderResMask,global->detector[detID].pix_nn*sizeof(int));
	for (long i=0; i<global->detector[detID].pix_nn; i++) mask[i] *= 
		global->detector[detID].hotpixelmask[i] *
		global->detector[detID].badpixelmask[i] *
		global->detector[detID].peakmask[i] *
		eventData->detector[detID].saturatedPixelMask[i];

	// zero out bad pixels in temporary intensity map
	float * temp = (float *) calloc(global->detector[detID].pix_nn,sizeof(float));
	for (long i=0; i<global->detector[detID].pix_nn; i++) temp[i] = eventData->detector[detID].corrected_data[i]*mask[i];

	// Loop over modules (8x8 array)
	for(long mj=0; mj<global->detector[detID].nasics_y; mj++){
	for(long mi=0; mi<global->detector[detID].nasics_x; mi++){	
	// Loop over pixels within a module
	for(long j=1; j<global->detector[detID].asic_ny-1; j++){
	for(long i=1; i<global->detector[detID].asic_nx-1; i++){

		ss = (j+mj*global->detector[detID].asic_ny);
		fs = i+mi*global->detector[detID].asic_nx;
		e = ss*global->detector[detID].pix_nx + fs;

		if ( global->hitfinderResMask[e] != 1 ) continue;

		if ( temp[e] < global->hitfinderADC ) continue;

		if ( global->hitfinderCheckGradient == 1 ){
		
			float dx1, dx2, dy1, dy2, dxs, dys;
			
			/* can't measure gradient where bad pixels present */
			if ( global->detector[detID].badpixelmask[e] != 1 ) continue;
			if ( global->detector[detID].badpixelmask[e+1] != 1 ) continue;
			if ( global->detector[detID].badpixelmask[e-1] != 1 ) continue;
			if ( global->detector[detID].badpixelmask[e+global->detector[detID].pix_nx] != 1 ) continue;
			if ( global->detector[detID].badpixelmask[e-global->detector[detID].pix_nx] != 1 ) continue;

			/* Get gradients */
			dx1 = temp[e] - temp[e+1];
			dx2 = temp[e-1] - temp[e];
			dy1 = temp[e] - temp[e+global->detector[detID].pix_nx];
			dy2 = temp[e-global->detector[detID].pix_nx] - temp[e];
			/* this is sort of like the mean squared gradient, times 4... */
			dxs = ((dx1*dx1) + (dx2*dx2)) ;
			dys = ((dy1*dy1) + (dy2*dy2)) ;	
			grad = dxs + dys;

			if ( grad < mingrad ) continue;
		}

		lbg = 0; /* local background value */
		if ( global->hitfinderSubtractLocalBG == 1 ) {
			int lbg_counter = 0;
			/* region nearby the peak */
			fsmin = fs - global->hitfinderLocalBGRadius;
			fsmax = fs + global->hitfinderLocalBGRadius;
			ssmin = ss - global->hitfinderLocalBGRadius;
			ssmax = ss + global->hitfinderLocalBGRadius;
			/* check module bounds */
			if ( fsmin < mi*global->detector[detID].asic_nx ) fsmin = mi*global->detector[detID].asic_nx;
			if ( fsmax >= (mi+1)*global->detector[detID].asic_nx ) fsmax = (mi+1)*global->detector[detID].asic_nx - 1; 
			if ( ssmin < mj*global->detector[detID].asic_ny ) ssmin = mj*global->detector[detID].asic_ny;
			if ( ssmax >= (mj+1)*global->detector[detID].asic_ny ) ssmax = (mj+1)*global->detector[detID].asic_ny - 1;
			/* buffer for calculating median */
			lbg_buffer = (float *) calloc((fsmax-fsmin+1)*(ssmax-ssmin+1),sizeof(float));
			/* now calculate median */
			for ( lbg_ss = ssmin; lbg_ss <= ssmax; lbg_ss++) {
			for ( lbg_fs = fsmin; lbg_fs <= fsmax; lbg_fs++ ) {
				thisss = (j+mj*global->detector[detID].asic_ny)*global->detector[detID].pix_nx;
				thisfs = i+mi*global->detector[detID].asic_nx;
				lbg_e = thisss + thisfs;
				/* check if we're ignoring this pixel*/ 
				if ( global->detector[detID].badpixelmask[lbg_e] == 1 ) {
					lbg_buffer[lbg_counter] = temp[lbg_e];
					lbg_counter++;		
				}
			}}
			if ( lbg_counter > 0 )
				lbg = kth_smallest(lbg_buffer,lbg_counter,lbg_counter/2);	
			free(lbg_buffer);
			if ( (temp[e]-lbg) > global->hitfinderADC ) continue;								
		}

		inx[0] = i;
		iny[0] = j;
		nat = 1;
		totI = 0; 
		peak_com_x = 0; 
		peak_com_y = 0; 
		int badpix = 0;

		// start counting bad pixels
		if ( mask[e] == 0 ) badpix += 1;

		// Keep looping until the pixel count within this peak does not change
		do {
			
			lastnat = nat;
			// Loop through points known to be within this peak
			for(long p=0; p<nat; p++){
				// Loop through search pattern
				for(long k=0; k<search_n; k++){

					// Array bounds check
					if((inx[p]+search_x[k]) < 0) continue;
					if((inx[p]+search_x[k]) >= global->detector[detID].asic_nx) continue;
					if((iny[p]+search_y[k]) < 0) continue;
					if((iny[p]+search_y[k]) >= global->detector[detID].asic_ny) continue;
					
					// Neighbour point 
					thisx = inx[p]+search_x[k]+mi*global->detector[detID].asic_nx;
					thisy = iny[p]+search_y[k]+mj*global->detector[detID].asic_ny;
					e = thisx + thisy*global->detector[detID].pix_nx;
					
					// count bad pixels within or neighboring this peak
					if ( mask[e] == 0 ) badpix += 1;

					// Above threshold?
					imbg = temp[e] - lbg; /* "intensitiy minus background" */
					if(imbg > global->hitfinderADC){
						totI += imbg; // add to integrated intensity
						peak_com_x += imbg*( (float) thisx ); // for center of mass x
						peak_com_y += imbg*( (float) thisy ); // for center of mass y
						temp[e] = 0; // zero out this intensity so that we don't count it again
						inx[nat] = inx[p]+search_x[k];
						iny[nat] = iny[p]+search_y[k];
						nat++;
					}
				}
			}
		} while(lastnat != nat);

		// Peak or junk?
		if( nat>=global->hitfinderMinPixCount 
			  && nat<=global->hitfinderMaxPixCount 
		     && badpix==0 ) {
			
			eventData->peakNpix += nat;
			eventData->peakTotal += totI;
			
			// Only space to save the first NpeaksMax peaks
			// (more than this and the pattern is probably junk)
			if ( counter > global->hitfinderNpeaksMax ) {
				eventData->nPeaks = counter;
				return 0;
			}
			
			// Remember peak information
			eventData->peak_intensity[counter] = totI;
			eventData->peak_com_x[counter] = peak_com_x/totI;
			eventData->peak_com_y[counter] = peak_com_y/totI;
			eventData->peak_npix[counter] = nat;

			e = lrint(peak_com_x/totI) + lrint(peak_com_y/totI)*global->detector[detID].pix_nx;
			eventData->peak_com_index[counter] = e;
			eventData->peak_com_x_assembled[counter] = global->detector[detID].pix_x[e];
			eventData->peak_com_y_assembled[counter] = global->detector[detID].pix_y[e];
			eventData->peak_com_r_assembled[counter] = global->detector[detID].pix_r[e];
			counter++;
			
		}
	}}}}	
	eventData->nPeaks = counter;

	/* check peak separations?  get rid of clusters? */
	if ( global->hitfinderCheckPeakSeparation == 1 ) {
		
		int peakNum;
		int peakNum1;
		int peakNum2;
		float diffX,diffY;
		float maxPeakSepSq = global->hitfinderMinPeakSeparation*global->hitfinderMinPeakSeparation;
		float peakSepSq;
		
		/* all peaks assumed "good" to start */
		for ( peakNum = 0; peakNum < eventData->nPeaks; peakNum++ )
			eventData->good_peaks[peakNum] = 1;
		
		/* loop through unique peak pairs, checking that they are not too close */
		for ( peakNum1 = 0; peakNum1 < eventData->nPeaks - 1; peakNum1++ ) {
			if ( eventData->good_peaks[peakNum1] == 0 ) continue;
			for (peakNum2 = peakNum1 + 1; peakNum2 < eventData->nPeaks; peakNum2++ ) {
				if ( eventData->good_peaks[peakNum2] == 0 ) continue;
				/* check the distance between these two peaks */
				diffX = eventData->peak_com_x[peakNum1] - eventData->peak_com_x[peakNum2];
				diffY = eventData->peak_com_y[peakNum1] - eventData->peak_com_y[peakNum2];
				peakSepSq = diffX*diffX + diffY*diffY;
				if ( peakSepSq < maxPeakSepSq ) {
					if (eventData->peak_intensity[peakNum1] > eventData->peak_intensity[peakNum2]) 
					eventData->good_peaks[peakNum2] = 0;
					else 
					eventData->good_peaks[peakNum2] = 0;
				}
			}
		}
		/* now repopulate the peak list with good ones */
		int gpc = 0;
		for ( peakNum = 0; peakNum < eventData->nPeaks; peakNum++ ) {
			if ( eventData->good_peaks[peakNum] == 1 ) {
				eventData->peak_com_x[gpc] = eventData->peak_com_x[peakNum];
				eventData->peak_com_y[gpc] = eventData->peak_com_y[peakNum];
				eventData->peak_com_x_assembled[gpc] = eventData->peak_com_x_assembled[peakNum];
				eventData->peak_com_y_assembled[gpc] = eventData->peak_com_y_assembled[peakNum];
				eventData->peak_com_r_assembled[gpc] = eventData->peak_com_r_assembled[peakNum];
				eventData->peak_com_index[gpc] = eventData->peak_com_index[peakNum];
				eventData->peak_intensity[gpc] = eventData->peak_intensity[peakNum];
				eventData->peak_npix[gpc] =eventData->peak_npix[peakNum];
				gpc++;
			}
		}
		counter = gpc;
	}	
	
	eventData->nPeaks = counter;

	// Now figure out whether this is a hit
	if(counter >= global->hitfinderNpeaks && counter <= global->hitfinderNpeaksMax)
		hit = 1;	

	free(inx); 			
	free(iny);
	free(mask);
	free(temp);

	return(hit);
	
}

/*
 *	Peak finder 6
 *	Rick Kirian
 */
int peakfinder6(cGlobal *global, cEventData *eventData, int detID) {
	
	int counter = 0;
	int hit = 0;
	int fail;
	int stride = global->detector[detID].pix_nx;
	int fs,ss,e,thise,p,ce,ne,nat,lastnat,cs,cf;
	int peakindex,newpeak;
	float dist, itot, ftot, stot;	
	float thisI,snr,bg,bgsig;
	
	/* For counting neighbor pixels */
	int * nexte = (int *) calloc(global->detector[detID].pix_nn,sizeof(int));	
	
	/* Shift in linear indices to eight nearest neighbors */
	int shift[8] = { +1, -1, +stride, -stride,
		+stride - 1, +stride + 1,
		-stride - 1, -stride + 1};
	
	/* Combined mask */
	int * mask = (int *) calloc(global->detector[detID].pix_nn, sizeof(int) );
	memcpy(mask,global->hitfinderResMask,global->detector[detID].pix_nn*sizeof(int));
	for (long i=0; i<global->detector[detID].pix_nn; i++) mask[i] *= 
		global->detector[detID].hotpixelmask[i] *
		global->detector[detID].badpixelmask[i] *
		global->detector[detID].peakmask[i] *
		eventData->detector[detID].saturatedPixelMask[i];
	
	/* Combined mask for pixel counting */
	int * natmask = (int *) calloc(global->detector[detID].pix_nn, sizeof(int) );
	memcpy(natmask,mask,global->detector[detID].pix_nn*sizeof(int));
	
	// zero out bad pixels in temporary intensity map
	float * temp = (float *) calloc(global->detector[detID].pix_nn,sizeof(float));
	for (long i=0; i<global->detector[detID].pix_nn; i++) temp[i] = eventData->detector[detID].corrected_data[i]*mask[i];
	
	// Loop over modules (8x8 array)
	for(long mj=0; mj<global->detector[detID].nasics_y; mj++){
		for(long mi=0; mi<global->detector[detID].nasics_x; mi++){	
			
			/* Some day, the local background radius may be different 
			 * for each panel.  Could even be specified for each pixel
			 * when detector geometry is determined */	
			int bgrad = global->hitfinderLocalBGRadius;
			
			int asic_min_fs = mi*global->detector[detID].asic_nx;
			int asic_min_ss = mj*global->detector[detID].asic_ny;

			int padding = bgrad + global->hitfinderLocalBGThickness - 1;
			
			// Loop over pixels within a module
			for(long j=padding; j<global->detector[detID].asic_ny-1-padding; j++){
				for(long i=padding; i<global->detector[detID].asic_nx-1-padding; i++){
					
					ss = asic_min_ss + j;
					fs = asic_min_fs + i;
					e = ss*stride + fs;
					
					/* Check simple intensity threshold first */
					if ( temp[e] < global->hitfinderADC ) continue;
					
					/* Check if this pixel value is larger than all of its neighbors */
					for ( int k=0; k<8; k++ ) if ( temp[e] <= temp[e+shift[k]] ) continue;
				
					/* get SNR for this pixel */
					fail = box_snr(temp, mask, e, bgrad, global->hitfinderLocalBGThickness, stride, &snr, &bg, &bgsig);
					if ( fail ) continue;

					/* Check SNR threshold */
					if ( snr < global->hitfinderMinSNR ) continue;

					/* Count the number of connected pixels and centroid. */
					nat = 1;
					nexte[0] = e;
					ce = 0;
					itot = 0; ftot = 0; stot = 0;
					do {
						lastnat = nat;
						thise = nexte[ce];
						// if ( natmask[thise] == 0 ) goto skipme;
						for ( int k=0; k<8; k++ ) {
							/* this is the index of a neighboring pixel */
							ne = thise + shift[k];
							/* Array bounds check */
							if ( ne < 0 || ne >= global->detector[detID].pix_nn ) continue;
							// Check that we aren't recounting the same pixel
							if ( natmask[ne] == 0 ) continue;
							/* Check SNR condition */
							if ( (temp[ne]-bg)/bgsig > global->hitfinderMinSNR ) {
								natmask[ne] = 0; /* Mask this pixel (don't count it again) */
								nexte[nat] = ne; /* Queue this location to search it's neighbors later */
								nat++; /* Increment the number of connected pixels */
								/* Track some info needed for rough center of mass: */
								thisI = temp[ne] - bg; 
								itot += thisI;
								cf = ne % stride;
								cs = ne / stride;
								ftot += thisI*(float)cf;
								stot += thisI*(float)cs;						
							}
						}
						ce++;
					} while ( nat != lastnat );
					
					/* Final check that we satisfied the connected pixel requirement */
					if ( nat < global->hitfinderMinPixCount || nat > global->hitfinderMaxPixCount ) continue;

					/* Approximate center of mass */
					fs = lrint(ftot/itot);
					ss = lrint(stot/itot);

					/* Have we already found better peak nearby? */
					newpeak = 1;
					peakindex = counter;
					for ( p=counter-1; p >= 0; p-- ) {
						/* Distance to neighbor peak */
						dist = pow(fs - eventData->peak_com_x[p],2) + 
						pow(ss - eventData->peak_com_y[p], 2);
						if ( dist <= global->hitfinderMinPeakSeparation ) {
							if ( snr > eventData->peak_snr[p]) {
								/* This peak will overtake its neighbor */ 
								newpeak = 0;
								peakindex = p; 
								continue;
							} else {
								/* There is a better peak nearby */
								goto skipme; 
							}
						} 
					}
					
					/* Now find proper centroid */
					itot = 0; ftot = 0; stot = 0;
					for ( cs=ss-bgrad; cs<=ss+bgrad; cs++) {
						for ( cf=fs-bgrad; cf<=fs+bgrad; cf++) {
							ce = cs*stride + cf;
							if ( ce < 0 || ce > global->detector[detID].pix_nn ) continue;
							if ( mask[ce] == 0 ) continue;
							thisI = temp[ce] - bg;
							itot += thisI;
							ftot += thisI*(float)cf;
							stot += thisI*(float)cs;
						}
					}		

					/* Dump peak info into thread structure, for writing hdf5 files, etc. */
					eventData->peak_intensity[peakindex] = itot;
					eventData->peak_com_x[peakindex] = ftot/itot;
					eventData->peak_com_y[peakindex] = stot/itot;
					eventData->peak_npix[peakindex] = 1;
					eventData->peak_snr[peakindex] = snr;
					eventData->peak_com_index[peakindex] = e;
					eventData->peak_com_x_assembled[peakindex] = global->detector[detID].pix_x[e];
					eventData->peak_com_y_assembled[peakindex] = global->detector[detID].pix_y[e];
					eventData->peak_com_r_assembled[peakindex] = global->detector[detID].pix_r[e];
				
					/* Note that we only increment the peak counter if this is a new one */
					if ( newpeak ) counter++;
					
					/* Have we found too many peaks? */
					if ( counter >= global->hitfinderNpeaksMax ) {
						eventData->nPeaks = global->hitfinderNpeaksMax;
						printf("MESSAGE: Found too many peaks - aborting peaksearch early.\n");
						hit = 0;
						goto nohit;
					}
					
				skipme:;
					
				}}
		}}	
	
	eventData->nPeaks = counter;
	
	if(eventData->nPeaks >= global->hitfinderNpeaks && 
	   eventData->nPeaks <= global->hitfinderNpeaksMax)
		hit = 1;
	
nohit:
	
	free(nexte);
	free(mask);
	free(natmask);
	free(temp);
	
	return(hit);
	
}




/* Calculate signal-to-noise ratio for the central pixel, using a square 
 * concentric annulus */

int box_snr(float * im, int * mask, int center, int radius, int thickness,
            int stride, float * SNR, float * background, float * backgroundSigma)
{

	int i, q, a, b, c, d, bgcount,thisradius;
	float bg,bgsq,bgsig,snr;	
	
	bg = 0;
	bgsq = 0;
	bgcount = 0;

	/* Number of pixels in the square annulus */
	int inpix = 2*(radius-1) + 1;
	inpix = inpix*inpix;
	int outpix = 2*(radius+thickness-1) + 1;
	outpix = outpix*outpix;
	int maxpix = outpix - inpix;

	/* Loop over pixels in the annulus */
	for (i=0; i<thickness; i++) {

		thisradius = radius + i;
	
		/* The starting indices at each corner of a square ring */
		int topstart = center - thisradius*(1+stride);
		int rightstart = center + thisradius*(stride-1);
		int bottomstart = center + thisradius*(1+stride);
		int leftstart = center + thisradius*(1-stride);

		/* Loop over pixels in a thin square ring */
		for (q=0; q < thisradius*2; q++) {
			a = topstart + q*stride;
			b = rightstart + q;
			c = bottomstart - q*stride;
			d = leftstart - q;
			bgcount += mask[a] + mask[b] + mask[c] + mask[d];
			bg += im[a] + im[b] + im[c] + im[d];
			bgsq += im[a]*im[a] + im[b]*im[b] + 
			im[c]*im[c] + im[d]*im[d];
		}
	}

	/* Assert that 50 % of pixels in the annulus are good */
	if ( bgcount < 0.5*maxpix ) {
		return 1;
	};

	/* Final statistics */
	bg = bg/bgcount;
	bgsq = bgsq/bgcount;
	bgsig = sqrt(bgsq - bg*bg);
	snr = ( im[center] - bg) / bgsig;

	*SNR = snr;
	*background = bg;
	*backgroundSigma = bgsig;

	return 0;

}

