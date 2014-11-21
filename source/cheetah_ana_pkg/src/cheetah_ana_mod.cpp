
//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class cheetah_ana_mod...
//	This is a psana-based module for running Cheetah (Anton's hitfinder)
//
// Author List:
//      Chunhong Yoon	06/2012		chun.hong.yoon@cfel.de
//  	Copyright (c) 2012 CFEL. All rights reserved.
//
//------------------------------------------------------------------------

#include <stdlib.h>
//-----------------------
// This Class's Header --
//-----------------------
#include <cheetah_ana_mod.h>
#include <cheetah.h>
//-----------------
// C/C++ Headers --
//-----------------
#include <iostream>
#include <string>
#include <time.h>
#include <signal.h>
//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
// to work with detector data include corresponding 
// header from psddl_psana package
// Includes can be found in /reg/g/psdm/sw/releases/ana-current.psddl_psana/include/...
#include "PSEvt/EventId.h"
#include "psddl_psana/bld.ddl.h"
#include "psddl_psana/cspad.ddl.h"
#include "psddl_psana/cspad2x2.ddl.h"
#include "psddl_psana/evr.ddl.h"
#include "psddl_psana/acqiris.ddl.h"
#include "psddl_psana/camera.ddl.h"
#include "psddl_psana/pnccd.ddl.h"

// LCLS event codes
#define beamCode 140
#define verbose 0

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

// This declares this class as psana module
using namespace cheetah_ana_pkg;
using namespace std;
PSANA_MODULE_FACTORY(cheetah_ana_mod)


//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------
namespace cheetah_ana_pkg {
    cGlobal cheetahGlobal;
	std::queue<pthread_t> runningThreads;
	volatile bool runCheetahCaller;
	pthread_t cheetahCallerThread;
	pthread_mutex_t pthread_queue_mutex;

	void sig_handler(int signo)
	{
		if (signo == SIGINT){
			// Wait for threads to finish
			printf("Waiting for cheetah caller to terminate\n");
			runCheetahCaller = false;
			pthread_join(cheetahCallerThread,NULL);
			while(cheetahGlobal.nActiveCheetahThreads > 0) {
				printf("Waiting for %li worker threads to terminate\n", cheetahGlobal.nActiveCheetahThreads);
				usleep(500000);
			}
			printf("Attempting to close CXIs cleanly\n");
			writeAccumulatedCXI(&cheetahGlobal);
			closeCXIFiles(&cheetahGlobal);
			signal(SIGINT,SIG_DFL);
			kill(getpid(),SIGINT);
	    
		}
	}



	
	//----------------
	// Constructors --
	//----------------
	cheetah_ana_mod::cheetah_ana_mod (const std::string& name)
		: Module(name) 
	{
		startT = 0;
//		cout << "*** Constructor ***" << endl;
        
		// Check if we're using psana of the same git commit
		if(!getenv("PSANA_GIT_SHA") || strcmp(getenv("PSANA_GIT_SHA"),GIT_SHA1)){
			fprintf(stderr,    "*******************************************************************************************\n");
			fprintf(stderr,"*** WARNING %s:%d ***\n",__FILE__,__LINE__);

			if(getenv("PSANA_GIT_SHA")){
				fprintf(stderr,"***        Using psana from git commit %s         ***\n",getenv("PSANA_GIT_SHA"));
				fprintf(stderr,"***        and cheetah_ana_mod from git commit %s ***\n",GIT_SHA1);
			}else{
				fprintf(stderr,"***         Using a psana version not compiled with cheetah!                            ***\n");
			}
			fprintf(stderr,    "*******************************************************************************************\n");
			sleep(10);
		}
		setenv("CHEETAH_ANA_MOD_GIT_SHA",GIT_SHA1,0);

		// get the values from configuration or use defaults
		m_key = configStr("inputKey", "");
		m_srcCspad0 = configStr("cspadSource0","DetInfo(:Cspad)");
		m_srcCspad1 = configStr("cspadSource1","DetInfo(:Cspad)");
		m_srcCspad2x2 = configStr("cspad2x2Source0","DetInfo(:Cspad2x2)");
		m_srcPnccd0 = configStr("pnccdSource0","DetInfo(:pnCCD)");
		m_srcPnccd1 = configStr("pnccdSource1","DetInfo(:pnCCD)");
		m_srcEvr = configStr("evrSource","DetInfo(:Evr)");
		m_srcBeam = configStr("beamSource","BldInfo(EBeam)");
		m_srcFee = configStr("feeSource","BldInfo(FEEGasDetEnergy)");
		m_srcFeeSpec = configStr("feeSpectrum","BldInfo(FEE-SPEC0)");
		m_srcCav = configStr("cavitySource","BldInfo(PhaseCavity)");
		m_srcSpec = configStr("spectrumSource","DetInfo()");
		m_srcCam = configStr("cameraSource","DetInfo()");

		nActiveAnaThreads = 0;
		pthread_mutex_init(&nActiveThreads_mutex, NULL);
		pthread_mutex_init(&counting_mutex, NULL);
		pthread_mutex_init(&pthread_queue_mutex, NULL);

		runCheetahCaller = true;
		int returnStatus = pthread_create(&cheetahCallerThread, NULL, cheetah_caller, &pthread_queue_mutex);

		if (returnStatus != 0) { // creation successful
			printf("Error: thread creation failed\n");
		}
	}

	//--------------
	// Destructor --
	//--------------
	cheetah_ana_mod::~cheetah_ana_mod ()
	{
	}

	
	/// Method which is called once at the beginning of the job
	void cheetah_ana_mod::beginJob(Event& evt, Env& env)
	{
		// This is to silence compiler warning about unused variable
		(void)evt;
		(void)env;
		//
		//cout << "*** beginJob ***" << endl;
		time(&startT);
		if (cheetahInit(&cheetahGlobal)){
			exit(0);
		}
		if(cheetahGlobal.saveCXI){
			signal(SIGINT, sig_handler);
		}

		/* We need to know the names of the TOF detector source
		   from the cheetah config file, that's why we're doing this here */
		for(int i = 0;i<cheetahGlobal.nTOFDetectors;i++){
			m_srcAcq.push_back((Source)configStr(cheetahGlobal.tofDetector[i].sourceName, 
												 cheetahGlobal.tofDetector[i].sourceIdentifier));
		}
	}

	
	/// Method which is called at the beginning of the run
	///	Pass new run information to Cheetah
	void cheetah_ana_mod::beginRun(Event& evt, Env& env)
	{
		cout << "Experiment = " << env.experiment() << endl;
		//cout << "*** beginRun ***" << endl;
		int runNumber = 0;
		PSTime::Time evtTime;
		boost::shared_ptr<PSEvt::EventId> eventId = evt.get();
		
		if (eventId.get()) {
			runNumber = eventId->run();
		}

		cheetahGlobal.runNumber = runNumber;

		// Set CXI file name				
		sprintf(cheetahGlobal.cxiFilename,"%s-r%04d.cxi", env.experiment().c_str(),runNumber);

		cheetahNewRun(&cheetahGlobal);
		printf("User analysis beginrun() routine called.\n");
		printf("*** Processing r%04u ***\n",runNumber);
	}


	/// Method which is called at the beginning of the calibration cycle
	void 
	cheetah_ana_mod::beginCalibCycle(Event& evt, Env& env)
	{
		// This is to silence compiler warning about unused variable
		(void)evt;
		//

		cout << "beginCalibCycle()" << endl;
		
		/*
		 *	pnCCD configuration
		 *	configuration objects (PNCCD::ConfigV2) do not appear in event but are stored in the environment. 
		 *	You should use the code like this to get configuration data and you should always check that 
		 *	you are returned non-zero pointer:
		 *	
		 *	config = env.configStore().get(m_srcPnccd);
		 *	if (not config) {
		 *		MsgLog(name(), error, "failed to retrieve configuration object , address: " << m_srcPnccd);
		 *		return;
		 *	}
		 */
		for(long detIndex=0; detIndex < cheetahGlobal.nDetectors; detIndex++)  {

			if (!strcmp(cheetahGlobal.detector[detIndex].detectorType, "pnccd")) {

				// Need to make this do 
				shared_ptr<Psana::PNCCD::ConfigV1> config1 = env.configStore().get(m_srcPnccd0);
				shared_ptr<Psana::PNCCD::ConfigV2> config2 = env.configStore().get(m_srcPnccd0);

				if (config1.get()) {
					cout << "PNCCD::ConfigV1:" << endl;
					cout << "\n  numLinks = " << config1->numLinks() << endl;
					cout << "\n  payloadSizePerLink = " << config1->payloadSizePerLink() << endl;
				}
				else if (config2.get()) {
					cout << "PNCCD::ConfigV2:" << endl;
					cout << "\n  numLinks = " << config2->numLinks() << endl;
					cout << "\n  payloadSizePerLink = " << config2->payloadSizePerLink() << endl;
					cout << "\n  numChannels = " << config2->numChannels() << endl;
					cout << "\n  numRows = " << config2->numRows() << endl;
					cout << "\n  numSubmoduleChannels = " << config2->numSubmoduleChannels() << endl;
					cout << "\n  numSubmoduleRows = " << config2->numSubmoduleRows() << endl;
					cout << "\n  numSubmodules = " << config2->numSubmodules() << endl;
					cout << "\n  camexMagic = " << config2->camexMagic() << endl;
					cout << "\n  info = " << config2->info() << endl;
					cout << "\n  timingFName = " << config2->timingFName() << endl;
				}
				else {
					cout << "Failed to retrieve pnCCD configuration object. " << detIndex << endl;
					cout << "cheetah_ana_mod::beginCalibCycle(Event& evt, Env& env)" << endl;
					cout << "Exiting..." << endl;
					exit(1);
				}
			}
			else {
				cout << "No configuration data" << endl;
			}
		}
	}


	///
	///	Event method
	/// This method is called with event data
	///	Start the threads which will copy across data into Cheetah structure and process
	///
	void cheetah_ana_mod::event(PSEvt::Event& evt, PSEnv::Env& env) {
		boost::shared_ptr<Event> evtp = evt.shared_from_this();
		boost::shared_ptr<Env> envp = env.shared_from_this();
		pthread_t thread;
		int returnStatus;
		/*
		 *  Wait until we have a spare thread in the thread pool
		 */
		while((nActiveAnaThreads >= cheetahGlobal.anaModThreads) || (cheetahGlobal.useSingleThreadCalibration && (cheetahGlobal.nActiveCheetahThreads > 1) && !cheetahGlobal.calibrated)) {
			usleep(10000);
		}
		
		// Create a new worker thread for this data frame
		returnStatus = pthread_create(&thread, NULL, threaded_event, (void*) new AnaModEventData(this, evtp, envp));		

		if (returnStatus == 0) { // creation successful
			// Increment threadpool counter
			pthread_mutex_lock(&nActiveThreads_mutex);
			nActiveAnaThreads += 1;
			pthread_mutex_unlock(&nActiveThreads_mutex);

			pthread_mutex_lock(&pthread_queue_mutex);
			runningThreads.push(thread);
			pthread_mutex_unlock(&pthread_queue_mutex);
		}
		else{
			printf("Error: thread creation failed (frame skipped)\n");
		}
	}
	// End of psana event method


	int cheetah_ana_mod::readTOF(Event & evt, Env & env,
								 cEventData* eventData){
		int tofPresent = 0;
		for(int i = 0;i<cheetahGlobal.nTOFDetectors;i++){
			cTOFDetectorEvent & tofEvent = eventData->tofDetector[i];
			cTOFDetectorCommon & tofDetector = cheetahGlobal.tofDetector[i];
			shared_ptr<Psana::Acqiris::DataDescV1> acqData = evt.get(m_srcAcq[i]);
			if (acqData) {
				int channel = tofDetector.channel;
				shared_ptr<Psana::Acqiris::ConfigV1> acqConfig = env.configStore().get(m_srcAcq[i]);
				const Psana::Acqiris::DataDescV1Elem& elem = acqData->data(channel);
				const Psana::Acqiris::VertV1& v = acqConfig->vert()[channel];
				double slope = v.slope();
				tofEvent.slope = slope;
				double offset = v.offset();
				tofEvent.offset = offset;
				const Psana::Acqiris::HorizV1& h = acqConfig->horiz();
				double sampInterval = h.sampInterval();
				tofEvent.sampInterval = sampInterval;
				const ndarray<const Psana::Acqiris::TimestampV1, 1>& timestamps = elem.timestamp();
				const ndarray<const int16_t, 2>& waveforms = elem.waveforms();
				int seg = 0;
				tofEvent.trigTime = timestamps[seg].pos();
				double timestamp = timestamps[seg].value();
				tofEvent.timeStamp = timestamp;
				ndarray<const int16_t, 1> raw(waveforms[seg]);
				for (int i = 0; i < tofDetector.numSamples; ++ i) {
					tofEvent.time.push_back(timestamp + i*sampInterval);
					tofEvent.voltage.push_back(raw[i]*slope - offset);
				}
				tofPresent = 1;
				
			}
		}
		return tofPresent;
	}
								  
	 
		
		
	/// Method which is called at the end of the calibration cycle
	void 
	cheetah_ana_mod::endCalibCycle(Event& evt, Env& env)
	{
		// This is to silence compiler warning about unused variable
		(void)evt;
		(void)env;
		//
	}

	void cheetah_ana_mod::waitForAllWorkers(){
		waitForCheetahWorkers();
		waitForAnaModWorkers();
	}

	void cheetah_ana_mod::waitForCheetahWorkers(){
		time_t	tstart, tnow;
		time(&tstart);
		double	dtime;
		int p=0, pp=0;

		while(cheetahGlobal.nActiveCheetahThreads > 0) {
			p = cheetahGlobal.nActiveCheetahThreads;
			if ( pp != p){
				pp = p;
				printf("Waiting for %li worker threads to finish.\n", cheetahGlobal.nActiveCheetahThreads);
			}
			time(&tnow);
			dtime = difftime(tnow, tstart);
			if(( dtime > ((float) cheetahGlobal.threadTimeoutInSeconds) ) && (cheetahGlobal.threadTimeoutInSeconds > 0)) {
				printf("\t%li threads still active after waiting %f seconds\n", cheetahGlobal.nActiveCheetahThreads, dtime);
				printf("\tGiving up and exiting anyway\n");
				cheetahGlobal.nActiveCheetahThreads = 0;
				break;
			}
			usleep(500000);
		}
		printf("Cheetah workers stopped successfully.\n");
		
	}

	void cheetah_ana_mod::waitForAnaModWorkers(){
		printf("Waiting for %d ana mod workers to finish.\n", nActiveAnaThreads);
		while(nActiveAnaThreads > 0) {
			usleep(50000);
		}
		printf("Ana mod workers stopped successfully.\n");
	}

	/// Method which is called at the end of the run
	void cheetah_ana_mod::endRun(Event& evt, Env& env)
	{
		// This is to silence compiler warning about unused variable
		(void)evt;
		(void)env;
		//
		
		/*
		 *	Wait for all worker threads to finish
		 *	Sometimes the program hangs here, so wait no more than 10 minutes before exiting anyway
		 */
		printf("Ending run.\n");
		waitForAnaModWorkers();		
		waitForAllWorkers();		
		
		if(cheetahGlobal.saveCXI) {
			printf("Writing accumulated CXIDB file\n");
			writeAccumulatedCXI(&cheetahGlobal);
			closeCXIFiles(&cheetahGlobal);
		}
	}


	/// Method which is called once at the end of the job
	///	Clean up all variables associated with libCheetah
	void cheetah_ana_mod::endJob(Event& evt, Env& env)
	{
		// This is to silence compiler warning about unused variable
		(void)evt;
		(void)env;
		//

		printf("Ending job. ");
		waitForAnaModWorkers();		
		cheetahExit(&cheetahGlobal);
	  
		time_t endT;
		time(&endT);
		double dif = difftime(endT,startT);
		cout << "time taken: " << dif << " seconds" << endl;
		// We shouldn't exit, and specially not with a value of 1
		//	  exit(1);
		// Just retuning allows the proper destructors to be called
		return;
	}


	void * threaded_event(void* threadData){
		boost::shared_ptr<AnaModEventData> data((AnaModEventData*) threadData);
		data->module->copy_event(data->evtp, data->envp);
		return 0;
	}
	

	// This function joins all the threads that copied the Ana events and calls cheetah
	void * cheetah_caller(void* ){
		while(runCheetahCaller){
			pthread_mutex_lock(&pthread_queue_mutex);
			bool empty = runningThreads.empty();
			// sleep if empty
			if(empty){
				// Unlikely to happen after the beginning
				pthread_mutex_unlock(&pthread_queue_mutex);
				usleep(100000);
				continue;
			}
			pthread_t thread = runningThreads.front();
			cEventData * eventData;
			runningThreads.pop();
			pthread_mutex_unlock(&pthread_queue_mutex);
			pthread_join(thread,(void **)&eventData);
			cheetahProcessEventMultithreaded(&cheetahGlobal, eventData);
			
		}
		pthread_exit(NULL);
		return 0;
	}
} // namespace cheetah_ana_pkg
