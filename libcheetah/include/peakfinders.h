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


typedef struct {
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
	
} tPeakList;



//void cPeakList::cPeakList(void) {
//}

//void cPeakList::~cPeakList(void) {
//	free();
//}

void freePeakList(tPeakList*);
void allocatePeakList(tPeakList*, long); 


#endif
