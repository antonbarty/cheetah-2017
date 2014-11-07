/*
 *  modularDetector.cpp
 *  cheetah
 *
 * Created by Benedikt Daurer on 18/09/14.
 * Copyright 2012 Biophysics & TDB @ Uppsala University. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>
#include <limits.h>

#include "cheetah.h"
#include "cheetahmodules.h"


/* 
 *  Index to the corner of a module
 */
int moduleCornerIndex(int module, int nasics_x, int asic_nx, int asic_ny){
	int row = floor(module/nasics_x);
	int col = module % nasics_x;
	int istart = (row * asic_nx * asic_ny * nasics_x) + (col * asic_nx);
	return istart;
}


template<typename T>
void stackModules(T * data, T * stackedModules, int asic_nx, int asic_ny, int nasics_x, int nasics_y) {
	
	int nasics = nasics_x * nasics_y;
	int asic_nn = asic_nx*asic_ny;

	// Loop through modules
	for (int a=0; a<nasics; a++){

		// module array
		T * module = &stackedModules[a*asic_nn];

		// index in flat data array where the module starts?
		int istart = moduleCornerIndex(a, nasics_x, asic_nx, asic_ny);
		
		// fill the module
		for (int j=0; j<asic_ny; j++){
			for (int i=0; i<asic_nx; i++){
				module[j*asic_nx+i] = data[istart + j*nasics_x*asic_nx  + i];
			}
		}
	}
}

/*
 *  Assemble data (floats) into a stack of detector modules
 */
void stackModulesMask(uint16_t * mask, uint16_t * stackedModules, int asic_nx, int asic_ny, int nasics_x, int nasics_y) {
	stackModules(mask, stackedModules, asic_nx, asic_ny, nasics_x, nasics_y);
}

/*
 *  Assemble mask (uint16) into a stack of detector modules
 */
void stackModulesData(float * data, float * stackedModules, int asic_nx, int asic_ny, int nasics_x, int nasics_y) {
	stackModules(data, stackedModules, asic_nx, asic_ny, nasics_x, nasics_y);
}

/* 
 * A stack of strings with name of the modules (module_identifiers)
 */
void moduleIdentifier(char * mId, int nasics, int stringSize){
	for (int i=0; i<nasics; i++){
		sprintf(&mId[i*stringSize], "%i", i);
	}
}

/*
 * Determine the corner positions of the detector modules
 */
void cornerPositions(float * cornerPos, float * pix_x, float * pix_y, float * pix_z, float pixelSize, int asic_nx, int asic_ny, int nasics_x, int nasics) {
	for (int a=0; a<nasics;a++){
		int cornerIndex = moduleCornerIndex(a, nasics_x, asic_nx, asic_ny);
		cornerPos[a*3]   = pix_x[cornerIndex] * pixelSize;
		cornerPos[a*3+1] = pix_y[cornerIndex] * pixelSize;
		cornerPos[a*3+2] = pix_z[cornerIndex] * pixelSize;
	}
}

/*
 * Determine the basis vectors of the detector modules
 */
void basisVectors(float * basisVec, float * pix_x, float * pix_y, float *pix_z, int asic_nx, int asic_ny, int nasics_x, int nasics) {
	for (int a=0; a<nasics;a++){
		int cornerIndex = moduleCornerIndex(a, nasics_x, asic_nx, asic_ny);
		
		// Basis vector along the first dimenstion
		basisVec[a*3*2]   = (pix_x[cornerIndex] - pix_x[cornerIndex + (asic_ny-1)*asic_nx*nasics_x])/(asic_nx - 1);
		basisVec[a*3*2+1] = (pix_y[cornerIndex] - pix_y[cornerIndex + (asic_ny-1)*asic_nx*nasics_x])/(asic_ny - 1);
		basisVec[a*3*2+2] = (pix_z[cornerIndex] - pix_z[cornerIndex + (asic_ny-1)*asic_nx*nasics_x])/1;

		// Basis vector along the second dimenstion
		basisVec[a*3*2+3] = (pix_x[cornerIndex] - pix_x[cornerIndex + asic_nx - 1])/(asic_nx - 1);
		basisVec[a*3*2+4] = (pix_y[cornerIndex] - pix_y[cornerIndex + asic_nx - 1])/(asic_ny - 1);
		basisVec[a*3*2+5] = (pix_z[cornerIndex] - pix_z[cornerIndex + asic_nx - 1])/1;
	}
}

