/*
 *  cheetahmodules.h
 *  cheetah
 *
 *  Created by Anton Barty on 7/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#include <hdf5.h>

/*
 *	Function prototypes
 */
void *worker(void *);

// detectorCorrection.cpp
void initDetectorCorrection(cEventData *eventData, cGlobal *global);
void initRaw(cEventData *eventData, cGlobal *global);
void initPixelmask(cEventData *eventData, cGlobal *global);
void subtractDarkcal(cEventData*, cGlobal*);
void applyGainCorrection(cEventData*, cGlobal*);
void applyPolarizationCorrection(cEventData*, cGlobal*);
void applySolidAngleCorrection(cEventData*, cGlobal*);
void setBadPixelsToZero(cEventData*, cGlobal*);
void cspadModuleSubtractMedian(cEventData*, cGlobal*);
void cspadModuleSubtractHistogram(cEventData*, cGlobal*);
void cspadModuleSubtract2(cEventData*, cGlobal*);
void cspadModuleSubtract(cEventData*, cGlobal*, int);
void cspadSubtractUnbondedPixels(cEventData*, cGlobal*);
void cspadSubtractBehindWires(cEventData*, cGlobal*);
void updateHotPixelBuffer(cEventData*, cGlobal*);
void setHotPixelsToZero(cEventData*, cGlobal*);
void photonCount(cEventData*, cGlobal*);

void subtractDarkcal(float*, float*, long);
void applyGainCorrection(float*, float*, long);
void applyPolarizationCorrection(float*, float*, float*, float*, float, double, float, double, long);
void applyAzimuthallySymmetricSolidAngleCorrection(float*, float*, float*, float*, float, double, float, double, long);
void applyRigorousSolidAngleCorrection(float*, float*, float*, float*, float, double, float, double, long);
void setBadPixelsToZero(float*, uint16_t*, long);
void cspadModuleSubtractMedian(float*, uint16_t*, float, long, long, long, long);
void cspadModuleSubtractHistogram(float*, uint16_t*, long, long, long, long, long);
void cspadSubtractUnbondedPixels(float*, long, long, long, long);
void cspadSubtractBehindWires(float*, uint16_t*, float, long, long, long, long);
long calculateHotPixelMask(uint16_t*, int16_t*, long, long, long);
void photonCount(float*, uint16_t*, long, float);

void pnccdModuleSubtract(cEventData*, cGlobal*);
void pnccdOffsetCorrection(cEventData*, cGlobal*);
void pnccdFixWiringError(cEventData*, cGlobal*);
void pnccdLineInterpolation(cEventData*, cGlobal*);
void pnccdLineMasking(cEventData*, cGlobal*);
void pnCcdModuleWiseOrderFilterBackgroundSubtraction(cEventData *eventData, cGlobal *global);
void pnccdModuleSubtract(float*, uint16_t*, int, int, float, float, int);
void pnccdOffsetCorrection(float*, uint16_t*);
void pnccdOffsetCorrection(float*);
void pnccdFixWiringError(float*);


// backgroundCorrection.cpp
void initPhotonCorrection(cEventData *eventData, cGlobal *global);
void subtractLocalBackground(cEventData*, cGlobal*);
void subtractRadialBackground(cEventData*, cGlobal*);
void checkSaturatedPixels(cEventData*, cGlobal*);
void checkSaturatedPixels(uint16_t*, uint16_t*, long, long);
void checkSaturatedPixelsPnccd(uint16_t*, uint16_t*);
void updateBackgroundBuffer(cEventData*, cGlobal*, int);
void subtractPersistentBackground(cEventData*, cGlobal*);
void subtractLocalBackground(float*, long, long, long, long, long);
void subtractRadialBackground(float*, float*, char*, long, float);
void subtractPersistentBackground(float*, float*, int, long);
void updateNoisyPixelBuffer(cEventData*, cGlobal*,int);

// saveFrame.cpp
void nameEvent(cEventData*, cGlobal*);
void writeHDF5(cEventData*, cGlobal*);
void writePeakFile(cEventData*, cGlobal*);
void writeSimpleHDF5(const char*, const void*, long, long, int);
void writeSimpleHDF5(const char*, const void*, long, long, int, const char*,long);
void writeSpectrumInfoHDF5(const char*, const void*, const void*, int, int, const void*, int, int);

// saveCXI.cpp
void writeCXI(cEventData*, cGlobal*);
void writeCXIHitstats(cEventData*, cGlobal*);
void writeAccumulatedCXI(cGlobal*);
void closeCXIFiles(cGlobal*);
void flushCXIFiles(cGlobal*);
herr_t cheetahHDF5ErrorHandler(hid_t,void*);

// assemble2DImage.cpp
void assemble2D(cEventData*, cGlobal*);
void assemble2DPowder(cGlobal*);
void assemble2DImage(float*, float*, float*, float*, long, long, long, int);
void assemble2DImage(int16_t*, float*, float*, float*, long, long, long, int);
void assemble2DMask(uint16_t*, uint16_t*, float*, float*, long, long, long, int);

// modularDetector.cpp
int moduleCornerIndex(int, int, int);
void stackModulesMask(uint16_t*, uint16_t*, int, int, int, int);
void stackModulesData(float*, float*, int, int, int, int);
void moduleIdentifier(char *, int, int); 
void cornerPositions(float*, float*, float*, float*, float, int, int, int, int);
void basisVectors(float*, float*, float*, float*, int, int, int, int);

// downsample.cpp
void downsample(cEventData *eventData, cGlobal *global);
void downsampleImageConservative(int16_t *img,int16_t *imgXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx,long downsampling,int debugLevel);
void downsampleImageConservative(float *img,float *imgXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx,long downsampling,int debugLevel);
void downsampleMaskConservative(uint16_t *msk,uint16_t *mskXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx,long downsampling,int debugLevel);
void downsampleImageNonConservative(float *img,float *imgXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx, uint16_t *msk,long downsampling,int debugLevel);
void downsampleMaskNonConservative(uint16_t *msk,uint16_t *mskXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx,long downsampling,int debugLevel);
void downsamplePowder(cGlobal*);

// hitfinders.cpp
int  hitfinder(cEventData*, cGlobal*);
long hitfinderFastScan(cEventData*, cGlobal*);
void sortPowderClass(cEventData*, cGlobal*);

// peakfinders.cpp
int peakfinder(cGlobal*, cEventData*, int);
int peakfinder3(tPeakList*, float*, char*, long, long, long, long, float, float, long, long, long);
int peakfinder6(tPeakList*, float*, char*, long, long, long, long, float, float, long, long, long, float);
int peakfinder8(tPeakList*, float*, char*, float*, long, long, long, long, float, float, long, long, long);
int peakfinder8old(tPeakList*, float*, char*, float*, long, long, long, long, float, float, long, long, long);
int killNearbyPeaks(tPeakList*, float );

// spectrum.cpp
void addFEEspectrumToStack(cEventData*, cGlobal*, int);
void saveFEEspectrumStack(cGlobal*, int);
void saveSpectrumStacks(cGlobal*);
void integrateSpectrum(cEventData*, cGlobal*);
void integrateSpectrum(cEventData*, cGlobal*, int, int);
void addToSpectrumStack(cEventData*, cGlobal*, int);
void saveEspectrumStacks(cGlobal*);
void saveEspectrumStack(cGlobal*, int);
void genSpectrumBackground(cEventData*, cGlobal*, int, int);
void integrateRunSpectrum(cEventData*, cGlobal*);
void saveIntegratedRunSpectrum(cGlobal*);
void readSpectrumDarkcal(cGlobal*, char *);
void readSpectrumEnergyScale(cGlobal*, char*);

// timetool.cpp
void addTimeToolToStack(cEventData*, cGlobal*, int);
void saveTimeToolStack(cGlobal*, int);
void saveTimeToolStacks(cGlobal*);

// powder.cpp
void addToPowder(cEventData*, cGlobal*);
void addToPowder(cEventData*, cGlobal*, int, long);
void saveRunningSums(cGlobal*);
void saveDarkcal(cGlobal*, int);
void saveGaincal(cGlobal*, int);
void savePowderPattern(cGlobal*, int, int);
void writePowderData(char*, void*, int, int, void*, void*, long, long, int);

// histogram.cpp
void addToHistogram(cEventData*, cGlobal*, int);
void saveHistograms(cGlobal*);
void saveHistogram(cGlobal*, int);
void calculateHistogramScale(long histMin, long histNBins, float histBinSize, float * scaleTarget);

// RadialAverage.cpp
void calculateRadialAverage(cEventData*, cGlobal*);
template <class T>
void calculateRadialAverage(T *data2d, uint16_t *pixelmask2d, T *dataRadial, uint16_t *pixelmaskRadial, float * pix_r, long radial_nn, long pix_nn);
void addToRadialAverageStack(cEventData*, cGlobal*);
void addToRadialAverageStack(cEventData*, cGlobal*, int, int);
void saveRadialAverageStack(cGlobal*, int, int);
void saveRadialStacks(cGlobal*);
void calculateRadialAveragePowder(cGlobal*);

// streakFinderWrapperWrapper.cpp
void initStreakFinder(cGlobal*);
void destroyStreakFinder(cGlobal*);
void streakFinder(cEventData*, cGlobal *);


// median.cpp
int16_t kth_smallest(int16_t*, long, long);

// fudge...
void evr41fudge(cEventData*, cGlobal*);

// integratePattern.cpp
void integratePattern(cEventData * eventData,cGlobal * global);

// datarate timing
void updateDatarate(cGlobal*);

// gmd.cpp
void calculateGmd(cEventData *eventData);
bool gmdBelowThreshold(cEventData *eventData, cGlobal *global);
void updateAvgGmd(cEventData *eventData, cGlobal *global);

// log.cpp
void writeLog(cEventData * eventData, cGlobal * global);
