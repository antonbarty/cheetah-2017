//============================================================================
// Name        : cheetahExtensions.cpp
// Author      : Yaro
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <string.h>
#include "testFileHandling.h"
#include "mask.h"
#include "cheetahLegacy_peakFinders.h"
#include "peakFinder.h"
#include "streakFinder.h"
#include "pythonWrapperConversions.h"
#include "detectorGeometry.h"
#include "radialBackgroundSubtraction.h"
#include "pnCcdWorkarounds.h"
#include "streakfinder_wrapper.h"
#include <Eigen/StdVector>

#include <stdlib.h>
#include <chrono>

using namespace std;
using namespace Eigen;

void flushCache()
{
    const int size = 20 * 1024 * 1024; // Allocate 20M. Set much larger then L2
    volatile char *c = (char *) malloc(size);
    volatile int i = 8;
    for (volatile int j = 0; j < size; j++)
        c[j] = i * j;

    free((void*) c);
}

int main()
{

    char importImageFilename[] = "P:\\Matlab_workspace\\data\\image";
    char importMaskFilename[] = "P:\\Matlab_workspace\\data\\mask";
    char importDetectorGeometryMatrixCommonFilename[] = "P:\\Matlab_workspace\\data\\detectorGeometryMatrix_float";
    char detectorRawSizeFilename[] = "P:\\Matlab_workspace\\data\\detectorRawSize";

    char exportImageFilename[] = "P:\\Matlab_workspace\\data\\image_processedByCpp";
    char exportMaskFilename[] = "P:\\Matlab_workspace\\data\\mask_processedByCpp";

    float *data, *dataCopy;
    uint8_t *mask;
    detectorRawSize_cheetah_t detectorRawSize_cheetah;
    Vector2f* detectorGeometryMatrix;
    float* detectorGeometryRadiusMatrix;
    vector < vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > > detectorPositions;

    tPeakList peakList;
    long NpeaksMax = 1024;

    importTestImage(importImageFilename, importMaskFilename, importDetectorGeometryMatrixCommonFilename, detectorRawSizeFilename, &data, &mask,
            &detectorGeometryMatrix, &detectorGeometryRadiusMatrix, detectorRawSize_cheetah);
    allocateTestImageCopy(&dataCopy, detectorRawSize_cheetah);
    allocatePeakList(&peakList, NpeaksMax);

    copy(data, data + detectorRawSize_cheetah.pix_nn, dataCopy);

    computeDetectorPositionsFromDetectorGeometryMatrix(detectorPositions, detectorRawSize_cheetah, detectorGeometryMatrix);

    streakFinder_accuracyConstants_t streakFinder_accuracyConstants;
    streakFinder_accuracyConstants.filterLength = 30;
    streakFinder_accuracyConstants.minFilterLength = 20;
    streakFinder_accuracyConstants.filterStep = 1.4;
    streakFinder_accuracyConstants.sigmaFactor = 14;
    streakFinder_accuracyConstants.streakElongationMinStepsCount = 10;
    streakFinder_accuracyConstants.streakElongationRadiusFactor = 0.08;
    streakFinder_accuracyConstants.streakPixelMaskRadius = 2;
    streakFinder_accuracyConstants.linesToCheck.push_back(1);
    streakFinder_accuracyConstants.linesToCheck.push_back(3);
//    streakFinder_accuracyConstants.linesToCheck.push_back(5);
    streakFinder_accuracyConstants.linesToCheck.push_back(9);
//    streakFinder_accuracyConstants.linesToCheck.push_back(14);
    streakFinder_accuracyConstants.linesToCheck.push_back(20);
//    streakFinder_accuracyConstants.linesToCheck.push_back(27);
    streakFinder_accuracyConstants.linesToCheck.push_back(35);
    streakFinder_accuracyConstants.linesToCheck.push_back(45);
    setStreakDetectorIndices(streakFinder_accuracyConstants, detectorCategory_t::detectorCategory_pnCCD);
    streakFinder_accuracyConstants.backgroundEstimationRegionsInDetector.push_back(ImageRectangle < uint16_t > (Point2D < uint16_t > (100, 400), 10, 10));
    streakFinder_accuracyConstants.backgroundEstimationRegionsInDetector.push_back(ImageRectangle < uint16_t > (Point2D < uint16_t > (250, 250), 10, 10));
    streakFinder_accuracyConstants.backgroundEstimationRegionsInDetector.push_back(ImageRectangle < uint16_t > (Point2D < uint16_t > (450, 200), 10, 10));

    uint32_t imageSize_x = detectorRawSize_cheetah.asic_nx * detectorRawSize_cheetah.nasics_x;
    uint32_t imageSize_y = detectorRawSize_cheetah.asic_ny * detectorRawSize_cheetah.nasics_y;

    float *pixel_map_x = new float[imageSize_x * imageSize_y];
    float *pixel_map_y = new float[imageSize_x * imageSize_y];
    for (uint32_t i = 0; i < imageSize_x * imageSize_y; ++i) {
        pixel_map_x[i] = detectorGeometryMatrix[i].x();
        pixel_map_y[i] = detectorGeometryMatrix[i].y();
    }

    streakFinder_constantArguments_t *streakFinder_constantArguments = precomputeStreakFinderConstantArguments(streakFinder_accuracyConstants.filterLength,
            streakFinder_accuracyConstants.minFilterLength, streakFinder_accuracyConstants.filterStep, streakFinder_accuracyConstants.sigmaFactor,
            streakFinder_accuracyConstants.streakElongationMinStepsCount, streakFinder_accuracyConstants.streakElongationRadiusFactor,
            streakFinder_accuracyConstants.streakPixelMaskRadius, 6, detectorCategory_pnCCD, 0, 0, detectorRawSize_cheetah.asic_nx,
            detectorRawSize_cheetah.asic_ny, detectorRawSize_cheetah.nasics_x, detectorRawSize_cheetah.nasics_y, pixel_map_x, pixel_map_y, mask, NULL);

    flushCache();

    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

    streakfinder(dataCopy, mask, mask, streakFinder_constantArguments);

    chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast < chrono::milliseconds > (t2 - t1).count();
    cout << "duration: " << duration << "ms" << endl;

    getMaskFromMergedMaskInData(dataCopy, mask, detectorRawSize_cheetah);
    exportTestImage(exportImageFilename, exportMaskFilename, dataCopy, mask, detectorRawSize_cheetah);

    deleteTestImageCopy(dataCopy);
    deleteTestImage(data, mask, detectorGeometryMatrix, detectorGeometryRadiusMatrix);
    freePrecomputedStreakFinderConstantArguments(streakFinder_constantArguments);

    return 0;
}

//pnccd raw
//int main()
//{
//
//    char importImageFilename[] = "P:\\Matlab_workspace\\data\\image";
//    char importMaskFilename[] = "P:\\Matlab_workspace\\data\\mask";
//    char importDetectorGeometryMatrixCommonFilename[] = "P:\\Matlab_workspace\\data\\detectorGeometryMatrix_float";
//    char detectorRawSizeFilename[] = "P:\\Matlab_workspace\\data\\detectorRawSize";
//
//    char exportImageFilename[] = "P:\\Matlab_workspace\\data\\image_processedByCpp";
//    char exportMaskFilename[] = "P:\\Matlab_workspace\\data\\mask_processedByCpp";
//
//    float *data, *dataCopy, *data_rearranged;
//    uint8_t *mask, *mask_rearranged;
//    detectorRawSize_cheetah_t detectorRawSize_cheetah;
//    Vector2f* detectorGeometryMatrix;
//    float* detectorGeometryRadiusMatrix;
//    vector < vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > > detectorPositions;
//    vector < vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > > detectorPositions_rearranged;
//
//    tPeakList peakList;
//    long NpeaksMax = 1024;
//
//    importTestImage(importImageFilename, importMaskFilename, importDetectorGeometryMatrixCommonFilename, detectorRawSizeFilename, &data, &mask,
//            &detectorGeometryMatrix, &detectorGeometryRadiusMatrix, detectorRawSize_cheetah);
//    allocateTestImageCopy(&dataCopy, detectorRawSize_cheetah);
//    allocatePeakList(&peakList, NpeaksMax);
//
//    copy(data, data + detectorRawSize_cheetah.pix_nn, dataCopy);
//
//    computeDetectorPositionsFromDetectorGeometryMatrix(detectorPositions, detectorRawSize_cheetah, detectorGeometryMatrix);
//    rearrangePnCcdGeometryForStreakFinder(detectorPositions_rearranged, detectorPositions);
//
//    streakFinder_accuracyConstants_t streakFinder_accuracyConstants;
//    streakFinder_accuracyConstants.filterLength = 30;
//    streakFinder_accuracyConstants.minFilterLength = 20;
//    streakFinder_accuracyConstants.filterStep = 1.4;
//    streakFinder_accuracyConstants.sigmaFactor = 14;
//    streakFinder_accuracyConstants.streakElongationMinStepsCount = 10;
//    streakFinder_accuracyConstants.streakElongationRadiusFactor = 0.08;
//    streakFinder_accuracyConstants.streakPixelMaskRadius = 2;
//    streakFinder_accuracyConstants.linesToCheck.push_back(1);
//    streakFinder_accuracyConstants.linesToCheck.push_back(3);
////    streakFinder_accuracyConstants.linesToCheck.push_back(5);
//    streakFinder_accuracyConstants.linesToCheck.push_back(9);
////    streakFinder_accuracyConstants.linesToCheck.push_back(14);
//    streakFinder_accuracyConstants.linesToCheck.push_back(20);
////    streakFinder_accuracyConstants.linesToCheck.push_back(27);
//    streakFinder_accuracyConstants.linesToCheck.push_back(35);
//    streakFinder_accuracyConstants.linesToCheck.push_back(45);
//    setStreakDetectorIndices(streakFinder_accuracyConstants, detectorCategory_t::detectorCategory_pnCCD);
//    streakFinder_accuracyConstants.backgroundEstimationRegionsInDetector.push_back(ImageRectangle < uint16_t > (Point2D < uint16_t > (100, 400), 10, 10));
//    streakFinder_accuracyConstants.backgroundEstimationRegionsInDetector.push_back(ImageRectangle < uint16_t > (Point2D < uint16_t > (250, 250), 10, 10));
//    streakFinder_accuracyConstants.backgroundEstimationRegionsInDetector.push_back(ImageRectangle < uint16_t > (Point2D < uint16_t > (450, 200), 10, 10));
//
//    mergeMaskIntoData(data, mask, detectorRawSize_cheetah);
//
//    mask_rearranged = new uint8_t[detectorRawSize_cheetah.pix_nn];
//    rearrangePnCcdMaskForStreakFinder(mask_rearranged, mask);
//    streakFinder_precomputedConstants_t streakFinder_precomputedConstants;
//    precomputeStreakFinderConstants(streakFinder_accuracyConstants, detectorRawSize_cheetah, detectorPositions_rearranged, mask_rearranged,
//            streakFinder_precomputedConstants);
//    flushCache();
//
//    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
//
//    data_rearranged = new float[detectorRawSize_cheetah.pix_nn];
//    rearrangePnCcdDataForStreakFinder(data_rearranged, dataCopy);
//    streakFinder(data_rearranged, streakFinder_accuracyConstants, detectorRawSize_cheetah, detectorPositions_rearranged, streakFinder_precomputedConstants);
//    reRearrangePnCcdDataForStreakFinder(dataCopy, data_rearranged);
//
//    chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
//
//    auto duration = chrono::duration_cast < chrono::milliseconds > (t2 - t1).count();
//    cout << "duration: " << duration << "ms" << endl;
//
//    getMaskFromMergedMaskInData(dataCopy, mask, detectorRawSize_cheetah);
//    exportTestImage(exportImageFilename, exportMaskFilename, dataCopy, mask, detectorRawSize_cheetah);
//
//    deleteTestImageCopy(dataCopy);
//    deleteTestImage(data, mask, detectorGeometryMatrix, detectorGeometryRadiusMatrix);
//    freePrecomputedStreakFinderConstants(streakFinder_precomputedConstants);
//    delete[] mask_rearranged;
//    delete[] data_rearranged;
//
//    return 0;
//}

//int main()
//{
//
//    char importImageFilename[] = "C:\\Users\\Yaro\\Documents\\MATLAB\\Desy\\data\\image";
//    char importMaskFilename[] = "C:\\Users\\Yaro\\Documents\\MATLAB\\Desy\\data\\mask";
//    char importDetectorGeometryMatrixCommonFilename[] = "C:\\Users\\Yaro\\Documents\\MATLAB\\Desy\\data\\detectorGeometryMatrix_float";
//
//    char exportImageFilename[] = "C:\\Users\\Yaro\\Documents\\MATLAB\\Desy\\data\\image_processedByCpp";
//    char exportMaskFilename[] = "C:\\Users\\Yaro\\Documents\\MATLAB\\Desy\\data\\mask_processedByCpp";
//
//    float *data, *dataCopy;
//    uint8_t* mask;
//    detectorRawSize_cheetah_t detectorRawSize_cheetah;
//    Vector2f* detectorGeometryMatrix;
//    float* detectorGeometryRadiusMatrix;
//    vector < vector< detectorPosition_t, Eigen::aligned_allocator< detectorPosition_t > > > detectorPositions;
//
//    tPeakList peakList;
//    long NpeaksMax = 1024;
//
//    vector < uint32_t > sparseMask;
//
//    importTestImage(importImageFilename, importMaskFilename, importDetectorGeometryMatrixCommonFilename, detectorRawSizeFilename, &data, &mask,
//            &detectorGeometryMatrix, &detectorGeometryRadiusMatrix, detectorRawSize_cheetah);
//    allocateTestImageCopy(&dataCopy, detectorRawSize_cheetah);
//    allocatePeakList(&peakList, NpeaksMax);
//
//    computeDetectorPositionsFromDetectorGeometryMatrix(detectorPositions, detectorRawSize_cheetah, detectorGeometryMatrix);
//
//    createSparseMask(mask, detectorRawSize_cheetah, sparseMask);
//
//    radialRankFilter_accuracyConstants_t radialRankFilter_accuracyConstants;
//    radialRankFilter_accuracyConstants.minValuesPerBin = 50;
//    radialRankFilter_accuracyConstants.minBinWidth = 3;
//    radialRankFilter_accuracyConstants.maxConsideredValuesPerBin = 500;
//    radialRankFilter_accuracyConstants.rank = 0.5;
//    for (int i = 0; i < 8; ++i) {
//        for (int j = 0; j < 8; ++j) {
//            radialRankFilter_accuracyConstants.detektorsToConsiderIndices.push_back(Point2D < uint_fast8_t > (i, j));
//            radialRankFilter_accuracyConstants.detektorsToCorrectIndices.push_back(Point2D < uint_fast8_t > (i, j));
//        }
//    }
////    radialRankFilter_accuracyConstants.detektorsToConsiderIndices.push_back(Point2D < uint_fast8_t > (0, 1));
////    radialRankFilter_accuracyConstants.detektorsToCorrectIndices.push_back(Point2D < uint_fast8_t > (0, 1));
////    radialRankFilter_accuracyConstants.detektorsToConsiderIndices.push_back(Point2D < uint_fast8_t > (2, 1));
////    radialRankFilter_accuracyConstants.detektorsToCorrectIndices.push_back(Point2D < uint_fast8_t > (2, 1));
//
//    streakFinder_accuracyConstants_t streakFinder_accuracyConstants;
//    streakFinder_accuracyConstants.filterLength = 9;
//    streakFinder_accuracyConstants.minFilterLength = 5;
//    streakFinder_accuracyConstants.filterStep = 1.3;
//    streakFinder_accuracyConstants.sigmaFactor = 9;
//    streakFinder_accuracyConstants.streakElongationMinStepsCount = 4;
//    streakFinder_accuracyConstants.streakElongationRadiusFactor = 0.01;
//    streakFinder_accuracyConstants.streakPixelMaskRadius = 2;
//    streakFinder_accuracyConstants.linesToCheck.push_back(1);
//    streakFinder_accuracyConstants.linesToCheck.push_back(2);
//    streakFinder_accuracyConstants.linesToCheck.push_back(3);
//    setStreakDetectorIndices(streakFinder_accuracyConstants, detectorCategory_t::detectorCategory_CSPAD);
//    setUserSelection_backgroundEstimationRegionInDetector(streakFinder_accuracyConstants, detectorRawSize_cheetah, 1, 10);
//
//    peakFinder9_accuracyConstants_t peakFinder9_accuracyConstants;
//    peakFinder9_accuracyConstants.sigmaFactorBiggestPixel = 7;
//    peakFinder9_accuracyConstants.sigmaFactorPeakPixel = 6;
//    peakFinder9_accuracyConstants.sigmaFactorWholePeak = 9;
//    peakFinder9_accuracyConstants.minimumSigma = 11;
//    peakFinder9_accuracyConstants.minimumPeakOversizeOverNeighbours = 50;
//    peakFinder9_accuracyConstants.windowRadius = 3;
//
//    radialRankFilter_precomputedConstants_t radialRankFilter_precomputedConstants;
//    precomputeRadialRankFilterConstants(radialRankFilter_precomputedConstants, mask, detectorGeometryRadiusMatrix, detectorPositions,
//            detectorRawSize_cheetah, radialRankFilter_accuracyConstants, detectorGeometryMatrix);
//
//    mergeMaskAndDataIntoDataCopy(data, dataCopy, sparseMask, detectorRawSize_cheetah);
//    flushCache();
//    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
//    applyRadialRankFilter(dataCopy, radialRankFilter_accuracyConstants, radialRankFilter_precomputedConstants, detectorRawSize_cheetah, detectorPositions);
//    chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
//
//    streakFinder_precomputedConstants_t streakFinder_precomputedConstants;
////    precomputeStreakFinderConstants(streakFinder_accuracyConstants, detectorRawSize_cheetah, detectorPositions, mask, streakFinder_precomputedConstants);
////
////    flushCache();
////
////    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
////
////    mergeMaskAndDataIntoDataCopy(data, dataCopy, sparseMask, detectorRawSize_cheetah);
////    streakFinder(dataCopy, streakFinder_accuracyConstants, detectorRawSize_cheetah, detectorPositions, streakFinder_precomputedConstants);
////    peakFinder9(dataCopy, peakFinder9_accuracyConstants, detectorRawSize_cheetah, peakList);
////
////    chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
////
//    auto duration = chrono::duration_cast < chrono::milliseconds > (t2 - t1).count();
//    cout << "duration: " << duration << "ms" << endl;
////    cout << "peak count: " << peakList.nPeaks << endl << endl;
//
//    getMaskFromMergedMaskInData(dataCopy, mask, detectorRawSize_cheetah);
//    exportTestImage(exportImageFilename, exportMaskFilename, dataCopy, mask, detectorRawSize_cheetah);
//
//    freePeakList(peakList);
//    deleteTestImageCopy(dataCopy);
//    deleteTestImage(data, mask, detectorGeometryMatrix, detectorGeometryRadiusMatrix);
////    freePrecomputedStreakFinderConstants(streakFinder_precomputedConstants);
//
//    return 0;
//}

