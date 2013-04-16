//
//  peakfinders.h
//  cheetah
//
//  Created by Anton Barty on 23/3/13.
//
//

#ifndef cheetah_peakfinders_h
#define cheetah_peakfinders_h


int box_snr(float * image, int * mask, int center, int radius, int thickness, int stride, float * SNR, float * background, float * backgroundSigma);


class cPeaklist {
public:
	long	    nPeaks;
	float		*peak_maxintensity;		// Maximum intensity in peak
	float		*peak_totalintensity;	// Integrated intensity in peak
	float		*peak_snr;				// Signal-to-noise ratio of peak
	float		*peak_npix;				// Number of pixels in peak
	float		*peak_com_x;			// peak center of mass x (in raw layout)
	float		*peak_com_y;			// peak center of mass y (in raw layout)
	long		*peak_com_index;		// closest pixel corresponding to peak
	float		*peak_com_x_assembled;	// peak center of mass x (in assembled layout)
	float		*peak_com_y_assembled;	// peak center of mass y (in assembled layout)
	float		*peak_com_r_assembled;	// peak center of mass r (in assembled layout)

	long	    nHot;
	float		peakResolution;			// Radius of 80% of peaks
	float		peakResolutionA;		// Radius of 80% of peaks
	float		peakDensity;			// Density of peaks within this 80% figure
	float		peakNpix;				// Number of pixels in peaks
	float		peakTotal;				// Total integrated intensity in peaks
	
public: 
	void cPeaklist;
	void ~cPeaklist;
	void allocate(long);
	void free(void);
};



void cPeakList::cPeakList(void) {
}

void cPeakList::~cPeakList(void) {
	free();
}

/*
 *	Create arrays for remembering Bragg peak data
 */
void cPeakList::allocate(long NpeaksMax) {
	peak_maxintensity = (float *) calloc(NpeaksMax, sizeof(float));
	peak_totalintensity = (float *) calloc(NpeaksMax, sizeof(float));
	peak_snr = (float *) calloc(NpeaksMax, sizeof(float));
	peak_npix = (float *) calloc(NpeaksMax, sizeof(float));
	peak_com_x = (float *) calloc(NpeaksMax, sizeof(float));
	peak_com_y = (float *) calloc(NpeaksMax, sizeof(float));
	peak_com_index = (long *) calloc(NpeaksMax, sizeof(long));
	peak_com_x_assembled = (float *) calloc(NpeaksMax, sizeof(float));
	peak_com_y_assembled = (float *) calloc(NpeaksMax, sizeof(float));
	peak_com_r_assembled = (float *) calloc(NpeaksMax, sizeof(float));
}

/*
 *	Clean up unused arrays
 */
void cPeakList::free(long n) {
	free(peak_maxintensity);
	free(peak_totalintensity);
	free(peak_snr);
	free(peak_npix);
	free(peak_com_x);
	free(peak_com_y);
	free(peak_com_index);
	free(peak_com_x_assembled);
	free(peak_com_y_assembled);
	free(peak_com_r_assembled);
}


#endif
