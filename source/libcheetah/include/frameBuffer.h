/*
 *  frameBuffer.h
 *  cheetah
 *
 *  Created by Max Felix Hantke on 3/11/14.
 *  Copyright 2014 LMB Uppsala University. All rights reserved.
 *
 */

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>

class cFrameBuffer {
 public:
	cFrameBuffer(long pix_nn0, long depth0, int threadSafetyLevel0);
	~cFrameBuffer();
	long writeNextFrame(float * data);
	void copyMedian(float * target);
	void updateMedian(float point);
	void copyStd(float * target);
	void updateStd();
	long pix_nn;
	long depth;
	int threadSafetyLevel;
	long counter;
 private:
	float * frames;
	float * median;
	float * std;
	bool filled,median_calculated,std_calculated;
	pthread_mutex_t * frame_mutexes;
	pthread_mutex_t median_mutex,std_mutex;
	long n_std_readers,n_median_readers;
	long * n_frame_readers;
	// Scheduling reading and writing
	void lockFrameWriters(long frameID);
	void lockAllFramesWriters();
	void unlockFrameWriters(long frameID);
	void unlockAllFramesWriters();
	void lockFrameReadersAndWriters(long frameID);
	void lockAllFramesReadersAndWriters();
	void unlockFrameReadersAndWriters(long frameID);
	void unlockAllFramesReadersAndWriters();
	void lockStdWriters();
	void unlockStdWriters();
	void lockStdReadersAndWriters();
	void unlockStdReadersAndWriters();
	void lockMedianWriters();
	void unlockMedianWriters();
	void lockMedianReadersAndWriters();
	void unlockMedianReadersAndWriters();
};

#endif



