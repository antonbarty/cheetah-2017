#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "myana.hh"
#include "main.hh"

    
/*
 *	Static global variables
 */
long 	frameNumber;
FILE	*logfile;


/*
 * beginjob() is called once at the beginning of the analysis job
 *	Ask for detector "configuration" information here.
 */
void beginjob() 
{ 
	printf("beginjob()\n");
}


/*
 *	beginrun() is called once for each run.  
 *	Check to see whether detector configuration information has changed.
 */
void beginrun() 
{
	printf("beginrun()\n");
  	frameNumber = 0;
  	
  	
	// Create output file
	char filename[1024];
	sprintf(filename,"r%04u-data.csv",getRunNumber());
	logfile = fopen(filename,"a");
  	
  	// Generate header line
  	printf("FrameNumber, Timestamp, Fiducial_hex, Time, PhotonEnergy_eV, GMD1_mJ, GMD2_mJ\n");
  	fprintf(logfile, "RunNo, FrameNumber, Timestamp, Fiducial_hex, ClockTime, PhotonEnergy_eV, GMD1_mJ, GMD2_mJ\n");
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
	 * Get electron beam parameters from beamline data
	 */     
		double fEbeamCharge;    // in nC
		double fEbeamL3Energy;  // in MeV 
		double fEbeamLTUPosX;   // in mm 
		double fEbeamLTUPosY;   // in mm 
		double fEbeamLTUAngX;   // in mrad 
		double fEbeamLTUAngY;   // in mrad
		double fEbeamPkCurrBC2; // in Amps

		fail = getEBeam(fEbeamCharge, fEbeamL3Energy, fEbeamLTUPosX, fEbeamLTUPosY, fEbeamLTUAngX, fEbeamLTUAngY, fEbeamPkCurrBC2);


	/*
	 * Calculate the resonant photon energy (ie: photon wavelength)
	 */

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
		double energyProfile = DL2energyGeV - 0.001*LTUwakeLoss - 0.0005*energyLossPerSegment;
		
		// Calculate the resonant photon energy of the first active segment
		double photonEnergyeV = 44.42*energyProfile*energyProfile;
		
		// printf("Resonant photon energy (energy corrected, eV): %f\n",photonEnergyeV);


	/*
	 * 	FEE gas detectors (pulse energy in mJ)
	 */     
		double 	shotEnergy[4];
		fail = getFeeGasDet( shotEnergy );

		double	gmd1;
		double 	gmd2;
		gmd1 = (shotEnergy[0]+shotEnergy[1])/2;
		gmd2 = (shotEnergy[2]+shotEnergy[3])/2;

		

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
	 * 	Retrieving Epics Pv Values
	 *	Just an example - put what XPP PV values we want in here
	 *	But be careful - EPICS is slow data and will not necessarily be correct on a shot-by-shot basis
	 */  
		float value;
		fail = getPvFloat( "AMO:DIA:SHC:11:R", value );


	/*
	 *	Print one line of output per event
	 */
  	printf("r%04u, %li, %u, 0x%x, %s, %f, %f, %f\n", runNumber, frameNumber, timeStamp, fiducial, time, photonEnergyeV, gmd1, gmd2);
   	fprintf(logfile, "r%04u, %li, %u, 0x%x, %s, %f, %f, %f\n", runNumber, frameNumber, timeStamp, fiducial, time, photonEnergyeV, gmd1, gmd2);

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
	fclose(logfile);
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
}
