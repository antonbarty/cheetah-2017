/*
 *	Function prototypes
 */
void *worker(void *);
void assemble2Dimage(cEventData*, cGlobal*, int);

// detectorCorrection.cpp
void subtractDarkcal(cEventData*, cGlobal*);
void applyGainCorrection(cEventData*, cGlobal*);
void applyBadPixelMask(cEventData*, cGlobal*);
void cmModuleSubtract(cEventData*, cGlobal*);
void cmSubtractUnbondedPixels(cEventData*, cGlobal*);
void cmSubtractBehindWires(cEventData*, cGlobal*);


void subtractDarkcal(float*, float*, long);
void applyGainCorrection(float*, float*, long);
void applyBadPixelMask(float*, int16_t*, long);
void cmModuleSubtract(float*, int16_t*, float, long, long, long, long);
void cmSubtractUnbondedPixels(float*, int16_t*, long, long, long, long);
void cmSubtractBehindWires(float*, int16_t*, float, long, long, long, long);


void calculateHotPixelMask(cGlobal*, int);
void killHotpixels(cEventData*, cGlobal*, int);


// backgroundCorrection.cpp
void checkSaturatedPixels(uint16_t*, int16_t*, long, long);

void updateBackgroundBuffer(cEventData*, cGlobal*, int);
void calculatePersistentBackground(cGlobal*, int);
void subtractPersistentBackground(cEventData*, cGlobal*, int);
void subtractLocalBackground(cEventData*, cGlobal*, int);
void checkSaturatedPixels(cEventData *eventData, cGlobal *global);


// saveFrame.cpp
void nameEvent(cEventData*, cGlobal*);
void writeHDF5(cEventData*, cGlobal*);
void writePeakFile(cEventData *eventData, cGlobal *global);
void writeSimpleHDF5(const char*, const void*, int, int, int);


// hitfinders.cpp
int  hitfinder(cEventData*, cGlobal*, int);

// powder.cpp
void addToPowder(cEventData*, cGlobal*, int, int);
void saveRunningSums(cGlobal*, int);
void saveDarkcal(cGlobal*, int);
void saveGaincal(cGlobal*, int);
void savePowderPattern(cGlobal*, int, int);
void writePowderData(char*, void*, int, int, void*, void*, long, long, int);


// RadialAverage.cpp
void addToRadialAverageStack(cEventData*, cGlobal*, int, int);
void saveRadialAverageStack(cGlobal*, int, int);
void saveRadialStacks(cGlobal*);

void calculateRadialAverage(float*, float*, float*, cGlobal*, int);
void calculateRadialAverage(double*, double*, double*, cGlobal*, int);

// median.cpp
int16_t kth_smallest(int16_t*, long, long);

// fudge...
void evr41fudge(cEventData *t, cGlobal *g);
