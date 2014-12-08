/*
 *  frameBuffer.cpp
 *  cheetah
 *
 *  Created by Max Felix Hantke on 3/11/14.
 *  Copyright 2014 LMB Uppsala University. All rights reserved.
 *
 */


#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <stdlib.h>
#include <iostream>
#include "detectorObject.h"
#include "frameBuffer.h"
#include "median.h"

cFrameBuffer::cFrameBuffer(long pix_nn0, long depth0, int threadSafetyLevel0) {
	pix_nn = pix_nn0;
	depth = depth0;
	threadSafetyLevel = threadSafetyLevel0;
	// initialize buffer
	frames = (float *) calloc(pix_nn*depth, sizeof(float));
	counter = 0;
    median = (float *) calloc(pix_nn, sizeof(float));
    mean = (float *) calloc(pix_nn, sizeof(float));
	std = (float *) calloc(pix_nn, sizeof(float));
	absAboveThresh = (float *) calloc(pix_nn, sizeof(float));
	// Frames scheduling
	n_frame_readers = (long *) calloc(depth,sizeof(long));
	frame_mutexes = (pthread_mutex_t*) calloc(depth, sizeof(pthread_mutex_t));
	for (long j=0; j<depth; j++) {
		pthread_mutex_init(&frame_mutexes[j], NULL);
	}
	filled = false;
	// Median scheduling
	n_median_readers = 0;
	pthread_mutex_init(&median_mutex, NULL);
	median_updated = false;
	// Mean scheduling
	n_mean_readers = 0;
	pthread_mutex_init(&mean_mutex, NULL);
	mean_updated = false;
	// Std scheduling
	n_std_readers = 0;
	pthread_mutex_init(&std_mutex, NULL);
	std_updated = false;
	// absAbovethresh scheduling
	n_absAboveThresh_readers = 0;
	pthread_mutex_init(&absAboveThresh_mutex, NULL);
	absAboveThresh_updated = false;	
}

cFrameBuffer::~cFrameBuffer() {
	free(frames);
	free(median);
	free(std);
	free(absAboveThresh);
	for (long j=0; j<depth; j++) {
		pthread_mutex_destroy(&frame_mutexes[j]);
	}
	free(frame_mutexes);
	free(n_frame_readers);
	pthread_mutex_destroy(&std_mutex);
	pthread_mutex_destroy(&median_mutex);
	pthread_mutex_destroy(&absAboveThresh_mutex);
}

//.........................................//
// Frame write / read scheduler functions
void cFrameBuffer::lockFrameWriters(long frameID) {
	pthread_mutex_lock(&frame_mutexes[frameID]);
	__sync_fetch_and_add(&n_frame_readers[frameID],1);
	pthread_mutex_unlock(&frame_mutexes[frameID]);
}

void cFrameBuffer::lockAllFramesWriters() {
	for (long j = 0; j<depth; j++) lockFrameWriters(j);
}

void cFrameBuffer::unlockFrameWriters(long frameID) {
	__sync_fetch_and_sub(&n_frame_readers[frameID],1);
}

void cFrameBuffer::unlockAllFramesWriters() {
	for (long j = 0; j<depth; j++) unlockFrameWriters(j);
}

void cFrameBuffer::lockFrameReadersAndWriters(long frameID) {
	// Prevent new reader from starting
	pthread_mutex_lock(&frame_mutexes[frameID]);
	// Wait for readers to finish
	while (n_frame_readers[frameID] > 0)
		usleep(10000);
}

void cFrameBuffer::lockAllFramesReadersAndWriters() {
	for (long j = 0; j<depth; j++) lockFrameReadersAndWriters(j);
}

void cFrameBuffer::unlockFrameReadersAndWriters(long frameID) {
	// Allows readers and writers to start
	pthread_mutex_unlock(&frame_mutexes[frameID]);
}

void cFrameBuffer::unlockAllFramesReadersAndWriters() {
	for (long j = 0; j<depth; j++) unlockFrameReadersAndWriters(j);
}

//.........................................//
// std write / read scheduler functions
void cFrameBuffer::lockStdWriters() {
	pthread_mutex_lock(&std_mutex);
	__sync_fetch_and_add(&n_std_readers,1);
	pthread_mutex_unlock(&std_mutex);
}

void cFrameBuffer::unlockStdWriters() {
	__sync_fetch_and_sub(&n_std_readers,1);
}

void cFrameBuffer::lockStdReadersAndWriters() {
	// Prevent new reader from starting
	pthread_mutex_lock(&std_mutex);
	// Wait for readers to finish
	while (n_std_readers > 0)
		usleep(10000);
}

void cFrameBuffer::unlockStdReadersAndWriters() {
	// Allows readers and writers to start
	pthread_mutex_unlock(&std_mutex);
}

//.........................................//
// median write / read scheduler functions
void cFrameBuffer::lockMedianWriters() {
	pthread_mutex_lock(&median_mutex);
	__sync_fetch_and_add(&n_median_readers,1);
	pthread_mutex_unlock(&median_mutex);
}

void cFrameBuffer::unlockMedianWriters() {
	__sync_fetch_and_sub(&n_median_readers,1);
}

void cFrameBuffer::lockMedianReadersAndWriters() {
	// Prevent new reader from starting
	pthread_mutex_lock(&median_mutex);
	// Wait for readers to finish
	while (n_median_readers > 0)
		usleep(10000);
}

void cFrameBuffer::unlockMedianReadersAndWriters() {
	// Allows readers and writers to start
	pthread_mutex_unlock(&median_mutex);
}

//.........................................//
// mean write / read scheduler functions
void cFrameBuffer::lockMeanWriters() {
	pthread_mutex_lock(&mean_mutex);
	__sync_fetch_and_add(&n_mean_readers,1);
	pthread_mutex_unlock(&mean_mutex);
}

void cFrameBuffer::unlockMeanWriters() {
	__sync_fetch_and_sub(&n_mean_readers,1);
}

void cFrameBuffer::lockMeanReadersAndWriters() {
	// Prevent new reader from starting
	pthread_mutex_lock(&mean_mutex);
	// Wait for readers to finish
	while (n_mean_readers > 0)
		usleep(10000);
}

void cFrameBuffer::unlockMeanReadersAndWriters() {
	// Allows readers and writers to start
	pthread_mutex_unlock(&mean_mutex);
}
//.........................................//
// absAboveThresh write / read scheduler functions
void cFrameBuffer::lockAbsAboveThreshWriters() {
	pthread_mutex_lock(&absAboveThresh_mutex);
	__sync_fetch_and_add(&n_absAboveThresh_readers,1);
	pthread_mutex_unlock(&absAboveThresh_mutex);
}

void cFrameBuffer::unlockAbsAboveThreshWriters() {
	__sync_fetch_and_sub(&n_absAboveThresh_readers,1);
}

void cFrameBuffer::lockAbsAboveThreshReadersAndWriters() {
	// Prevent new reader from starting
	pthread_mutex_lock(&absAboveThresh_mutex);
	// Wait for readers to finish
	while (n_absAboveThresh_readers > 0)
		usleep(10000);
}

void cFrameBuffer::unlockAbsAboveThreshReadersAndWriters() {
	// Allows readers and writers to start
	pthread_mutex_unlock(&absAboveThresh_mutex);
}
//.........................................//

long cFrameBuffer::writeNextFrame(float * data) {
	long counter_last = __sync_fetch_and_add(&counter,1);
	long frameID = counter_last % depth;
	if (threadSafetyLevel > 0) lockFrameReadersAndWriters(frameID);
	memcpy(frames+frameID*pix_nn,data,pix_nn*sizeof(float));
	if (threadSafetyLevel > 0) unlockFrameReadersAndWriters(frameID);
	filled = counter >= (depth-1);
	return counter_last;
}

void cFrameBuffer::copyMedian(float * target) {
	if (threadSafetyLevel > 0) lockMedianWriters();
	memcpy(target,median,pix_nn*sizeof(float));
	if (threadSafetyLevel > 0) unlockMedianWriters();
}

void cFrameBuffer::subtractMedian(float * data, uint16_t * mask, int scale,float minAbsMedianOverStdRatio) {
	float	top = 0;
	float	s1 = 0;
	float	s2 = 0;
	float	v1, v2;
	float	factor = 1;
	if (threadSafetyLevel > 0) lockMedianWriters();
	/*
	 *	Find appropriate scaling factor to match background with current image
	 *	Use with care: this assumes background vector is orthogonal to the image vector (which is often not true)
	 */
	if(scale) {
		for(long i=0; i<pix_nn; i++){
			v1 = median[i];
			v2 = data[i];
			
			// Simple inner product gives cos(theta), which is always less than zero
			// Want ( (a.b)/|b| ) * (b/|b|)
			top += v1*v2;
			s1 += v1*v1;
			s2 += v2*v2;
		}
		factor = top/s1;
	}
	// Do the weighted subtraction
	bool flag = false; 
	for(long i=0; i<pix_nn; i++) {
		if(minAbsMedianOverStdRatio > 0.){
			flag = (abs(median[i]/std[i]) >= minAbsMedianOverStdRatio);
			/*if (i==555555){
				printf("median[i]=%g, std[i]=%g, flag=%d\n",median[i],std[i],flag);
				}*/
		} else {
			flag = true;
		}
		if(flag) {
			data[i] -= (factor*median[i]);
		    mask[i] |= PIXEL_IS_PHOTON_BACKGROUND_CORRECTED;
		}
	}
	if (threadSafetyLevel > 0) unlockMedianWriters();
}

void cFrameBuffer::updateMedian(float point) {
	float * buffer = (float *) calloc(depth, sizeof(float));
	if (threadSafetyLevel > 0) {
		lockAllFramesReadersAndWriters();
		lockMedianReadersAndWriters();
	}
	// Loop over all pixels 
	for(long i=0; i<pix_nn; i++) {
		// Create a local array for sorting
		for(long j=0; j< depth; j++) {
			buffer[j] = frames[j*pix_nn+i];
		}
		// Find median value of the temporary array
		median[i] = (float) kth_smallest(buffer, depth, point);
	}
	if (threadSafetyLevel > 0) {
		unlockAllFramesReadersAndWriters();
		unlockMedianReadersAndWriters();
	}
	free (buffer);	
	median_updated = true;
}

void cFrameBuffer::copyAbsAboveThresh(float * target) {
	if (threadSafetyLevel > 0) lockAbsAboveThreshWriters();
	memcpy(target,absAboveThresh,pix_nn*sizeof(float));
	if (threadSafetyLevel > 0) unlockAbsAboveThreshWriters();
}

void cFrameBuffer::updateAbsAboveThresh(float threshold) {
	if (threadSafetyLevel > 0) lockAllFramesReadersAndWriters();
	long * n = (long *) calloc(pix_nn,sizeof(long));
	for (long j=0; j<depth; j++) {
		for (long i=0; i<pix_nn; i++) {
			n[i] += (fabs(frames[j*pix_nn+i])>threshold)?(1):(0);
		}
	}
	if (threadSafetyLevel > 0) unlockAllFramesReadersAndWriters();
	if (threadSafetyLevel > 0) lockAbsAboveThreshReadersAndWriters();
	for (long i=0; i<pix_nn; i++) {
		absAboveThresh[i] = ((float) n[i])/((float) depth);
	}
	if (threadSafetyLevel > 0) unlockAbsAboveThreshReadersAndWriters();
	absAboveThresh_updated = true;
}

void cFrameBuffer::copyStd(float * target) {
	lockStdWriters();
	memcpy(target,std,pix_nn*sizeof(float));
	unlockStdWriters();
}


void cFrameBuffer::updateStd() {
	double v;
	double * sum = (double *) calloc(pix_nn,sizeof(double));
	double * sumsq = (double *) calloc(pix_nn,sizeof(double));
	if (threadSafetyLevel > 0) {
		lockAllFramesReadersAndWriters();
		lockStdReadersAndWriters();
	}
	// Loop over all pixels and frames and sum up
	for(long j=0; j< depth; j++) {
		for(long i=0; i<pix_nn; i++) {
			v = frames[j*pix_nn+i];
			sum[i] += v;
			sumsq[i] += v*v;
		}
	}
	// Calculate standard deviation for all pixels
	for(long i=0; i<pix_nn; i++) {
		std[i] = sqrt(sumsq[i]/depth - (sum[i]/depth)*(sum[i]/depth));
	}
	if (threadSafetyLevel > 0) {
		unlockAllFramesReadersAndWriters();
		unlockStdReadersAndWriters();
	}
	std_updated = true;
	free(sum);
	free(sumsq);
}

void cFrameBuffer::copyMean(float * target) {
	lockMeanWriters();
	memcpy(target,mean,pix_nn*sizeof(float));
	unlockMeanWriters();
}


void cFrameBuffer::updateMean() {
	double * sum = (double *) calloc(pix_nn,sizeof(double));
	if (threadSafetyLevel > 0) {
		lockAllFramesReadersAndWriters();
		lockMeanReadersAndWriters();
	}
	// Loop over all pixels and frames and sum up
	for(long j=0; j< depth; j++) {
		for(long i=0; i<pix_nn; i++) {
			sum[i] += frames[j*pix_nn+i];
		}
	}
	// Calculate mean value for every pixel
	for(long i=0; i<pix_nn; i++) {
		mean[i] = sum[i]/depth;
	}
	if (threadSafetyLevel > 0) {
		unlockAllFramesReadersAndWriters();
		unlockMeanReadersAndWriters();
	}
	mean_updated = true;
	free(sum);
}

void cFrameBuffer::subtractMean(float * data, uint16_t * mask, int scale,float minAbsMeanOverStdRatio) {
	float	top = 0;
	float	s1 = 0;
	float	s2 = 0;
	float	v1, v2;
	float	factor = 1;
	if (threadSafetyLevel > 0) lockMeanWriters();
	/*
	 *	Find appropriate scaling factor to match background with current image
	 *	Use with care: this assumes background vector is orthogonal to the image vector (which is often not true)
	 */
	if(scale) {
		for(long i=0; i<pix_nn; i++){
			v1 = mean[i];
			v2 = data[i];
			
			// Simple inner product gives cos(theta), which is always less than zero
			// Want ( (a.b)/|b| ) * (b/|b|)
			top += v1*v2;
			s1 += v1*v1;
			s2 += v2*v2;
		}
		factor = top/s1;
	}
	// Do the weighted subtraction
	bool flag = false; 
	for(long i=0; i<pix_nn; i++) {
		if(minAbsMeanOverStdRatio > 0.){
			flag = (abs(mean[i]/std[i]) >= minAbsMeanOverStdRatio);
			/*if (i==555555){
				printf("mean[i]=%g, std[i]=%g, flag=%d\n",median[i],std[i],flag);
				}*/
		} else {
			flag = true;
		}
		if(flag) {
			data[i] -= (factor*mean[i]);
		    mask[i] |= PIXEL_IS_PHOTON_BACKGROUND_CORRECTED;
		}
	}
	if (threadSafetyLevel > 0) unlockMeanWriters();
}
