
#ifndef HITFINDERS_H
#define HITFINDERS_H

int box_snr(float * image, int * mask, int center, int radius, int thickness, int stride, float * SNR, float * background, float * backgroundSigma);

#endif
