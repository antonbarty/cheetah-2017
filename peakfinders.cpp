
#include "myana/myana.hh"
#include "myana/main.hh"
#include "myana/XtcRun.hh"
#include "release/pdsdata/cspad/ConfigV1.hh"
#include "release/pdsdata/cspad/ConfigV2.hh"
#include "release/pdsdata/cspad/ConfigV3.hh"
#include "release/pdsdata/cspad/ElementHeader.hh"
#include "release/pdsdata/cspad/ElementIterator.hh"
#include "cspad/CspadTemp.hh"
#include "cspad/CspadCorrector.hh"
#include "cspad/CspadGeometry.hh"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>

#include "setup.h"
#include "worker.h"
#include "peakfinders.h"

int peakfinder6(cGlobal *global, tThreadInfo	*threadInfo) {

	int counter = 0;
	int hit = 0;
	int stride = global->pix_nx;
	int fs,ss,e,p,ce;
	int peakindex,newpeak;
	float dist, tooclose;
	float itot, ftot, stot;	

	/* Calculate these elsewhere */
	threadInfo->peakResolution = 0;
	threadInfo->peakDensity = 0;

	/* Shift in linear indices to nearest neighbor */
	int shift[8] = { +1, -1, +stride, -stride,
	                 +stride - 1, +stride + 1,
						  -stride - 1, -stride + 1};
	
	/* Combined mask */
	int * mask = (int *) calloc(global->pix_nn, sizeof(int) );
	memcpy(mask,global->hitfinderResMask,global->pix_nn*sizeof(int));
	for (long i=0; i<global->pix_nn; i++) mask[i] *= 
		global->hotpixelmask[i] *
		global->badpixelmask[i] *
		threadInfo->saturatedPixelMask[i];

	// zero out bad pixels in temporary intensity map
	float * temp = (float *) calloc(global->pix_nn,sizeof(float));
	for (long i=0; i<global->pix_nn; i++) temp[i] = threadInfo->corrected_data[i]*mask[i];

	// Loop over modules (8x8 array)
	for(long mj=0; mj<8; mj++){
	for(long mi=0; mi<8; mi++){	
	// Loop over pixels within a module
	for(long j=1; j<CSPAD_ASIC_NY-1; j++){
	for(long i=1; i<CSPAD_ASIC_NX-1; i++){

		ss = (j+mj*CSPAD_ASIC_NY);
		fs = i+mi*CSPAD_ASIC_NX;
		e = ss*global->pix_nx + fs;

		if ( temp[e] < global->hitfinderADC ) continue;

		// What's the appropriate radius for the background of this
		// pixel?  Eventually, this comes from geometry.
		int bgrad = 4;

		// Check that we can actually calculate a background from 
		// concentric ring (or box, really) at this location.
		if ( j < bgrad || i < bgrad ||
			j >= CSPAD_ASIC_NY-bgrad || i >= CSPAD_ASIC_NX-bgrad ) continue;

		// Check if this pixel value is larger than all of its neighbors
		for ( int k=0; k<8; k++ ) if ( temp[e] <= temp[e+shift[k]] ) continue;

		// Check SNR (using one-pixel-thick square ring surrounding pixel of interest)
		int bgcount = 0;
		float bg = 0;
		float bgsq = 0;
		float thisI,bgsig,snr;
		int a,b,c,d;
		int topstart = e - bgrad*(1+stride);
		int rightstart = e + bgrad*(stride-1);
		int bottomstart = e + bgrad*(1+stride);
		int leftstart = e + bgrad*(1-stride);
		for (int q=0; q < bgrad*2; q++) {
			a = topstart + q;
			b = rightstart + q;
			c = bottomstart + q;
			d = leftstart + q;
			bgcount += mask[a] + mask[b] + mask[c] + mask[d];
			bg += temp[a] + temp[b] + temp[c] + temp[d];
			bgsq += temp[a]*temp[a] + temp[b]*temp[b] + 
			        temp[c]*temp[c] + temp[d]*temp[d];
		}
		// Skip it if there are less then 7/8 good pixels in the ring
		if ( bgcount < 7*bgrad ) continue;
		bg = bg/bgcount;
		thisI = temp[e] - bg;
		// Recheck intensity now that background is known
		if ( thisI < global->hitfinderADC ) continue;
		bgsq = bgsq/bgcount;
		bgsig = sqrt(bgsq - bg*bg);
		snr = thisI/bgsig;
		// Most importantly, threshold the SNR
		if ( snr < global->hitfinderMinSNR ) continue;

		// Have we already found better peak nearby?
		newpeak = 1;
		peakindex = counter;
		tooclose = 4*bgrad*bgrad;
		for ( p=counter-1; p >= 0; p-- ) {
			dist = pow(fs - threadInfo->peak_com_x[p],2) + pow(ss - threadInfo->peak_com_y[p], 2);
			if ( dist <= tooclose ) {
				if ( snr > threadInfo->peak_snr[p]) { 
					newpeak = 0;
					peakindex = p; 
					continue;
				} else { goto skipme; }
			} 
		}
		
		// Now find proper centroid
		itot = 0; ftot = 0; stot = 0;
		for ( int cs=ss-bgrad; cs<=ss+bgrad; cs++) {
		for ( int cf=fs-bgrad; cf<=fs+bgrad; cf++) {
			ce = cs*stride + cf;
			if ( mask[ce] == 0 ) goto skipme;
			itot += temp[ce];
			ftot += temp[ce]*(float)cf;
			stot += temp[ce]*(float)cs;
		}}	

		threadInfo->peak_intensity[peakindex] = thisI;
		threadInfo->peak_com_x[peakindex] = ftot/itot;
		threadInfo->peak_com_y[peakindex] = stot/itot;
		threadInfo->peak_npix[peakindex] = 1;
		threadInfo->peak_snr[peakindex] = snr;
		threadInfo->peak_com_index[peakindex] = e;
		threadInfo->peak_com_x_assembled[peakindex] = global->pix_x[e];
		threadInfo->peak_com_y_assembled[peakindex] = global->pix_y[e];
		threadInfo->peak_com_r_assembled[peakindex] = global->pix_r[e];
	
		if ( newpeak ) counter++;
	
		if ( counter == global->hitfinderNpeaksMax ) {
				threadInfo->nPeaks = counter;
				printf("MESSAGE: Found too many peaks - aborting peaksearch early.\n");
				return 0;
		}
		
		skipme:;
	
	}}}}	

	threadInfo->nPeaks = counter;

	if(counter >= global->hitfinderNpeaks && counter <= global->hitfinderNpeaksMax)
		hit = 1;

	free(mask);
	free(temp);

	return hit;

}

