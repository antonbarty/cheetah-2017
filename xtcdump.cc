#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits>
#include <string.h>

#include "myana.hh"
#include "main.hh"

    
/*
 *	Static global variables
 */
long 	frameNumber;
FILE	*logfile = NULL;
char 	current_filename[1024];

/*
 * beginjob() is called once at the beginning of the analysis job
 *	Ask for detector "configuration" information here.
 */
void beginjob() 
{ 
	printf("beginjob()\n");
	strcpy(current_filename, "none");
}


/*
 *	beginrun() is called once for each run.  
 *	Check to see whether detector configuration information has changed.
 */
void beginrun() 
{
	printf("beginrun()\n");
  	frameNumber = 0;
  	
  	// Commented out as this causes problems when processing multiple runs from file
	//if ( logfile != NULL ) {
	//	fprintf(stderr, "beginrun() called for a second time.  Uh-oh.\n");
	//	abort();
	//}


	// Current output filename
	char filename[1024];
	sprintf(filename,"r%04u-data.csv",getRunNumber());
	

	// If run has changed, the output filename should be different to the currently open file 
	// Create a new file with current filename (also do this if logfile==NULL!)
	if(strcmp(current_filename, filename) != 0 || logfile == NULL) {
		if ( logfile != NULL ) {
			fclose(logfile);
			logfile = NULL;
		}

		printf("Creating new data file %s\n", filename);
		logfile = fopen(filename,"w");
		if ( logfile == NULL ) {
			fprintf(stderr, "Couldn't open file '%s' for writing.\n",
					filename);
			exit(1);
		}
		strcpy(current_filename, filename);
	}


  	// Generate header line
	printf("Run#, Frame, Timestamp,              Fiducial, BeamOn?,"
	       " PhotonEnergy_eV, Wavelength_A, GMD1_mJ, GMD2_mJ,"
	       "    IPM1_V,     IPM2_V,     IPM3_V,"
	       " Mono_A\n");
	fprintf(logfile, "Run#, Frame, Timestamp,              Fiducial, BeamOn?,"
	                 " PhotonEnergy_eV, Wavelength_A, GMD1_mJ, GMD2_mJ,"
	                 "   IPM1_V,     IPM2_V,     IPM3_V,"
	                 " Mono_A\n");
}
void begincalib()
{
	printf("begincalib()\n");
}


/*
 *	event() is called once every shot.  
 * 	Process individual shot data here.
 */
void event() {

	/*
	 *	Increment frame number
	 */
	frameNumber++;
	// printf("Frame Number %li\n",frameNumber);	
	int fail = 0;

	/*
	 *	Get run number
	 */
	unsigned runNumber;
	runNumber = getRunNumber();


	/*
	 *	Get event information
	 */	 
	int 			numEvrData;
	unsigned int 	eventCode;
	unsigned int 	fiducial;
	unsigned int 	timeStamp;
	
	numEvrData = getEvrDataNumber();
	for (long i=0; i<numEvrData; i++) {
		fail = getEvrData( i, eventCode, fiducial, timeStamp );
	}
	// EventCode==140 = Beam On

	fail = getFiducials(fiducial);


	/*
 	 * Get time information
	 */
	int seconds;
	int nanoSeconds;
	const char* time;
	
	getTime( seconds, nanoSeconds );  
	fail = getLocalTime( time );
	// printf("Time (single shot): %s.%09d\n",time,nanoSeconds);


  	/*
  	 *	Is the beam on?
  	 */
	bool 	beamOn = 0;
	int nfifo = getEvrDataNumber();
	for(int i=0; i<nfifo; i++) {
		unsigned eventCode, fiducial, timestamp;
		if (getEvrData(i,eventCode,fiducial,timestamp)) 
			printf("Failed to fetch evr fifo data\n");
		else if (eventCode==140)
			beamOn = 1;
	}
	//printf("Beam %s\n", beamOn ? "On":"Off");



	/*
	 * Get electron beam parameters from beamline data
	 */     
	double fEbeamCharge;    // in nC
	double fEbeamL3Energy;  // in MeV
	double fEbeamLTUPosX;   // in mm
	double fEbeamLTUPosY;   // in mm
	double fEbeamLTUAngX;   // in mrad
	double fEbeamLTUAngY;   // in mrad
	double fEbeamPkCurrBC2; // in Amps
	double photonEnergyeV;
	double wavelengthA;

	if ( getEBeam(fEbeamCharge, fEbeamL3Energy, fEbeamLTUPosX, fEbeamLTUPosY,
	              fEbeamLTUAngX, fEbeamLTUAngY, fEbeamPkCurrBC2) ) {

		wavelengthA = std::numeric_limits<double>::quiet_NaN();
		photonEnergyeV = std::numeric_limits<double>::quiet_NaN();

	} else {

		/* Calculate the resonant photon energy (ie: photon wavelength) */
		// Get the present peak current in Amps
		double peakCurrent = fEbeamPkCurrBC2;
		// Get present charge in pC
		double charge = 1000*fEbeamCharge;
		// Get present beam energy [GeV]
		double DL2energyGeV = 0.001*fEbeamL3Energy;
		// wakeloss prior to undulators
		double LTUwakeLoss = 0.0016293*peakCurrent;
		// Spontaneous radiation loss per segment
		double SRlossPerSegment = 0.63*DL2energyGeV;
		// wakeloss in an undulator segment
		double wakeLossPerSegment = 0.0003*peakCurrent;
		// energy loss per segment
		double energyLossPerSegment = SRlossPerSegment + wakeLossPerSegment;
		// energy in first active undulator segment [GeV]
		double energyProfile = DL2energyGeV - 0.001*LTUwakeLoss
		         - 0.0005*energyLossPerSegment;
		// Calculate the resonant photon energy of the first active segment
		photonEnergyeV = 44.42*energyProfile*energyProfile;
		// Calculate wavelength in Angstrom
		wavelengthA = 13988./photonEnergyeV;
	}

	/*
	 * 	FEE gas detectors (pulse energy in mJ)
	 */     
	double shotEnergy[4];
	double gmd1;
	double gmd2;
	if ( getFeeGasDet(shotEnergy) ) {
		gmd1 = std::numeric_limits<double>::quiet_NaN();
		gmd2 = std::numeric_limits<double>::quiet_NaN();
	} else {
		gmd1 = (shotEnergy[0]+shotEnergy[1])/2;
		gmd2 = (shotEnergy[2]+shotEnergy[3])/2;
	}


	/*
	 * Phase cavity data
	 *	(we probably won't need this info)
	 */     
	double 	phaseCavityTime1 = nan("");
	double	phaseCavityTime2 = nan("");
	double	phaseCavityCharge1;
	double	phaseCavityCharge2;

	fail = getPhaseCavity(phaseCavityTime1, phaseCavityTime2, phaseCavityCharge1, phaseCavityCharge2);


	/*
	 * Get IPM values
	 */
	float diodes[4];
	float ipm1sum, ipm2sum, ipm3sum, xpos, ypos;
	if ( getIpmFexValue(Pds::DetInfo::XppSb1Ipm, 0, diodes,
	                    ipm1sum, xpos, ypos) ) {
		ipm1sum = std::numeric_limits<double>::quiet_NaN();
	}
	if ( getIpmFexValue(Pds::DetInfo::XppSb2Ipm, 0, diodes,
	                    ipm2sum, xpos, ypos) ) {
		ipm2sum = std::numeric_limits<double>::quiet_NaN();
	}
	if ( getIpmFexValue(Pds::DetInfo::XppSb3Ipm, 0, diodes,
	                    ipm3sum, xpos, ypos) ) {
		ipm3sum = std::numeric_limits<double>::quiet_NaN();
	}

	/*
	 * Get monochromator position
	 */
	char mono[32];
	double alio;
	if ( getControlValue("XPP:MON:MPZ:07A:POSITIONSET", 0, alio) ) {
		snprintf(mono, 31, "n/a");
	} else {
		const double R = 3.175;
		const double D = 231.303;
		const double theta0 = 15.08219;
		const double Si111dspacing = 3.13556044;
		const double theta = theta0 + 180/M_PI*2.0
		           * atan( (sqrt(alio*alio+D*D+2.0*R*alio)-D) / (2.0*R+alio));
		const double monov = 2.0*Si111dspacing*sin(theta/180*M_PI);
		snprintf(mono, 31, "%f", monov);
	}

	/*
	 * 	Retrieving Epics Pv Values
	 *	Just an example - put what XPP PV values we want in here
	 *	But be careful - EPICS is slow data and will not necessarily be correct on a shot-by-shot basis
	 */  
	float value;
	fail = getPvFloat( "AMO:DIA:SHC:11:R", value );


	/*
	 *	Print one line of output per event
	 */
	printf("r%04u, %4li, %s, 0x%5x, %s, %10.6f, %10.2f, %10.6f, %10.6f,"
	       " %+10.6f, %+10.6f, %+10.6f, %s\n",
	      runNumber, frameNumber,  time, fiducial,
	      beamOn?"Beam On ":"Beam Off", photonEnergyeV, wavelengthA,
	      gmd1, gmd2, ipm1sum, ipm2sum, ipm3sum, mono);
	fprintf(logfile, "r%04u, %4li, %s, 0x%5x, %s, %10.2f, %10.6f, %10.6f,"
	                 "%10.6f, %+10.6f, %+10.6f, %+10.6f, %s\n",
	        runNumber, frameNumber, time, fiducial,
	        beamOn?"Beam On ":"Beam Off", photonEnergyeV, wavelengthA,
	        gmd1, gmd2, ipm1sum, ipm2sum, ipm3sum, mono);

  	// printf("%li, %s, %f, %f, %f\n", frameNumber, time, photonEnergyeV, gmd1, gmd2);

  
}  
// End of main per-event loop


/*
 * 	endrun() is called once at the end of each run
 *	Clean up and do final housekeeping
 */
void endrun() 
{
	printf("endrun()\n");
	//fclose(logfile);
	//logfile == NULL;
}

void endcalib() {
	printf("endcalib()\n");
}


/*
 * 	endjob() is called once at the end of the analysis job
 *	Clean up and do final housekeeping
 */
void endjob() {
	printf("endjob()\n");
	if (logfile != NULL) {
		fclose(logfile);
		logfile = NULL;
	}
}
