
#ifndef HITFINDERS_H
#define HITFINDERS_H

int box_snr(float * image, uint16_t * mask,uint16_t combined_pixel_options, int center, int radius, int thickness, int stride, float * SNR, float * background, float * backgroundSigma);

#endif
