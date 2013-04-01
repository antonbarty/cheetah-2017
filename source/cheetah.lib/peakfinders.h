//
//  peakfinders.h
//  cheetah
//
//  Created by Anton Barty on 23/3/13.
//
//

#ifndef cheetah_peakfinders_h
#define cheetah_peakfinders_h
int peakfinder3(cGlobal*, cEventData*, int);
int peakfinder5(cGlobal*, cEventData*, int);
int peakfinder6(cGlobal*, cEventData*, int);

int box_snr(float * image, int * mask, int center, int radius, int thickness, int stride, float * SNR, float * background, float * backgroundSigma);

#endif
