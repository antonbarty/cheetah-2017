#include <sys/time.h>
#include <processRateMonitor.h>

ProcessRateMonitor::ProcessRateMonitor(double avgTime, int ringSize){
	averagingTime = avgTime;
	timeRingBuffer.resize(ringSize);
	for(int i = 0; i < ringSize; i++){
		timeRingBuffer[i] = -1;
	}
	ringPos = 0;
	pthread_mutex_init(&ringMutex,NULL);
}

void ProcessRateMonitor::frameFinished(){
	timeval timevalNow;
	gettimeofday(&timevalNow, NULL);
	double time = timevalNow.tv_sec+timevalNow.tv_usec/1.0e6;
	ringInsert(time);
}

double ProcessRateMonitor::getRate(){
	double wN = 0;
	int curPos = ringPos-1;
	double tEnd;
	// Use exponential weighting to give less 
	// importance to old frame rates
	double wTime = 0;
	double decayPower = 0.99;
	double weight = 1;
	if(curPos < 0 || ringGet(curPos) < 0){
		return 0;
    }else{
		tEnd = ringGet(curPos);
	}
	double nextT = tEnd;
	
	while(true){
		curPos--;
		double t = ringGet(curPos);
		if(t < 0 || t > tEnd){
			break;
		}
		wN += weight;
		wTime += (nextT-t)*weight;
		weight *= decayPower;
		nextT = t;		
		if(tEnd-t > averagingTime){
			break;
		}			   		
	}
	if(wN == 0){
		return 0;
	}
	return wN/wTime;
}

void ProcessRateMonitor::ringInsert(double time){
#ifdef __GNUC__
	int index = __sync_fetch_and_add(&ringPos,1);
	ringSet(index,time);
#else
	pthread_mutex_lock(&ringMutex);
	int index = ringPos;
	ringPos++;
	pthread_mutex_unlock(&ringMutex);
	ringSet(index,time);
#endif
}

double ProcessRateMonitor::ringGet(int index){
	if(index < 0){
		return -1;
	}
	return timeRingBuffer[index%timeRingBuffer.size()];
}


void ProcessRateMonitor::ringSet(int index, double time){
	if(index < 0){
		return;
	}
	timeRingBuffer[index%timeRingBuffer.size()] = time;
}





