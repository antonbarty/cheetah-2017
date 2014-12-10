#pragma once

#include <pthread.h>
#include <vector>

class ProcessRateMonitor{
 public:
	// averagingTime in seconds
	// ringSize should be bigger than expected data rate * averagingTime
	ProcessRateMonitor(double averagingTime = 30, int ringSize = 20000);
	void frameFinished();
	double getRate();
 private:
	void ringInsert(double time);
    double ringGet(int index);
	void ringSet(int index, double time);
	int ringPos;
	double averagingTime;	
	std::vector<double> timeRingBuffer;
	pthread_mutex_t ringMutex;
};

