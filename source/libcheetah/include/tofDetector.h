#ifndef TOFDETECTOR_H
#define TOFDETECTOR_H

#define MAX_FILENAME_LENGTH 1024

#include <vector>

#define MAX_TOF_DETECTORS 20

class cTOFDetectorCommon {
public:
	cTOFDetectorCommon(){
		channel = 0;
		strcpy(sourceName,"acqirisSource");
		strcpy(sourceIdentifier,"DetInfo(:Acqiris.0)");
		numSamples = 12288;
		configGroup[0] = 0;
		detectorName[0] = 0;
		detectorType[0] = 0;
		description[0] = 0;
		hitfinderMinSample = 0;
		hitfinderMaxSample = 1000;
		hitfinderMeanBackground = 2;
		hitfinderThreshold = 100;
	}
	/** @brief ID of grouped configuration keywords */
	char configGroup[MAX_FILENAME_LENGTH];
	/** @brief Name of the detector */
	char detectorName[MAX_FILENAME_LENGTH];
	/** @brief Type of detector.
	 Should always be "tof" */
	char detectorType[MAX_FILENAME_LENGTH];
	/** @brief Channel to use */
	int channel;
	/** @brief Name of the psana.cfg entry for the signal source */
	char sourceName[MAX_FILENAME_LENGTH];
	/** @brief The psana name of the signal source */
	char sourceIdentifier[MAX_FILENAME_LENGTH];
	/** @brief Human readable (text) description of the detector */
	char description[MAX_FILENAME_LENGTH];

	/** @brief Number of samples to use */
	int numSamples;
	
	/** @brief Voltage threshold of TOF for hitfinding. */
	double hitfinderThreshold;
	/** @brief First sample in the TOF scan to consider. */
	int hitfinderMinSample;
	/** @brief Last sample in the TOF scan to consider. */
	int hitfinderMaxSample;
	/** @brief Mean voltage of TOF signal for TOF hitfinding */
	double hitfinderMeanBackground;

	int parseConfigTag(char*, char*);
 private:
	void configFromName();
};


class cTOFDetectorEvent{
 public:
	std::vector<double> time;
	std::vector<double> voltage;
	double trigTime;
	double slope;
	double offset;
	double sampInterval;
	double timeStamp;
};
#endif
