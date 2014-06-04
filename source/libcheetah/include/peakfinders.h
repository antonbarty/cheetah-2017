//
//  peakfinders.h
//  cheetah
//
//  Created by Anton Barty on 23/3/13.
//
//

#ifndef cheetah_peakfinders_h
#define cheetah_peakfinders_h



typedef struct {
public:
	long	    nPeaks;
	long	    nHot;
	float		peakResolution;			// Radius of 80% of peaks
	float		peakResolutionA;		// Radius of 80% of peaks
	float		peakDensity;			// Density of peaks within this 80% figure
	float		peakNpix;				// Number of pixels in peaks
	float		peakTotal;				// Total integrated intensity in peaks
	int			memoryAllocated;
	long		nPeaks_max;
    
	float		*peak_maxintensity;		// Maximum intensity in peak
	float		*peak_totalintensity;	// Integrated intensity in peak
	float		*peak_sigma;			// Signal-to-noise ratio of peak
	float		*peak_snr;				// Signal-to-noise ratio of peak
	float		*peak_npix;				// Number of pixels in peak
	float		*peak_com_x;			// peak center of mass x (in raw layout)
	float		*peak_com_y;			// peak center of mass y (in raw layout)
	long		*peak_com_index;		// closest pixel corresponding to peak
	float		*peak_com_x_assembled;	// peak center of mass x (in assembled layout)
	float		*peak_com_y_assembled;	// peak center of mass y (in assembled layout)
	float		*peak_com_r_assembled;	// peak center of mass r (in assembled layout)
	float		*peak_com_q;			// Scattering vector of this peak
	float		*peak_com_res;			// REsolution of this peak
} tPeakList;



//void cPeakList::cPeakList(void) {
//}

//void cPeakList::~cPeakList(void) {
//	free();
//}

void allocatePeakList(tPeakList*, long);
void freePeakList(tPeakList);


#endif
