//
//  peakfinders.cpp
//  cheetah
//
//  Created by Anton Barty on 23/3/13.
//
//

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
#include "peakfinders.h"


/*
 *	Create arrays for remembering Bragg peak data
 */
void allocatePeakList(tPeakList *peaks, long NpeaksMax) {
	peaks->peak_maxintensity = (float *) calloc(NpeaksMax, sizeof(float));
	peaks->peak_totalintensity = (float *) calloc(NpeaksMax, sizeof(float));
	peaks->peak_snr = (float *) calloc(NpeaksMax, sizeof(float));
	peaks->peak_npix = (float *) calloc(NpeaksMax, sizeof(float));
	peaks->peak_com_x = (float *) calloc(NpeaksMax, sizeof(float));
	peaks->peak_com_y = (float *) calloc(NpeaksMax, sizeof(float));
	peaks->peak_com_index = (long *) calloc(NpeaksMax, sizeof(long));
	peaks->peak_com_x_assembled = (float *) calloc(NpeaksMax, sizeof(float));
	peaks->peak_com_y_assembled = (float *) calloc(NpeaksMax, sizeof(float));
	peaks->peak_com_r_assembled = (float *) calloc(NpeaksMax, sizeof(float));
}

/*
 *	Clean up unused arrays
 */
void freePeakList(tPeakList *peaks) {
	free(peaks->peak_maxintensity);
	free(peaks->peak_totalintensity);
	free(peaks->peak_snr);
	free(peaks->peak_npix);
	free(peaks->peak_com_x);
	free(peaks->peak_com_y);
	free(peaks->peak_com_index);
	free(peaks->peak_com_x_assembled);
	free(peaks->peak_com_y_assembled);
	free(peaks->peak_com_r_assembled);
}



/*
 *	Peakfinder 3
 *	Count peaks by searching for connected pixels above threshold
 *	Anton Barty
 */
int peakfinder3(cGlobal *global, cEventData *eventData, int detID) {
	
	if(detID > global->nDetectors) {
		printf("peakfinder 3: false detectorID %i\n",detID);
		exit(1);
	}
	
	// Dereference stuff
	long		pix_nx = global->detector[detID].pix_nx;
	//long		pix_ny = global->detector[detID].pix_ny;
	long		pix_nn = global->detector[detID].pix_nn;
	long		asic_nx = global->detector[detID].asic_nx;
	long		asic_ny = global->detector[detID].asic_ny;
	long		nasics_x = global->detector[detID].nasics_x;
	long		nasics_y = global->detector[detID].nasics_y;
	uint16_t	*mask = eventData->detector[detID].pixelmask;
	
	// Variables for this hitfinder
	long	nat = 0;
	long  lastnat = 0;
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
    float maxI;
	float peak_com_x;
	float peak_com_y;
	long thisx;
	long thisy;
	long fs, ss;
	float mingrad = global->hitfinderMinGradient*2;
	mingrad *= mingrad;
	
	nat = 0;
	counter = 0;
	total = 0.0;
	
	
	/*
	 *	Create a buffer for image data so we don't nuke the main image by mistake
	 */
	float *temp = (float*) calloc(pix_nn, sizeof(float));
	memcpy(temp, eventData->detector[detID].corrected_data, pix_nn*sizeof(float));
	
	
	// combine pixelmask bits
	uint16_t combined_pixel_options = PIXEL_IS_IN_PEAKMASK | PIXEL_IS_BAD | PIXEL_IS_HOT | PIXEL_IS_BAD | PIXEL_IS_SATURATED | PIXEL_IS_OUT_OF_RESOLUTION_LIMITS ;
	
	/*
	 *	Apply masks
	 *	(multiply data by 0 to ignore regions)
	 */
	for(long i=0;i<pix_nn;i++){
		temp[i] *= isNoneOfBitOptionsSet(mask[i], combined_pixel_options);
	}
	
	
    
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
					  printf("Array bounds error: e=%li\n",e);
					  exit(1);
					}
					
					if(temp[e] > global->hitfinderADC){
						// This might be the start of a peak - start searching
						inx[0] = i;
						iny[0] = j;
						nat = 1;
						totI = 0;
                        maxI = 0;
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
                                        if (temp[e] > maxI)
                                            maxI = temp[e];
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
                            //if( totI < localSigma*global->hitfinderMinSNR )
                            //    continue;
                            if( maxI < localSigma*global->hitfinderMinSNR )
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
	
	
    eventData->nPeaks = (int) counter;
    
	// Now figure out whether this is a hit
	if(counter >= global->hitfinderNpeaks && counter <= global->hitfinderNpeaksMax)
		hit = 1;
    
	
	free(temp);
	free(inx);
	free(iny);
    
    return(hit);
	
}


/*
 *	Peak finder 6
 *	Rick Kirian
 */
int peakfinder6(cGlobal *global, cEventData *eventData, int detID) {
	
	int counter = 0;
	int counter2 = 0;
	int hit = 0;
	int fail;
	int stride = global->detector[detID].pix_nx;
	int i,fs,ss,e,thise,p,p1,p2,ce,ne,nat,lastnat,cs,cf;
	int peakindex,newpeak;
	float dist, itot, ftot, stot;
	float thisI,snr,bg,bgsig;
	uint16_t *mask = eventData->detector[detID].pixelmask;
	float minPeakSepSq =  global->hitfinderMinPeakSeparation * global->hitfinderMinPeakSeparation;
	nat = 0;
	lastnat = 0;
	
	/* For counting neighbor pixels */
	int *nexte = (int *) calloc(global->detector[detID].pix_nn,sizeof(int));
	int *killpeak = (int *) calloc(global->detector[detID].pix_nn,sizeof(int));
	for (i=0; i<global->detector[detID].pix_nn; i++){
		killpeak[i] = 0;
	}
	
	/* Shift in linear indices to eight nearest neighbors */
	int shift[8] = { +1, -1, +stride, -stride,
		+stride - 1, +stride + 1,
		-stride - 1, -stride + 1};
	
	/* Combined mask */
	uint16_t combined_pixel_options = (PIXEL_IS_IN_PEAKMASK | PIXEL_IS_OUT_OF_RESOLUTION_LIMITS | PIXEL_IS_HOT | PIXEL_IS_BAD | PIXEL_IS_SATURATED);
	
	/* Combined mask for pixel counting */
	// natmask == 0 if pixel masked out (bad)
	int * natmask = (int *) calloc(global->detector[detID].pix_nn, sizeof(int) );
	for(long i=0; i<global->detector[detID].pix_nn; i++){
		natmask[i] = isNoneOfBitOptionsSet(mask[i],combined_pixel_options);
	}
	
	// zero out bad pixels in temporary intensity map
	float * temp = (float *) calloc(global->detector[detID].pix_nn,sizeof(float));
	for (long i=0; i<global->detector[detID].pix_nn; i++){
		temp[i] = eventData->detector[detID].corrected_data[i] * isNoneOfBitOptionsSet(mask[i],combined_pixel_options);
	}
	
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
					fail = box_snr(temp, mask, combined_pixel_options, e, bgrad, global->hitfinderLocalBGThickness, stride, &snr, &bg, &bgsig);
					if ( fail ) continue;
					/* Check SNR threshold */
					if ( snr < global->hitfinderMinSNR ) continue;
					
					/* Count the number of connected pixels and centroid. */
					nat = 1;
					nexte[0] = e;
					ce = 0;
					itot = temp[e] - bg;
					cf = e % stride;
					cs = e / stride;
					ftot = itot*(float)cf;
					stot = itot*(float)cs;
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
						if ( dist <= minPeakSepSq ) {
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
							if ( isAnyOfBitOptionsSet(mask[ce],combined_pixel_options) ) continue;
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
					
				}
			}
		}
	}
	
	
	/*
	 *	Final round of killing peaks
	 */
	// Find peaks that are too close together
	if(global->hitfinderMinPeakSeparation > 0 ) {
		for ( p1=0; p1 < counter; p1++) {
			for ( p2=p1+1; p2 < counter; p2++) {
				/* Distance to neighbor peak */
				dist = pow(eventData->peak_com_x[p1] - eventData->peak_com_x[p2],2) +
				pow(eventData->peak_com_y[p1] - eventData->peak_com_y[p2], 2);
				if ( dist <= minPeakSepSq ) {
					if ( eventData->peak_snr[p1] > eventData->peak_snr[p2]) {
						killpeak[p2] = 1;
					}
					else {
						killpeak[p1] = 1;
					}
				}
			}
		}
	}
	
	counter2 = 0;
	for ( p=0; p < counter; p++) {
		if (killpeak[p] == 0) {
			eventData->peak_intensity[counter2] = eventData->peak_intensity[p];
			eventData->peak_com_x[counter2] = eventData->peak_com_x[p];
			eventData->peak_com_y[counter2] = eventData->peak_com_y[p];
			eventData->peak_npix[counter2] = eventData->peak_npix[p] ;
			eventData->peak_snr[counter2] = eventData->peak_snr[p];
			eventData->peak_com_index[counter2] = eventData->peak_com_index[p];
			eventData->peak_com_x_assembled[counter2] = eventData->peak_com_x_assembled[p];
			eventData->peak_com_y_assembled[counter2] = eventData->peak_com_y_assembled[p];
			eventData->peak_com_r_assembled[counter2] = eventData->peak_com_r_assembled[p];
			counter2++;
		}
	}
	eventData->nPeaks = counter2;
	
	if(eventData->nPeaks >= global->hitfinderNpeaks && eventData->nPeaks <= global->hitfinderNpeaksMax)
		hit = 1;
	
nohit:
	
	free(nexte);
	free(natmask);
	free(temp);
	free(killpeak);
	
	return(hit);
	
}




/* Calculate signal-to-noise ratio for the central pixel, using a square
 * concentric annulus */

int box_snr(float * im, uint16_t * mask, uint16_t combined_pixel_options, int center, int radius, int thickness,
            int stride, float * SNR, float * background, float * backgroundSigma)
{
	
	int i, q, a, b, c, d,thisradius;
	int bgcount;
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
			bgcount += isNoneOfBitOptionsSet(mask[a],combined_pixel_options);
			bgcount += isNoneOfBitOptionsSet(mask[b],combined_pixel_options);
			bgcount += isNoneOfBitOptionsSet(mask[c],combined_pixel_options);
			bgcount += isNoneOfBitOptionsSet(mask[d],combined_pixel_options);
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

