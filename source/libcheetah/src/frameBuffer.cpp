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
	std = (float *) calloc(pix_nn, sizeof(float));
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
	std_calculated = false;
	// Std scheduling
	n_std_readers = 0;
	pthread_mutex_init(&std_mutex, NULL);
	median_calculated = false;
}

cFrameBuffer::~cFrameBuffer() {
	free(frames);
	free(median);
	free(std);
	for (long j=0; j<depth; j++) {
		pthread_mutex_destroy(&frame_mutexes[j]);
	}
	free(frame_mutexes);
	free(n_frame_readers);
	pthread_mutex_destroy(&std_mutex);
	pthread_mutex_destroy(&median_mutex);
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

long cFrameBuffer::writeNextFrame(float * data) {
	long counter_last = __sync_fetch_and_add(&counter,1);
	long frameID = counter_last % depth;
	lockFrameReadersAndWriters(frameID);
	memcpy(frames+frameID*pix_nn,data,pix_nn*sizeof(float));
	unlockFrameReadersAndWriters(frameID);
	filled = counter >= (depth-1);
	return counter_last;
}

void cFrameBuffer::copyMedian(float * target) {
	lockMedianWriters();
	memcpy(target,median,pix_nn*sizeof(float));
	unlockMedianWriters();
}


void cFrameBuffer::updateMedian(float point) {
	float * buffer = (float *) calloc(depth, sizeof(float));
	lockAllFramesReadersAndWriters();
	lockMedianReadersAndWriters();
	// Loop over all pixels 
	for(long i=0; i<pix_nn; i++) {
		// Create a local array for sorting
		for(long j=0; j< depth; j++) {
			buffer[j] = frames[j*pix_nn+i];
		}
		// Find median value of the temporary array
		median[i] = (float) kth_smallest(buffer, depth, point);
	}
	unlockAllFramesReadersAndWriters();
	unlockMedianReadersAndWriters();
	free (buffer);	
	median_calculated = true;
}


void cFrameBuffer::copyStd(float * target) {
	lockStdWriters();
	memcpy(target,std,pix_nn*sizeof(float));
	unlockStdWriters();
}


void cFrameBuffer::updateStd() {
	float sum,sumsq,v;
	lockAllFramesReadersAndWriters();
	lockStdReadersAndWriters();
	// Loop over all pixels 
	for(long i=0; i<pix_nn; i++) {
		sum = 0.;
		sumsq = 0.;
		// Loop over depth
		for(long j=0; j< depth; j++) {
			v = frames[j*pix_nn+i];
			sum += v;
			sumsq += v*v;
		}
		// Calculate standard deviation for this pixel
		std[i] = sqrt(sumsq/depth - (sum*sum)/depth);
	}
	unlockAllFramesReadersAndWriters();
	unlockStdReadersAndWriters();
	std_calculated = true;
}
