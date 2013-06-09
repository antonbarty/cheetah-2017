#include <hdf5.h>

/*
 *	Function prototypes
 */
void *worker(void *);

// detectorCorrection.cpp
void subtractDarkcal(cEventData*, cGlobal*);
void applyGainCorrection(cEventData*, cGlobal*);
void applyBadPixelMask(cEventData*, cGlobal*);
void cspadModuleSubtract(cEventData*, cGlobal*);
void cspadModuleSubtract2(cEventData*, cGlobal*);
void cspadModuleSubtract(cEventData*, cGlobal*, int);
void cspadSubtractUnbondedPixels(cEventData*, cGlobal*);
void cspadSubtractBehindWires(cEventData*, cGlobal*);
void calculateHotPixelMask(cGlobal*);
void identifyHotPixels(cEventData*, cGlobal*);
void calculateHotPixelMask(cEventData*, cGlobal*);
void applyHotPixelMask(cEventData*, cGlobal*);
void calculateHaloPixelMask(cEventData*,cGlobal*);
void updateHaloBuffer(cEventData*, cGlobal*,int);

void subtractDarkcal(float*, float*, long);
void applyGainCorrection(float*, float*, long);
void applyBadPixelMask(float*, uint16_t*, long);
void cspadModuleSubtract(float*, uint16_t*, float, long, long, long, long);
void cspadSubtractUnbondedPixels(float*, uint16_t*, long, long, long, long);
void cspadSubtractBehindWires(float*, uint16_t*, float, long, long, long, long);
long calculateHotPixelMask(uint16_t*, int16_t*, long, long, long);
long calculateHaloPixelMask(uint16_t*, uint16_t*, uint16_t*, float*, float, long, long);

void pnccdOffsetCorrection(cEventData*, cGlobal*);
void pnccdFixWiringError(cEventData*, cGlobal*);
void pnccdOffsetCorrection(float*);
void pnccdFixWiringError(float*);

// backgroundCorrection.cpp
void subtractLocalBackground(cEventData*, cGlobal*);
void checkSaturatedPixels(cEventData*, cGlobal*);
void subtractPersistentBackground(cEventData*, cGlobal*);
void updateBackgroundBuffer(cEventData*, cGlobal*, int);
void calculatePersistentBackground(cEventData*, cGlobal*);
void initBackgroundBuffer(cEventData*, cGlobal*);

void subtractLocalBackground(float*, long, long, long, long, long);
void checkSaturatedPixels(uint16_t*, uint16_t*, long, long);
void subtractPersistentBackground(float*, float*, int, long);
void calculatePersistentBackground(float*, int16_t*, long, long, long);

// saveFrame.cpp
void nameEvent(cEventData*, cGlobal*);
void writeHDF5(cEventData*, cGlobal*);
void writePeakFile(cEventData *eventData, cGlobal *global);
void writeSimpleHDF5(const char*, const void*, int, int, int);
void writeSpectrumInfoHDF5(const char*, const void*, const void*, int, int, const void*, int, int);

// saveCXI.cpp
void writeCXI(cEventData *info, cGlobal *global);
void writeAccumulatedCXI(cGlobal * global);
void closeCXIFiles(cGlobal * global);
herr_t cheetahHDF5ErrorHandler(hid_t,void *unused);

// assemble2DImage.cpp
void assemble2Dimage(cEventData*, cGlobal*);
void assemble2Dmask(cEventData*, cGlobal*);
void assemble2Dimage(int16_t*, float*, float*, float*, long, long, long, int);
void assemble2Dmask(uint16_t*, uint16_t*, float*, float*, long, long, long, int);
void downsample(cEventData*, cGlobal*);
void downsampleImage(int16_t*, int16_t*, long, long, long, long);
void downsampleMask(uint16_t*, uint16_t*, long, long, long, long);

// hitfinders.cpp
int  hitfinder(cEventData*, cGlobal*);

// peakfinders.cpp
int peakfinder3(cGlobal*, cEventData*, int);
int peakfinder6(cGlobal*, cEventData*, int);


// spectrum.cpp
void integrateSpectrum(cEventData*, cGlobal*);
void integrateSpectrum(cEventData*, cGlobal*, int, int);
void genSpectrumBackground(cEventData*, cGlobal*, int, int);
void integrateRunSpectrum(cEventData*, cGlobal*);
void saveIntegratedRunSpectrum(cGlobal*);
void readSpectrumDarkcal(cGlobal*, char *);
void readSpectrumEnergyScale(cGlobal*, char*);

// powder.cpp
void addToPowder(cEventData*, cGlobal*);
void addToPowder(cEventData*, cGlobal*, int, int);
void saveRunningSums(cGlobal*, int);
void saveDarkcal(cGlobal*, int);
void saveGaincal(cGlobal*, int);
void savePowderPattern(cGlobal*, int, int);
void writePowderData(char*, void*, int, int, void*, void*, long, long, int);

// RadialAverage.cpp
void calculateRadialAverage(cEventData*, cGlobal*);
void calculateRadialAverage(float*, float*, long, float*, float*, long, int*);
void calculateRadialAverage(double*, double*, double*, cGlobal*, int);
void addToRadialAverageStack(cEventData*, cGlobal*);
void addToRadialAverageStack(cEventData*, cGlobal*, int, int);
void saveRadialAverageStack(cGlobal*, int, int);
void saveRadialStacks(cGlobal*);
    
// median.cpp
int16_t kth_smallest(int16_t*, long, long);

// fudge...
void evr41fudge(cEventData*, cGlobal*);

// datarate timing
void updateDatarate(cEventData*, cGlobal*);

