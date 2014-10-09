#include <hdf5.h>

/*
 *	Function prototypes
 */
void *worker(void *);

// detectorCorrection.cpp
void subtractDarkcal(cEventData*, cGlobal*);
void applyGainCorrection(cEventData*, cGlobal*);
void applyPolarizationCorrection(cEventData*, cGlobal*);
void applySolidAngleCorrection(cEventData*, cGlobal*);
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

void subtractDarkcal(float*, float*, long);
void applyGainCorrection(float*, float*, long);
void applyPolarizationCorrection(float*, float*, float*, float*, float, double, float, double, long);
void applyAzimuthallySymmetricSolidAngleCorrection(float*, float*, float*, float*, float, double, float, double, long);
void applyRigorousSolidAngleCorrection(float*, float*, float*, float*, float, double, float, double, long);
void applyBadPixelMask(float*, uint16_t*, long);
void cspadModuleSubtract(float*, uint16_t*, float, long, long, long, long);
void cspadSubtractUnbondedPixels(float*, uint16_t*, long, long, long, long);
void cspadSubtractBehindWires(float*, uint16_t*, float, long, long, long, long);
long calculateHotPixelMask(uint16_t*, int16_t*, long, long, long);

void pnccdOffsetCorrection(cEventData*, cGlobal*);
void pnccdFixWiringError(cEventData*, cGlobal*);
void pnccdOffsetCorrection(float*);
void pnccdFixWiringError(float*);
void pnccdLineInterpolation(cEventData*, cGlobal*);
void pnccdLineMasking(cEventData*, cGlobal*);



// backgroundCorrection.cpp
void subtractLocalBackground(cEventData*, cGlobal*);
void subtractRadialBackground(cEventData*, cGlobal*);
void checkSaturatedPixels(cEventData*, cGlobal*);
void checkPnccdSaturatedPixels(cEventData*, cGlobal*);
void subtractPersistentBackground(cEventData*, cGlobal*);
void updateBackgroundBuffer(cEventData*, cGlobal*, int);
void calculatePersistentBackground(cEventData*, cGlobal*);
void initBackgroundBuffer(cEventData*, cGlobal*);
void subtractLocalBackground(float*, long, long, long, long, long);
void subtractRadialBackground(float*, float*, char*, long, float);
void checkSaturatedPixels(uint16_t*, uint16_t*, long, long);
void subtractPersistentBackground(float*, float*, int, long);
void calculatePersistentBackground(float*, int16_t*, long, long, long);
void updateHaloBuffer(cEventData*, cGlobal*,int);
void calculateHaloPixelMask(cEventData*,cGlobal*);
long calculateHaloPixelMask(uint16_t*, uint16_t*, uint16_t*, float*, float, long, long);


// saveFrame.cpp
void nameEvent(cEventData*, cGlobal*);
void writeHDF5(cEventData*, cGlobal*);
void writePeakFile(cEventData *eventData, cGlobal *global);
void writeSimpleHDF5(const char*, const void*, int, int, int);
void writeSimpleHDF5(const char*, const void*, int, int, int, const char*,long);
void writeSpectrumInfoHDF5(const char*, const void*, const void*, int, int, const void*, int, int);


// saveCXI.cpp
void writeCXI(cEventData *info, cGlobal *global);
void writeCXIHitstats(cEventData *info, cGlobal *global);
void writeAccumulatedCXI(cGlobal * global);
void closeCXIFiles(cGlobal * global);
herr_t cheetahHDF5ErrorHandler(hid_t,void *unused);


// assemble2DImage.cpp
void assemble2Dimage(cEventData*, cGlobal*);
void assemble2Dpowder(cGlobal*);
void assemble2Dmask(cEventData*, cGlobal*);
void assemble2Dimage(float*, float*, float*, float*, long, long, long, int);
void assemble2Dimage(int16_t*, float*, float*, float*, long, long, long, int);
void assemble2Dmask(uint16_t*, uint16_t*, float*, float*, long, long, long, int);

// modularDetector.cpp
int moduleCornerIndex(int, int, int);
void stackModulesMask(uint16_t*, uint16_t*, int, int, int, int);
void stackModulesData(float*, float*, int, int, int, int);
void moduleIdentifier(char *, int, int); 
void cornerPositions(float*, float*, float*, float*, float, int, int, int, int);
void basisVectors(float*, float*, float*, float*, int, int, int, int);

// downsample.cpp
void downsample(cEventData *eventData, cGlobal *global);
void downsampleImageConservative(int16_t *img,int16_t *imgXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx,long downsampling);
void downsampleImageConservative(float *img,float *imgXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx,long downsampling);
void downsampleMaskConservative(uint16_t *msk,uint16_t *mskXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx,long downsampling);
void downsampleImageNonConservative(float *img,float *imgXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx, uint16_t *msk,long downsampling);
void downsampleMaskNonConservative(uint16_t *msk,uint16_t *mskXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx,long downsampling);


// hitfinders.cpp
int  hitfinder(cEventData*, cGlobal*);
long hitfinderFastScan(cEventData*, cGlobal*);
void sortPowderClass(cEventData*, cGlobal*);



// peakfinders.cpp
int peakfinder(cGlobal*, cEventData*, int);
int peakfinder3(tPeakList*, float*, char*, long, long, long, long, float, float, long, long, long);
int peakfinder6(tPeakList*, float*, char*, long, long, long, long, float, float, long, long, long, float);
int peakfinder8(tPeakList*, float*, char*, float*, long, long, long, long, float, float, long, long, long);
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


// powder.cpp
void addToPowder(cEventData*, cGlobal*);
void addToPowder(cEventData*, cGlobal*, int, int);
void saveRunningSums(cGlobal*);
void saveRunningSums(cGlobal*, int);
void saveDarkcal(cGlobal*, int);
void saveGaincal(cGlobal*, int);
void savePowderPattern(cGlobal*, int, int);
void writePowderData(char*, void*, int, int, void*, void*, long, long, int);


// histogram.cpp
void addToHistogram(cEventData*, cGlobal*);
void addToHistogram(cEventData*, cGlobal*, int);
void saveHistograms(cGlobal*);
void saveHistogram(cGlobal*, int);



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

// integratePattern.cpp
void integratePattern(cEventData * eventData,cGlobal * global);

// datarate timing
void updateDatarate(cEventData*, cGlobal*);
void updateAvgGMD(cEventData *eventData, cGlobal *global);
