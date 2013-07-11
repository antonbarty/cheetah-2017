/*
 *  angularCorrelation.cpp
 *  cheetah
 *
 *  Created by Jonas Sellberg on 7/10/13.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>
#include <sstream>
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "cheetahmodules.h"

#include "crosscorrelator.h"
#include "arrayclasses.h"
#include "arraydataIO.h"


//-------------------------------------------------------------------
// apply angular cross correlation
//-------------------------------------------------------------------
void calculateAngularCorrelation(cEventData *eventData, cGlobal *global) {
    
    //prepare some things for output
    arraydataIO *io = new arraydataIO;
    std::ostringstream osst;
    osst << eventData->eventname;
    string eventname_str = osst.str();
    
    //create cross correlator object that takes care of the computations
    //the arguments that are passed to the constructor determine 2D/3D calculations with/without mask
    CrossCorrelator *cc = NULL;
    
    DETECTOR_LOOP {
		if (global->detector[detID].useAngularCorrelation) {

            cout << "calculateAngularCorrelation(): made arraydataIO and CrossCorrelator for detector" << detID << endl;
            
        }
    }
    
    /*
    if (global->autoCorrelateOnly) {
        if (global->correlationQScale == 1) {
            cc = new CrossCorrelator( //auto-correlation 2D case, no mask, r pixel map [pixels]
                                     threadInfo->corrected_data, global->pix_x, global->pix_y, RAW_DATA_LENGTH, 
                                     global->correlationNumPhi, global->correlationNumQ );
            
        } else {
            cc = new CrossCorrelator( //auto-correlation 2D case, no mask, q pixel map [Å-1]
                                     threadInfo->corrected_data, threadInfo->pix_qx, threadInfo->pix_qy, RAW_DATA_LENGTH, 
                                     global->correlationNumPhi, global->correlationNumQ );
        }
    } else {
        if (global->correlationQScale == 1) {
            cc = new CrossCorrelator( //full cross-correlation 3D case, no mask, r pixel map [pixels]
                                     threadInfo->corrected_data, global->pix_x, global->pix_y, RAW_DATA_LENGTH, 
                                     global->correlationNumQ, global->correlationNumQ, global->correlationNumPhi );	
        } else {
            cc = new CrossCorrelator( //full cross-correlation 3D case, no mask, q pixel map [Å-1]
                                     threadInfo->corrected_data, threadInfo->pix_qx, threadInfo->pix_qy, RAW_DATA_LENGTH, 
                                     global->correlationNumQ, global->correlationNumQ, global->correlationNumPhi );	
        }
    }
    
    //set bad pixel mask, if necessary
    if (global->useBadPixelMask){
        cc->setMask( global->badpixelmask, RAW_DATA_LENGTH );
    }
    
    //normalize by variance, if necessary
    if (global->correlationNormalization == 2){
        cc->setVarianceEnable(true);
    }
    
    //turn on debug level inside the CrossCorrelator, if needed
    DEBUGL1_ONLY cc->setDebug(1); 
    DEBUGL2_ONLY cc->setDebug(2);
    
    //--------------------------------------------------------------------------------------------alg1
    if (global->useCorrelation == 1) {							
        DEBUGL1_ONLY cout << "XCCA regular (algorithm 1)" << endl;
        
        cc->calculatePolarCoordinates(global->correlationStartQ, global->correlationStopQ);
        cc->calculateXCCA();
        
		//--------------------------------------------------------------------------------------------alg2
    } else if (global->useCorrelation == 2) {					
        DEBUGL1_ONLY cout << "XCCA fast (algorithm 2)" << endl;
        cc->setLookupTable( global->correlationLUT, global->correlationLUTdim1, global->correlationLUTdim2 );
        cc->calculatePolarCoordinates_FAST(global->correlationStartQ, global->correlationStopQ);
        
        // need to protect the FFTW at the core with a mutex, not thread-safe!!
        pthread_mutex_lock(&global->correlationFFT_mutex);
        cc->calculateXCCA_FAST();
        pthread_mutex_unlock(&global->correlationFFT_mutex);
        
        io->writeToTiff( eventname_str+"-polar.tif", cc->polar(), 1 );		// 0: unscaled, 1: scaled
        
    } else {
        cerr << "ERROR in correlate: correlation algorithm " << global->useCorrelation << " not known." << endl;
    }
    
    
    //--------------------------------------------------------------------------------------------save to sum
    //add to threadInfo->correlation
    //pthread_mutex_lock(&global->correlation_mutex);
    if (global->useCorrelation && global->sumCorrelation) {
        
        if (threadInfo->correlation) {
            free(threadInfo->correlation);
        }
        threadInfo->correlation = (double*) calloc(global->correlation_nn, sizeof(double));
        
        if (global->autoCorrelateOnly) {
            // autocorrelation only (q1=q2)
            for (int i=0; i < cc->nQ(); i++) {
                for (int k=0; k < cc->nLag(); k++) {
                    threadInfo->correlation[i*cc->nLag() + k] = cc->autoCorr()->get(i,k);
                }
            }				
        } else {
            // cross-correlation
            for (int i=0; i < cc->nQ(); i++) {
                for (int j=0; j < cc->nQ(); j++) {
                    for (int k=0; k < cc->nLag(); k++) {
                        threadInfo->correlation[i*cc->nQ()*cc->nLag() + j*cc->nLag() + k] = cc->crossCorr()->get(i,j,k);
                    }
                }
            }
        }
        
    }
    
    
    //--------------------------------------------------------------------------------------------output for this shot
    // check if hits should be saved to disk
    if (global->useCorrelation && (global->hdf5dump || global->generateDarkcal
                                   || (hit->standard && global->hitfinder.savehits) 
                                   || (hit->water && global->waterfinder.savehits) 
                                   || (hit->ice && global->icefinder.savehits) 
                                   || (!hit->background && global->backgroundfinder.savehits) )) {
        //HDF5 output
        if (global->correlationOutput % 2){
            if (global->autoCorrelateOnly){
                io->writeToHDF5( osst.str()+"-xaca.h5", cc->autoCorr(), 0 );				// 0: supposed to be double, but it seems to currently save in float (32-bit) precision
            }else{
                cerr << "WARNING in correlate: no HDF5 output for 3D cross-correlation case implemented, yet!" << endl;
            }
        }
        //bin output
        if (global->correlationOutput % 4 > 1){
            //writeSAXS(threadInfo, global, cc, threadInfo->eventname);			// writes SAXS to binary
            writeXCCA(threadInfo, global, cc, threadInfo->eventname); 			// writes XCCA+SAXS to binary
        }
        //TIFF image output
        if (global->correlationOutput > 3){
            if (global->autoCorrelateOnly){
                io->writeToTiff( eventname_str+"-xaca.tif", cc->autoCorr(), 1 );			// 0: unscaled, 1: scaled
            }else{
                cerr << "WARNING in correlate: no tiff output for 3D cross-correlation case implemented, yet!" << endl;
                //one possibility would be to write a stack of tiffs, one for each of the outer q values
            }
        }
    }
    //pthread_mutex_unlock(&global->correlation_mutex);
    */
    delete io;
    delete cc;
}



/*

//----------------------------------------------------------------------------writeSAXS
// write SAXS intensity to binary
//-------------------------------------------------------------------------------------
void writeSAXS(tThreadInfo *info, cGlobal *global, CrossCorrelator *cc, char *eventname) {	
    
    DEBUGL1_ONLY cout << "writing SAXS to file..." << std::flush;
    FILE *filePointerWrite;
    char outfile[1024];
    double nQD = (double) cc->nQ(); // save everything as doubles
    double *buffer;
    buffer = (double*) calloc(cc->nQ(), sizeof(double));
    
    sprintf(outfile,"%s-saxs.bin",eventname);
    DEBUGL1_ONLY printf("r%04u:%i (%2.1f Hz): Writing data to: %s\n", (int)global->runNumber, (int)info->threadNum, global->datarate, outfile);
    
    filePointerWrite = fopen(outfile,"w+");
    
    // angular averages
    for (int i=0; i<cc->nQ(); i++) {
        buffer[i] = cc->iAvg()->get(i);
    }
    fwrite(&nQD,sizeof(double),1,filePointerWrite); // saving dimensions of array before the actual data
    fwrite(&buffer[0],sizeof(double),cc->nQ(),filePointerWrite);
    
    // q binning
    for (int i=0; i<cc->nQ(); i++) {
        buffer[i] = cc->qAvg()->get(i);
    }
    fwrite(&nQD,sizeof(double),1,filePointerWrite);
    fwrite(&buffer[0],sizeof(double),cc->nQ(),filePointerWrite);
    
    fclose(filePointerWrite);
    free(buffer);
    
    DEBUGL1_ONLY cout << "writeSAXS done" << endl;
}



//----------------------------------------------------------------------------writeXCCA
// write cross-correlation to binary
//-------------------------------------------------------------------------------------
void writeXCCA(tThreadInfo *info, cGlobal *global, CrossCorrelator *cc, char *eventname) {
    
    FILE *filePointerWrite;
    char outfile[1024];
    
    double nQD = (double) cc->nQ(); // save everything as doubles
    double nLagD = (double) cc->nLag();
    double nPhiD = (double) cc->nPhi();
    
    const int nQ = cc->nQ();
    const int nPhi = cc->nPhi();
    const int nLag = cc->nLag();
    
    double *buffer;
    buffer = (double*) calloc(nQ*nQ*nLag, sizeof(double));
    
    if (global->autoCorrelateOnly){
        sprintf(outfile,"%s-xaca.bin",eventname);
    } else {
        sprintf(outfile,"%s-xcca.bin",eventname);
    }
    DEBUGL1_ONLY printf("r%04u:%i (%2.1f Hz): Writing data to: %s\n", (int)global->runNumber, (int)info->threadNum, global->datarate, outfile);
    
    filePointerWrite = fopen(outfile,"w+");
    
    // angular averages
    for (int i=0; i<nQ; i++) {
        buffer[i] = cc->iAvg()->get(i);
    }
    fwrite(&nQD,sizeof(double),1,filePointerWrite); // saving dimensions of array before the actual data
    fwrite(&buffer[0],sizeof(double),nQ,filePointerWrite);
    
    // q binning
    for (int i=0; i<nQ; i++) {
        buffer[i] = cc->qAvg()->get(i);
    }
    fwrite(&nQD,sizeof(double),1,filePointerWrite);
    fwrite(&buffer[0],sizeof(double),nQ,filePointerWrite);
    
    // angle binning
    for (int i=0; i<nPhi; i++) {
        buffer[i] = cc->phiAvg()->get(i);
    }
    fwrite(&nPhiD,sizeof(double),1,filePointerWrite);
    fwrite(&buffer[0],sizeof(double),nPhi,filePointerWrite);
    
    // cross-correlation
    if (global->useCorrelation) {
        if (global->autoCorrelateOnly) {
            // autocorrelation only (q1=q2)
            if (!global->sumCorrelation) {
                for (int i=0; i < nQ; i++) {
                    for (int k=0; k < nLag; k++) {
                        buffer[i*nLag + k] = cc->autoCorr()->get(i,k);
                    }
                }
            }
            fwrite(&nQD,sizeof(double),1,filePointerWrite);
            fwrite(&nLagD,sizeof(double),1,filePointerWrite);
            
        } else {
            // full version
            if (!global->sumCorrelation) {
                for (int i=0; i < nQ; i++) {
                    for (int j=0; j < nQ; j++) {
                        for (int k=0; k < nLag; k++) {
                            buffer[i*nQ*nLag + j*nLag + k] = cc->crossCorr()->get(i,j,k);
                        }
                    }
                }
            }
            fwrite(&nQD,sizeof(double),1,filePointerWrite);
            fwrite(&nQD,sizeof(double),1,filePointerWrite);
            fwrite(&nLagD,sizeof(double),1,filePointerWrite);
            
        }
        if (global->sumCorrelation)
            fwrite(&info->correlation[0],sizeof(double),global->correlation_nn,filePointerWrite);
        else fwrite(&buffer[0],sizeof(double),global->correlation_nn,filePointerWrite); 
    }
    
    fclose(filePointerWrite);
    free(buffer);
}

*/

