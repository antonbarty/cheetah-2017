/*
 *  worker.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 6/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "cheetah.h"
#include "cheetahmodules.h"
#include "median.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

/*
 *	Worker thread function for processing each cspad data frame
 */
void *worker(void *threadarg)
{

    // Turn threadarg into a more useful form
    cGlobal *global;
    cEventData *eventData;
    int hit = 0;
    float hitRatio;
    double processRate;
    eventData = (cEventData*) threadarg;
    global = eventData->pGlobal;

    // Take a copy of the calibrated flag, important is the status at the beginning of the worker call
    int calibrated = global->calibrated;

    std::vector< int > myvector;
    std::stringstream sstm;
    std::string result;
    std::ofstream outFlu;
    //std::ios_base::openmode mode;
    std::stringstream sstm1;
    std::ofstream outHit;

    //---------------------------//
    //--------MONITORING---------//
    //---------------------------//
    DEBUG2("Monitoring");

    updateDatarate(global);
    processRate = global->processRateMonitor.getRate();

    //--------------------------------------//
    //---INITIALIZATIONS-AND-PREPARATIONS---//
    //--------------------------------------//
    DEBUG2("Initializations and preparations");

    /*
     *  Inside-thread speed test
     */
    if (global->ioSpeedTest == 3) {
        printf("r%04u:%li (%3.1fHz): I/O Speed test #3 (exiting within thread)\n", global->runNumber, eventData->frameNumber, global->datarate);
        goto cleanup;
    }

    // Nasty fudge for evr41 (i.e. "optical pump laser is on") signal when only 
    // Acqiris data (i.e. temporal profile of the laser diode signal) is available...
    // Hopefully this never happens again... 
    if (global->fudgeevr41 == 1) {
        evr41fudge(eventData, global);
    }

    // Create a unique name for this event
    nameEvent(eventData, global);

    // GMD
    calculateGmd(eventData);
    if (gmdBelowThreshold(eventData, global)) {
        printf("r%04u:%li Skipping frame (GMD below threshold: %f mJ < %f mJ).\n", global->runNumber, eventData->frameNumber, eventData->gmd,
                global->gmdThreshold);
        goto cleanup;
    }

    // Initialise pixelmask with pixelmask_shared
    initPixelmask(eventData, global);

    // Initialise raw data array (float) THIS MIGHT SLOW THINGS DOWN, WE MIGHT WANT TO CHANGE THIS
    initRaw(eventData, global);

    //-------------------------//
    //---DETECTOR-CORRECTION---//
    //-------------------------//
    DEBUG2("Detector correction");

    // Initialise data_detCorr with data_raw16
    initDetectorCorrection(eventData, global);

    // Check for saturated pixels before applying any other corrections
    checkSaturatedPixels(eventData, global);

    // Subtract darkcal image (static electronic offsets)
    subtractDarkcal(eventData, global);

    // If no darkcal file: Subtract persistent background here (background = photon background + static electronic offsets)
    // Commenting this out because it was was causing crashes with memory access violations (and the problem went away when this was commented out) <-- Anton 14 Dec 2014
    //subtractPersistentBackground(eventData, global);

    // Fix CSPAD artefacts:
    // Subtract common mode offsets (electronic offsets)
    // cmModule = 1
    // (these corrections will be automatically skipped for any non-CSPAD detector)
    cspadModuleSubtractMedian(eventData, global);
    cspadModuleSubtractHistogram(eventData, global);
    cspadSubtractUnbondedPixels(eventData, global);
    cspadSubtractBehindWires(eventData, global);

    // Fix pnCCD artefacts:
    // pnCCD offset correction (read out artifacts prominent in lines with high signal)
    // pnCCD wiring error (shift in one set of rows relative to another - and yes, it's a wiring error).
    // pnCCD signal drop in every second line (fast changing dimension) can be fixed by interpolation and/or masking of the affected lines
    //  (these corrections will be automatically skipped for any non-pnCCD detector)
    pnccdModuleSubtract(eventData, global);
    pnccdOffsetCorrection(eventData, global);
    pnccdFixWiringError(eventData, global);
    pnccdLineInterpolation(eventData, global);
    pnccdLineMasking(eventData, global);

    // Apply gain correction
    applyGainCorrection(eventData, global);

    // Zero out bad pixels
    setBadPixelsToZero(eventData, global);

    // Histogram of detector values
    addToHistogram(eventData, global, 0);
    //addToHistogram(eventData, global, hit);

    //  Inside-thread speed test
    if (global->ioSpeedTest == 4) {
        printf("r%04u:%li (%3.1fHz): I/O Speed test 4 (after detector correction)\n", global->runNumber, eventData->frameNumber, global->datarate);
        goto cleanup;
    }

    // Subtract residual common mode offsets (cmModule=2)
    cspadModuleSubtract2(eventData, global);

    // Set bad pixels to zero
    setBadPixelsToZero(eventData, global);

    // Identify hot pixels and set them to zero
    updateHotPixelBuffer(eventData, global);
    setHotPixelsToZero(eventData, global);

    // Inside-thread speed test
    if (global->ioSpeedTest == 5) {
        printf("r%04u:%li (%3.1fHz): I/O Speed test #5 (photon background correction)\n", global->runNumber, eventData->frameNumber, global->datarate);
        goto cleanup;
    }

    //----------------------------------//
    //---PHOTON-BACKGROUND-CORRECTION---//
    //----------------------------------//
    DEBUG2("Background correction");

    // Initialise data_detPhotCorr with data_detCorr
    initPhotonCorrection(eventData, global);

    // Some of these conversions are mutually exclusive
    // For example, it does not make sense to photon count (sparse data), when applying polarisation or
    // solid angle correction (which affect ADU values and not the Poisson photon count)
    // Convert to photons
    photonCount(eventData, global);

    // Apply polarization correction
    applyPolarizationCorrection(eventData, global);

    // Apply solid angle correction
    applySolidAngleCorrection(eventData, global);

    // If a darkcal file is available: Subtract persistent background is for photon subtraction (persistent background = photon background)
    subtractPersistentBackground(eventData, global);

    // Streak finder
    streakFinder(eventData, global);

    // Radial background subtraction (!!! Radial background subtraction subtracts a photon background, therefore moved here)
    subtractRadialBackground(eventData, global);

    // Hitfinder fast-scan
    // Looks at the inner part of the detector first to see whether it's worth looking at the rest
    // Useful for local background subtraction (which is effective but slow)
    if (global->hitfinder && global->hitfinderFastScan) {
        if (global->hitfinderAlgorithm == 3 || global->hitfinderAlgorithm == 6 || global->hitfinderAlgorithm == 8 || global->hitfinderAlgorithm == 14) {

            if (global->hitfinderAlgorithm == 14) {
                printf("\n\n\n\n\n\nERROR!!!!! Fast scan not implemented with peakFinder9 yet!!!!!!!!!!\n\n\n\n\n\n\n\n");
            }

            hit = hitfinderFastScan(eventData, global);
            eventData->hit = hit;

            if (!hit)
                goto hitknown;
        }
    }

    // Local background subtraction - this is photon background correction
    if (!global->hitfinderFastScan) {
        subtractLocalBackground(eventData, global);
    }

    //----------------------------------------//
    //---HITFINDING AND POWDERCLASS SORTING---//
    //----------------------------------------//
    int powderClass;
    if (global->hitfinder && (global->hitfinderForInitials ||
            !(eventData->threadNum < global->nInitFrames || !calibrated))) {

        DEBUG2("Hit finding");

        hit = hitfinder(eventData, global);
        eventData->hit = hit;

        pthread_mutex_lock(&global->hitclass_mutex);
        for (int coord = 0; coord < 3; coord++) {
            if (eventData->nPeaks < 100)
                continue;
            global->hitClasses[coord][std::make_pair(eventData->samplePos[coord] * 1000, hit)]++;
        }
        pthread_mutex_unlock(&global->hitclass_mutex);
    }

    hitknown:
    //-------------------------------------//
    //---PROCEDURES-DEPENDENT-ON-HIT-TAG---//
    //-------------------------------------//
    DEBUG2("Procedures depending on hit tag");

    // Sort event into different classes (eg: laser on/off)
    // Slightly wrong that all initial frames are blanks when hitfinderForInitials is 0

    sortPowderClass(eventData, global);
    powderClass = eventData->powderClass;

    // Update central hit counter - done in hitfinder.cpp
    //pthread_mutex_lock(&global->nhits_mutex);
    //global->nhitsandblanks++;
    //if(hit) {
    //	global->nhits++;
    //	global->nrecenthits++;
    //}
    //pthread_mutex_unlock(&global->nhits_mutex);
    hitRatio = 100. * (global->nhits / (float) global->nhitsandblanks);

    // Update running backround estimate based on non-hits
    updateBackgroundBuffer(eventData, global, hit);

    // Identify noisy pixels
    updateNoisyPixelBuffer(eventData, global, hit);

    // Skip first set of frames to build up running estimate of background...
    if (eventData->threadNum < global->nInitFrames || !calibrated) {
        // Update running backround estimate based on non-hits and calculate background from buffer
        global->updateCalibrated();
        printf("r%04u:%li (%2.1lf Hz, %3.3f %% hits): Digesting initial frame %s (hit=%i, npeaks=%i)\n", global->runNumber, eventData->threadNum, processRate,
                hitRatio, eventData->eventStamp, hit, eventData->nPeaks);
        goto cleanup;
    }

    // Inside-thread speed test
    if (global->ioSpeedTest == 6) {
        printf("r%04u:%li (%3.1fHz): I/O Speed test #6 (after hitfinding)\n", global->runNumber, eventData->frameNumber, global->datarate);
        goto cleanup;
    }

    //----------------------------------//
    //---ASSEMBLE-AND-ACCUMULATE-DATA---//
    //----------------------------------//
    DEBUG2("Assemble and accumulate data");

    // Inside-thread speed test
    if (global->ioSpeedTest == 7) {
        printf("r%04u:%li (%3.1fHz): I/O Speed test #7 (after powder sum and reverting images)\n", global->runNumber, eventData->frameNumber, global->datarate);
        goto cleanup;
    }

    // Assemble, downsample and radially average current frame
    assemble2D(eventData, global);
    //downsample(eventData, global);

    // Powder
    // Maintain a running sum of data (powder patterns)
    addToPowder(eventData, global);

    // Calculate radial averages
    calculateRadialAverage(eventData, global);
    addToRadialAverageStack(eventData, global);

    // Calculate the one dimesional beam spectrum
    integrateSpectrum(eventData, global);
    integrateRunSpectrum(eventData, global);

    // Update GMD average
    updateAvgGmd(eventData, global);

    // Integrate pattern
    integratePattern(eventData, global);

    // Histogram of detector values
    //addToHistogram(eventData, global, hit);

    // Inside-thread speed test
    if (global->ioSpeedTest == 8) {
        printf("r%04u:%li (%3.1fHz): I/O Speed test #8 (radial average and spectrum)\n", global->runNumber, eventData->frameNumber, global->datarate);
        goto cleanup;
    }

    // Inside-thread speed test
    if (global->ioSpeedTest == 9) {
        printf("r%04u:%li (%3.1fHz): I/O Speed test #9 (After histograms)\n", global->runNumber, eventData->frameNumber, global->datarate);
        goto cleanup;
    }

    //----------------------//
    //---WRITE-DATA-TO-H5---//
    //----------------------//
    DEBUG2("Write data to h5");

    updateDatarate(global);

    if (global->saveCXI == 1) {
        //writeCXIHitstats(eventData, global);
    }

    // If this is a hit, write out to our favourite HDF5 format
    // Put here anything only needed for data saved to file (why waste the time on events that are not saved)
    // eg: only assemble 2D images, 2D masks and downsample if we are actually saving this frame

    eventData->writeFlag =
            (hit && global->saveHits && (eventData->nPeaks >= global->saveHitsMinNPeaks)) ||
                    (!hit && global->saveBlanks) ||
                    ((global->hdf5dump > 0) && ((eventData->frameNumber % global->hdf5dump) == 0));

    // Synchronisation of all writing so that stacks, CXI file, etc stay in step with each other
    pthread_mutex_lock(&global->saveSynchronisation_mutex);

    if (global->generateDarkcal || global->generateGaincal) {
        // Print frames for dark/gain
        printf("r%04u:%li (%2.1lf Hz): Processed %s\n", global->runNumber, eventData->threadNum, processRate, eventData->eventStamp);
    }
    else {
        if (eventData->writeFlag) {
            DEBUG2("About to write frame.");
            // one CXI or many H5?
            if (global->saveCXI) {
                printf("r%04u:%li (%2.1lf Hz, %3.3f %% hits): Writing %s (hit=%i,npeaks=%i)\n", global->runNumber, eventData->threadNum, processRate, hitRatio,
                        eventData->eventStamp, hit, eventData->nPeaks);
                writeCXI(eventData, global);
                writeCXIHitstats(eventData, global);
                addTimeToolToStack(eventData, global, powderClass);
                addFEEspectrumToStack(eventData, global, powderClass);
            }
            else {
                printf("r%04u:%li (%2.1lf Hz, %3.3f %% hits): Writing to %s.h5 (hit=%i,npeaks=%i)\n", global->runNumber, eventData->threadNum, processRate,
                        hitRatio, eventData->eventStamp, hit, eventData->nPeaks);
                writeHDF5(eventData, global);
                addTimeToolToStack(eventData, global, powderClass);
                addFEEspectrumToStack(eventData, global, powderClass);
            }
            DEBUG2("Frame written.");
        }
        // This frame is not going to be saved, but print anyway
        else {
            printf("r%04u:%li (%2.1lf Hz, %3.3f %% hits): Processed %s (hit=%i,npeaks=%i)\n", global->runNumber, eventData->threadNum, processRate, hitRatio,
                    eventData->eventStamp, hit, eventData->nPeaks);
        }
    }

    // If this is a hit, write out peak info to peak list file	
    if (hit && global->savePeakInfo) {
        writePeakFile(eventData, global);
    }

    DEBUG2("Logbook keeping");
    writeLog(eventData, global);

    // Release synchronisation lock 
    pthread_mutex_unlock(&global->saveSynchronisation_mutex);

    // Inside-thread speed test
    if (global->ioSpeedTest == 10) {
        printf("r%04u:%li (%3.1fHz): I/O Speed test #1 (after saving frames)\n", global->runNumber, eventData->frameNumber, global->datarate);
        goto cleanup;
    }

    //-----------------------//
    //---CLEANUP-AND-EXIT----//
    //-----------------------//
    cleanup:
    DEBUG2("Clean up and exit");

    // Save accumulated data periodically
    // Update counters
    pthread_mutex_lock(&global->saveinterval_mutex);
    global->nprocessedframes += 1;
    global->nrecentprocessedframes += 1;

    // Save some types of information from time to time (for example, powder patterns get updated while running)
    if (global->saveInterval != 0 && (global->nprocessedframes % global->saveInterval) == 0
            && (global->nprocessedframes > global->detector[0].startFrames + 50)) {
        DEBUG3("Save data.");

        // Assemble, downsample and radially average powder
        assemble2DPowder(global);
        downsamplePowder(global);
        calculateRadialAveragePowder(global);
        saveRadialStacks(global);

        // Flush CXI files (makes them readable if program crashes)
        if (global->saveCXI) {
            writeAccumulatedCXI(global);
            flushCXIFiles(global);
        }

        // Write running sums
        if (global->writeRunningSumsFiles) {
            saveRunningSums(global);
            saveHistograms(global);
            saveSpectrumStacks(global);
            saveTimeToolStacks(global);
        }

        global->updateLogfile();
        global->writeStatus("Not finished");
    }
    pthread_mutex_unlock(&global->saveinterval_mutex);

    // Decrement thread pool counter by one
    pthread_mutex_lock(&global->nActiveThreads_mutex);
    global->nActiveCheetahThreads -= 1;
    pthread_mutex_unlock(&global->nActiveThreads_mutex);
    sem_post(&global->availableCheetahThreads);

    global->processRateMonitor.frameFinished();

    // Free memory only if running multi-threaded
    if (eventData->useThreads == 1) {
        cheetahDestroyEvent(eventData);
        pthread_exit (NULL);
    }
    else {
        return (NULL);
    }
}

/*
 * Nasty little bit of code that aims to toggle the evr41 signal based on the Acqiris
 * signal.  Very simple: scan along the Acqiris trace (starting from the ini keyword 
 * hitfinderTOFMinSample and ending at hitfinderTOFMaxSample) and check that there is 
 * at least one sample above the threshold set by the hitfinderTOFThresh keyword. Oh,
 * and don't forget to specify the Acqiris channel with:
 * tofChannel=0
 * And you'll need to do this as well:
 * fudgeevr41=1
 * tofName=CxiSc1
 *
 * Agreed - doesn't sound very robust.  As far as I can tell at the moment, only a single
 * sample rises above threshold when the laser trigger is on... maybe bad sampling interval?
 * Or, I could be wrong...
 * 
 * -Rick  
 */
void evr41fudge(cEventData *t, cGlobal *g)
{

    if (g->TOFPresent == 0) {
        //printf("Acqiris not present; can't fudge EVR41...\n");
        return;
    }

    //int nCh = g->AcqNumChannels;
    //int nSamp = g->AcqNumSamples;
    double * Vtof = &(t->tofDetector[0].voltage[0]);
    int i;
    double Vtot = 0;
    double Vmax = 0;
    int tCounts = 0;
    for (i = g->tofDetector[0].hitfinderMinSample; i < g->tofDetector[0].hitfinderMaxSample; i++) {
        Vtot += Vtof[i];
        if (Vtof[i] > Vmax)
            Vmax = Vtof[i];
        if (Vtof[i] >= g->tofDetector[0].hitfinderThreshold)
            tCounts++;
    }

    bool acqLaserOn = false;
    if (tCounts >= 1) {
        acqLaserOn = true;
    }
    //if ( acqLaserOn ) printf("acqLaserOn = true\n"); else printf("acqLaserOn = false\n");
    //if ( t->pumpLaserCode ) printf("pumpLaserCode = true\n"); else printf("pumpLaserCode = false\n");
    if (acqLaserOn != t->pumpLaserCode) {
        if (acqLaserOn) {
            printf("MESSAGE: Acqiris and evr41 disagree.  We trust acqiris (set evr41 = 1 )\n");
        } else {
            printf("MESSAGE: Acqiris and evr41 disagree.  We trust acqiris (set evr41 = 0 )\n");
        }
        t->pumpLaserCode = acqLaserOn;
    }
}

double difftime_timeval(timeval t1, timeval t2)
{
    return ((t1.tv_sec - t2.tv_sec) + (t1.tv_usec - t2.tv_usec) / 1000000.0);
}

void updateDatarate(cGlobal *global)
{

    timeval timevalNow;
    double mem = global->datarateWorkerMemory;
    double dtNew, dtNow;
    gettimeofday(&timevalNow, NULL);

    pthread_mutex_lock(&global->datarateWorker_mutex);
    if (timercmp(&timevalNow,&global->datarateWorkerTimevalLast,!=)) {
        dtNow = difftime_timeval(timevalNow, global->datarateWorkerTimevalLast) / (1 + global->datarateWorkerSkipCounter);
        dtNew = 1 / global->datarateWorker * mem + dtNow * (1 - mem);
        global->datarateWorker = 1 / dtNew;
        global->datarateWorkerTimevalLast = timevalNow;
        global->datarateWorkerSkipCounter = 0;
    } else {
        global->datarateWorkerSkipCounter += 1;
    }
    pthread_mutex_unlock(&global->datarateWorker_mutex);

}

