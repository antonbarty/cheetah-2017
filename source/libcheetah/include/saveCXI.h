#pragma once

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>
#include <vector>


#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "cheetahmodules.h"
#include "median.h"


namespace CXI{
	const char* ATTR_NAME_NUM_EVENTS = "numEvents";

	class Node{
	public:
		Node(const char * filename, bool swmr){
			parent = NULL;
			name = std::string("/");
			hid_t fapl_id = H5Pcreate(H5P_FILE_ACCESS);
			if(fapl_id < 0 || H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG) < 0){
				ERROR("Cannot set file access properties.\n");
			}
			if(swmr){
#ifdef H5F_ACC_SWMR_WRITE
				if(H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0){
					ERROR("Cannot set file access properties.\n");
				}
#else
				ERROR("Cannot write in SWMR mode, HDF5 library does not support it.\n");
#endif
			}

			id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id);
			if( id<0 ) {ERROR("Cannot create file.\n");}
			stackSlice = 0;
		}
		
		Node(std::string s, hid_t oid, Node * p){
			name = s;
			parent = p;
			id = oid;
		}
		Node *  operator [](char * s){
			if(children.find(s) != children.end()){
				return children.find(s)->second;
			}
			return 0;
		}
		hid_t hid(){
			return id;
		}
		/*
		  The base name of the group should be used.
		  For example "entry" if you want to create "entry_N"
		*/
		Node * addClass(const char * s);
		Node * createGroup(const char * s);
		/* 
		   To create a stack pass length = H5S_UNLIMITED
		   To create a string dataset pass dataType = H5T_NATIVE_CHAR and width as maximum string size.
		 */
		Node * createDataset(const char * s, hid_t dataType, hsize_t width = 0, hsize_t height = 0, hsize_t length = 0, int chunkSize = 0);
		Node * createStack(const char * s, hid_t dataType, hsize_t width = 0, hsize_t height = 0, hsize_t length = H5S_UNLIMITED, int chunkSize = 0){
			return createDataset(s,dataType,width,height,length,chunkSize);
		}
		template<class T>
			void write(T * data, int stackSlice = -1);
	private:
		Node * addNode(const char * s, hid_t oid);
		void addStackAttributes(hid_t dataset, int ndims);
		hid_t writeNumEvents(hid_t dataset, int stackSlice);
		std::string nextKey(const char * s);

		std::map<std::string,Node *> children;
		std::string name;
		Node * parent;
		hid_t id;
		int stackSlice;
	};

	typedef struct{
	  hid_t self;
	  hid_t translation;
	}Geometry;

	typedef struct{
	  hid_t self;
	  Geometry geometry;
	}Sample;

	typedef struct{
		hid_t self;
		hid_t distance;
		hid_t data;
		hid_t tofTime;
		hid_t mask;
		hid_t mask_shared;
		hid_t thumbnail;
		hid_t description;
		hid_t xPixelSize;
		hid_t yPixelSize;
	}Detector;


	typedef struct{
		/* ID of the group itself */
		hid_t self;
		hid_t data;
		hid_t mask;
		hid_t mask_shared;
		hid_t mask_shared_min;
		hid_t mask_shared_max;
		hid_t thumbnail;
		hid_t dataType;
		hid_t dataSpace;
	}Image;

	typedef struct{
		hid_t self;

		hid_t energy;
	}Source;

	typedef struct{
		hid_t self;

		Source source;
		std::vector<Detector> detectors;
	}Instrument;

	typedef struct{
		hid_t self;

		hid_t machineTime;
		hid_t fiducial;
		hid_t ebeamCharge;
		hid_t ebeamL3Energy;
		hid_t ebeamPkCurrBC2;
		hid_t ebeamLTUPosX;
		hid_t ebeamLTUPosY;
		hid_t ebeamLTUAngX;
		hid_t ebeamLTUAngY;
		hid_t phaseCavityTime1;
		hid_t phaseCavityTime2;
		hid_t phaseCavityCharge1;
		hid_t phaseCavityCharge2;
		hid_t photon_energy_eV;
		hid_t photon_wavelength_A;
		hid_t f_11_ENRC;
		hid_t f_12_ENRC;
		hid_t f_21_ENRC;
		hid_t f_22_ENRC;
		hid_t evr41;
		std::vector<hid_t> detector_positions;
		std::vector<hid_t> detector_EncoderValues;
		hid_t eventTimeString;
		hid_t tofTime;
		hid_t tofVoltage;
	}LCLS;

	typedef struct{
		hid_t self;
	}ConfValues;

	typedef struct{
		hid_t self;
		hid_t eventName;
		hid_t frameNumber;
		hid_t frameNumberIncludingSkipped;
		hid_t threadID;
		hid_t gmd1;
		hid_t gmd2;
		hid_t energySpectrumExist;
		hid_t nPeaks;
		hid_t peakNpix;
		hid_t peakTotal;
		hid_t peakResolution;
		hid_t peakResolutionA;
		hid_t peakDensity;
		hid_t laserEventCodeOn;
		hid_t laserDelay;
		hid_t hit;
		std::vector<hid_t> sums;
	}UnsharedValues;

	typedef struct{
		hid_t self;
		hid_t lastBgUpdate[MAX_DETECTORS];
		hid_t nHot[MAX_DETECTORS];
		hid_t lastHotPixUpdate[MAX_DETECTORS];
		hid_t hotPixCounter[MAX_DETECTORS];
		hid_t nHalo[MAX_DETECTORS];
		hid_t lastHaloPixUpdate[MAX_DETECTORS];
		hid_t haloPixCounter[MAX_DETECTORS];
		hid_t hit;
		hid_t nPeaks;
	}SharedValues;

	typedef struct{
		hid_t self;
		//needs to be implemented:
		//hid_t cheetah_version;
		ConfValues confVal;
		UnsharedValues unsharedVal;
		SharedValues sharedVal;
	}CheetahValues;

	typedef struct{
		std::vector<hid_t> radialAverage;
		std::vector<hid_t> radialAverageCounter;
	}Radial;

	typedef struct{
		hid_t self;
	}Data;

	typedef struct{
		hid_t self;

		Data data;
		Instrument instrument;
		hid_t experimentIdentifier;
		std::vector<Image> images;
		Sample sample;
	}Entry;

	typedef struct{
		hid_t self;
		Entry entry;
		LCLS lcls;
		CheetahValues cheetahVal;
  
		/*  This counter defines where in the file each image is stored.
		 *  It is atomically incremented by each thread */
		uint stackCounter;
	}File;


	const int version = 130;
	const int thumbnailScale = 8;
	// The preferred chunk size for 2D stacks is 16 MBytes
	const int chunkSize2D = 16777216;
	// The preferred chunk size for 1D stacks is 1 MByte
	const int chunkSize1D = 1048576;
}




