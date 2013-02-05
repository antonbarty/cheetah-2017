//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class chuck_ana_mod...
//
// Author List:
//      Chunhong Yoon
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "chuck_ana_pkg/chuck_ana_mod.h"
//#include "/reg/neh/home3/yoon82/cheetah/source/cheetah.lib/cheetah.h"
#include "../../../cheetah.lib/cheetah.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <iostream>
#include <string>
#include <iomanip>
//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
// to work with detector data include corresponding 
// header from psddl_psana package
#include "psddl_psana/bld.ddl.h"
#include "PSEvt/EventId.h"
#include "psddl_psana/cspad.ddl.h"
#include "psddl_psana/evr.ddl.h"
#include "psddl_psana/acqiris.ddl.h"
#include "psddl_psana/camera.ddl.h"
#include "psddl_psana/cspad2x2.ddl.h"

#include "../../../lcls/myana/release/pdsdata/cspad/ElementIterator.hh"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>

#define beamCode 140
#define laserCode 41
#define verbose 0
	
#define outputImage 0	// use to output dark frame	(no region)
#define subtractImage 0	// use to subtract dark frame from fluorescence frame (darkRegion)
#define subtractNGainImage 0 // // use to subtract dark frame from fluorescence frame followed by gain correction (darkRegion)
#define lookAtSubregion 0 // use to look at the position of the subregion wrt ASIC	(darkRegion)
#define lookAtGain 0 // use to look at the position of gainmap	(no region)
#define generateDark 0		// generate average dark	(no region)
#define sumSubsectionMinusDark 0	// Dark corrected calculations (darkSubregion)
#define sumSubsectionMinusDarkCommonMode 0	// Dark background and common mode corrected calculations (darkSubregion)
#define correctFluores 1 // correct for dark, common mode and gain map
#define generateHistogram 0		// histogram of all pixel values for a run (darkSubregion)
#define generateDarkFluctuation 0	// fluctuation of a pixel shot to shot for a dark (darkSubregion)
#define generateMeanAsicDark 0	// fluctuation of mean Asic shot to shot for dark run (no region)
//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

// This declares this class as psana module
using namespace chuck_ana_pkg;
using namespace std;
PSANA_MODULE_FACTORY(chuck_ana_mod)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace chuck_ana_pkg {
static int count = 0;
static int nonzeroCount = 0;
static int runCount = 0;
static int nevent = 0;
static int hotpixel = 3000;
static int histOffset = 500;	// Also store minus pixel values
int histLength = 10000;
int pickR = 0;
int pickC = 0;

static cGlobal cheetahGlobal;
char detZ[] = "CXI:DS1:MMS:06.RBV";
// Subregion rectangle
int sx = 40;
int ex = 145;
int sy = 210;
int ey = 265;
int dx = ex-sx;
int dy = ey-sy;
// common mode rectangle
int csx = 10;
int cex = 175;
int csy = 330;
int cey = 360;
int cdx = cex-csx;
int cdy = cey-csy;
// CsPad2x1 dimension
int dimX = 185;
int dimY = 388;
// Output files
ofstream outStats("cspad2x2_stats.txt"); // output mean and stdev for all runs

vector<vector<double> > gainRegion ( 2*dimX, vector<double> (dimY) ); // CsPad2x2
vector<vector<double> > cmSubregion ( cdx, vector<double> (cdy) );
vector<vector<double> > darkSubregion ( dx, vector<double> (dy) );
vector<vector<double> > darkRegion ( dimX, vector<double> (dimY) );
vector<double> photonEnergy;
vector<double> pulseEnergy;
vector<double> fluores;
vector<double> meanFluores;
vector<double> stdFluores;
vector<int> runNumber;
vector<int> numImages;
vector<double> hist(histLength,0);
vector<double> meanAsic0; // upper left
vector<double> meanAsic1; // lower left
vector<double> meanAsic2; // upper right
vector<double> meanAsic3; // lower right
vector<double> commonMode;

template <typename T>
bool eventCodePresent(const ndarray<T, 1>& array, unsigned EvrCode){
  //const int nfifo =  array.size();
  for (unsigned i = 0; i < array.size(); ++i) {
	if (array[i].eventCode() == EvrCode) {
		return true;
	}
  }
  return false;
}

string trim( const string& s )
  {
  string result( s );
  result.erase( result.find_last_not_of( " " ) + 1 );
  result.erase( 0, result.find_first_not_of( " " ) );
  return result;
  }

void readin1D(string vecFile, vector<double>& myVec){
	cout << "Reading in common mode" << endl;
	string input_buffer;
	ifstream infConfig(vecFile.c_str());
	if (infConfig.is_open()) {
		//while (!infConfig.eof()) {
			getline(infConfig,input_buffer);
			istringstream iss( input_buffer );
			string word;
			while (getline( iss, word, ' ' )){
				trim(word);
				myVec.push_back( atof(word.c_str()) );
			}
		//}
	}
	cout << "Done reading in common mode" << endl;
}

void readinDark(string darkFile, vector<vector<double> >& dark){
	cout << "Reading in Dark" << endl;
	string input_buffer;
	ifstream infConfig(darkFile.c_str());
	int cntrR = 0; // counter Row
	int cntrC = 0; // counter Col
	if (infConfig.is_open()) {
		while (!infConfig.eof()) {
			getline(infConfig,input_buffer);
			//cout << "****" << input_buffer << endl;
			istringstream iss( input_buffer );
  			string word;
 			cntrC = 0; // counter Col
			while (getline( iss, word, ' ' )){
    			//cout << "word " << cntrC << ": " << trim( word ) << '\n';
				trim(word);
//				if (subtractImage || lookAtSubregion || subtractNGainImage){
					dark[cntrC][cntrR] = atof(word.c_str()); // 388 x 185
					// cntrC=185 and cntrR=388
//				}
//				if (sumSubsectionMinusDarkCommonMode ||sumSubsectionMinusDark || generateHistogram || generateDarkFluctuation) {
//					darkSubregion[cntrC][cntrR] = atof(word.c_str());
//				}
				cntrC++;
    		}
			cntrR++;
		}
	}
	cout << "Done reading in Dark" << endl;
}

// Read in gainCoor_clean_v1_angle.txt transposed
void readinGain(string gainFile, vector<vector<double> >& gain){
	cout << "Reading in Gain" << endl;
	string input_buffer;
	ifstream infConfig(gainFile.c_str());
	int cntrR = 0; // counter Row
	int cntrC = 0; // counter Col
	if (infConfig.is_open()) {
		while (!infConfig.eof()) {
			getline(infConfig,input_buffer);
			//cout << "****" << cntrR << ": " << input_buffer << endl;
			istringstream iss( input_buffer );
  			string word;
 			cntrR = 0; // counter Row
			while (getline( iss, word, ' ' )){
    			//cout << "word " << cntrC << ": " << trim( word ) << '\n';
				trim(word);
				//cout << cntrR << " " << word << endl;
				gain[cntrC][cntrR] = atof(word.c_str()); // 388 x 370
				// cntrC=370 and cntrR=388
				cntrR++;
    		}
			cntrC++;
		}
	}
	cout << "Done reading in Gain" << endl;
}

//----------------
// Constructors --
//----------------
chuck_ana_mod::chuck_ana_mod (const std::string& name)
  : Module(name)
  , m_src()
  , m_maxEvents()
  , m_filter()
  , m_count(0)
{
    // get the values from configuration or use defaults
    //m_src = configStr("source", "DetInfo(:Acqiris)");
    m_maxEvents = config("events", 32U);
    m_filter = config("filter", false);
	m_key = configStr("inputKey", "");
	m_src = configStr("source","DetInfo(:Cspad)");
	m_srcEvr = configStr("evrSource","DetInfo(:Evr)");
	m_srcBeam = configStr("beamSource","BldInfo(:EBeam)");
	m_srcFee = configStr("feeSource","BldInfo(:FEEGasDetEnergy)");
	m_srcCav = configStr("cavitySource","BldInfo(:PhaseCavity)");
	m_srcAcq = configStr("acqirisSource","DetInfo(:Acqiris)");
	m_srcCam = configStr("cameraSource","DetInfo()");
  	m_src2x2 = configStr("source2x2", "DetInfo(:Cspad2x2)");
}

//--------------
// Destructor --
//--------------
chuck_ana_mod::~chuck_ana_mod (){}

/// Method which is called once at the beginning of the job
void chuck_ana_mod::beginJob(Event& evt, Env& env)
{
	cout << "begin beginJob" << endl;
	//if (subtractImage || lookAtSubregion || subtractNGainImage) {
		//readinDark("/reg/neh/home3/yoon82/cheetah/source/lcls-psana/dark_region0.txt"); // 388 x 185
		readinDark("/reg/neh/home3/yoon82/cheetah/source/lcls-psana/dark_region.txt", darkRegion); // 388 x 185
	//}

	//if (sumSubsectionMinusDarkCommonMode || sumSubsectionMinusDark || generateHistogram || generateDarkFluctuation) {
		readinDark("/reg/neh/home3/yoon82/cheetah/source/lcls-psana/dark112_cspad2x2",darkSubregion); // 55 x 105
	//}

	readinGain("/reg/neh/home3/yoon82/cheetah/source/lcls-psana/gainCoor_clean_v1_angle.txt",gainRegion); // 388 x 370
	
	readin1D("/reg/neh/home3/yoon82/cheetah/source/lcls-psana/r110_cspad2x2_commonModeDark",commonMode);

	cout << "done beginJob" << endl;
}

/// Method which is called at the beginning of the run
void chuck_ana_mod::beginRun(Event& evt, Env& env)
{
	cout << "runCount: " << runCount << endl;
	runCount++;
	count = 0;
	nonzeroCount = 0;
	nevent = 0;

	// clear vectors
	fluores.clear();
	photonEnergy.clear();
	pulseEnergy.clear();
	//hist.clear();
	for (int i = 0; i < histLength; i++){
		hist[i] = 0;
	}
	cout << "end beginRun" << endl;
}

/// Method which is called at the beginning of the calibration cycle
void chuck_ana_mod::beginCalibCycle(Event& evt, Env& env){}

/// Method which is called with event data, this is the only required 
/// method, all other methods are optional
void chuck_ana_mod::event(Event& evt, Env& env)
{
nevent++;

if (generateMeanAsicDark) {
	shared_ptr<Psana::CsPad2x2::ElementV1> elem1 = evt.get(m_src2x2, m_key);
	if (elem1.get()) {
//		const ndarray<int16_t, 3>& data = elem1->data();
		const ndarray<const int16_t, 3> data = elem1->data();
		int asicHeight = dimY/2;
		int numPix = dimX*asicHeight;
		double mean0 = 0;
		double mean1 = 0;
		double mean2 = 0;
		double mean3 = 0;
		for (int i = 0; i < dimX; i++) {
		for (int j = 0; j < asicHeight; j++) {
			mean0 += data[i][j+asicHeight][0];
			mean1 += data[i][j][0];
			mean2 += data[i][j+asicHeight][1];
			mean3 += data[i][j][1];
		}
		}
		meanAsic0.push_back(mean0/numPix); // must add since it is flipped
		meanAsic1.push_back(mean1/numPix);
		meanAsic2.push_back(mean2/numPix);
		meanAsic3.push_back(mean3/numPix);
	}
}

if (generateDarkFluctuation) {
	shared_ptr<Psana::CsPad2x2::ElementV1> elem1 = evt.get(m_src2x2, m_key);
	if (elem1.get()) {
		const ndarray<const int16_t, 3>& data = elem1->data();
		pickC = 102; 	// pick a value between sx and ex, (40,145)
		pickR = 223;	// pick a value between sy and ey, (210,265)
		int k = 1;
		fluores.push_back(data[pickC][pickR][k] - darkSubregion[pickC-sx][pickR-sy]);
	}
}

if (generateHistogram) {
	shared_ptr<Psana::CsPad2x2::ElementV1> elem1 = evt.get(m_src2x2, m_key);
	if (elem1.get()) {
		const ndarray<const int16_t, 3>& data = elem1->data();
		int k = 1;
		for (int i = sx; i < ex; i++) { // width
		for (int j = sy; j < ey; j++) { // height
			int val = int (ceil(data[i][j][k] - darkSubregion[i-sx][j-sy]));
			if (val+histOffset <= histLength && val+histOffset >= 0) {
				hist[val+histOffset] = hist[val+histOffset] + 1;
			}
		}
		}
		count++; // frame count
	}
}

if (correctFluores) {
	shared_ptr<Psana::CsPad2x2::ElementV1> elem1 = evt.get(m_src2x2, m_key);
	if (elem1.get()) {
		const ndarray<const int16_t, 3>& data = elem1->data();
		int k = 1;
		double sumPerShot = 0;
		int goodPixPerShot = 0;
		// fluores area
		for (int i = sx; i < ex; i++) { // width
		for (int j = sy; j < ey; j++) { // height
			if (data[i][j][k] > 0 && data[i][j][k] < hotpixel) {
				sumPerShot += ( data[i][j][k] - darkSubregion[i-sx][j-sy] - commonMode[nevent] ) / gainRegion[i+k*dimX][j];
				goodPixPerShot++;
			}
		}
		}
		fluores.push_back(sumPerShot/(double)goodPixPerShot);
	}
}

if (sumSubsectionMinusDarkCommonMode) {
// Subsection: CsPad2x2 - Dark
	shared_ptr<Psana::CsPad2x2::ElementV1> elem1 = evt.get(m_src2x2, m_key);
	if (elem1.get()) {
		double ev = getPhotonEnergyeV(evt, env);
		double pulseEv = getPulseEnergymJ(evt, env);
		nonzeroCount++;
		const ndarray<const int16_t, 3>& data = elem1->data();
		int k = 1;
		double sumPerShot = 0;
		int goodPixPerShot = 0;
		// fluores area
		for (int i = sx; i < ex; i++) { // width
		for (int j = sy; j < ey; j++) { // height
			if (data[i][j][k] > 0 && data[i][j][k] < hotpixel) {
				sumPerShot += data[i][j][k] - darkSubregion[i-sx][j-sy];
				goodPixPerShot++;
			}
		}
		}
		fluores.push_back(sumPerShot/(double)goodPixPerShot);
		photonEnergy.push_back(ev);
		pulseEnergy.push_back(pulseEv);
		sumPerShot = 0;
		goodPixPerShot = 0;
		// common mode dark area
		for (int i = csx; i < cex; i++) { // width
		for (int j = csy; j < cey; j++) { // height
			if (data[i][j][k] > 0 && data[i][j][k] < hotpixel) {
				sumPerShot += data[i][j][k];
				goodPixPerShot++;
			}
		}
		}
		meanAsic3.push_back(sumPerShot/(double)goodPixPerShot);
	}
}

if (sumSubsectionMinusDark) {
// Subsection: CsPad2x2 - Dark
	shared_ptr<Psana::CsPad2x2::ElementV1> elem1 = evt.get(m_src2x2, m_key);
	if (elem1.get()) {
		double ev = getPhotonEnergyeV(evt, env);
		double pulseEv = getPulseEnergymJ(evt, env);
		//if (ev > 0) { // sometimes ebeam data is not recorded
			nonzeroCount++;
			const ndarray<const int16_t, 3>& data = elem1->data();
			int k = 1;
			double sumPerShot = 0;
			int goodPixPerShot = 0;
			for (int i = sx; i < ex; i++) { // width
			for (int j = sy; j < ey; j++) { // height
				if (data[i][j][k] > 0 && data[i][j][k] < hotpixel) {
					sumPerShot += data[i][j][k] - darkSubregion[i-sx][j-sy];
					goodPixPerShot++;
				}
			}
			}
			fluores.push_back(sumPerShot/(double)goodPixPerShot);
			photonEnergy.push_back(ev);
			pulseEnergy.push_back(pulseEv);
		//}
	}
}

if (generateDark) { // This works on a subregion
// Darkcal: Sum subsection of CsPad2x2
	shared_ptr<Psana::CsPad2x2::ElementV1> elem1 = evt.get(m_src2x2, m_key);
	if (elem1.get()) {
		double ev = getPhotonEnergyeV(evt, env);
		double pulseEv = getPulseEnergymJ(evt, env);
		//if (ev > 0) { // sometimes ebeam data is not recorded
			nonzeroCount++;
			photonEnergy.push_back(ev);
			pulseEnergy.push_back(pulseEv);
			const ndarray<const int16_t, 3>& data = elem1->data();
			int k = 1; // second CsPad2x1
			double sumPerShot = 0;
			int goodPixPerShot = 0;
			for (int i = sx; i < ex; i++) { // width
			for (int j = sy; j < ey; j++) { // height
				darkSubregion[i-sx][j-sy] += data[i][j][k];
				if (data[i][j][k] > 0 && data[i][j][k] < hotpixel) {
					sumPerShot += data[i][j][k];
					goodPixPerShot++;
				}
			}
			}
			fluores.push_back(sumPerShot/(double)goodPixPerShot);
		//}
	}
}

if (lookAtSubregion) {
		ofstream outFile("cspad2x2_subregion.txt");
		shared_ptr<Psana::CsPad2x2::ElementV1> elem1 = evt.get(m_src2x2, m_key);
		if (elem1.get()) {
			const ndarray<const int16_t, 3>& data = elem1->data();
			for (int j = 0; j < dimY; j++) {
			for (int i = 0; i < dimX; i++) {
				int k = 1;
				if ( i > sx && i < ex && j > sy && j < ey) {
					outFile << 0 << " ";
				} else {
					outFile << data[i][j][k] - darkRegion[i][j] << " ";
				}
			}
			outFile << endl;
			}
		}
		outFile.close();
		exit(1);
}

if (lookAtGain) {
	ofstream outFile("cspad2x2_gain.txt");
	for (int j = 0; j < dimY; j++) { // 388
	for (int i = 0; i < 2*dimX; i++) { // 370
		outFile << gainRegion[i][j] << " ";
	}
	outFile << endl;
	}
	outFile.close();
	exit(1);
}

// Subtract a Fluorescence image by Dark then divide by gain
if (subtractNGainImage) {
		ofstream outFile("cspad2x2_DarkSubtractedGain.txt");
		shared_ptr<Psana::CsPad2x2::ElementV1> elem1 = evt.get(m_src2x2, m_key);
		if (elem1.get()) {
			const ndarray<const int16_t, 3>& data = elem1->data();
			for (int j = 0; j < dimY; j++) { // 388
			for (int i = 0; i < dimX; i++) { // 185
				int k = 1;
				outFile << (data[i][j][k] - darkRegion[i][j]) / gainRegion[i+k*dimX][j] << " ";
			}
			outFile << endl;
			}
		}
		outFile.close();
		exit(1);
}

// Subtract a Fluorescence image by Dark
if (subtractImage) {
		//ofstream outFile("cspad2x2_DarkSubtracted0.txt");
		ofstream outFile("cspad2x2_DarkSubtracted.txt");
		shared_ptr<Psana::CsPad2x2::ElementV1> elem1 = evt.get(m_src2x2, m_key);
		if (elem1.get()) {
			const ndarray<const int16_t, 3>& data = elem1->data();
			for (int j = 0; j < dimY; j++) { // 388
			for (int i = 0; i < dimX; i++) { // 185
				int k = 1;
				outFile << data[i][j][k] - darkRegion[i][j] << " ";
			}
			outFile << endl;
			}
		}
		outFile.close();
		exit(1);
}

// Print half of CsPad2x2 to file 
if (outputImage) {
		//ofstream outFile("fluores_region.txt"); // modify myPsana1 to r109
		//ofstream outFile("fluores_region0.txt"); // modify myPsana1 to r109
		//ofstream outFile("dark_region0.txt"); // modify myPsana1 to r117
		ofstream outFile("dark_region.txt"); // modify myPsana1 to r117
		shared_ptr<Psana::CsPad2x2::ElementV1> elem1 = evt.get(m_src2x2, m_key);
		if (elem1.get()) {
			const ndarray<const int16_t, 3>& data = elem1->data();
			int k = 1;
			for (int j = 0; j < dimY; j++) { // 388
			for (int i = 0; i < dimX; i++) { // 185
				outFile << data[i][j][k] << " ";
			}
			outFile << endl;
			}
		}
		outFile.close();
		exit(1);
}

}
  
/// Method which is called at the end of the calibration cycle
void chuck_ana_mod::endCalibCycle(Event& evt, Env& env){}

/// Method which is called at the end of the run
void chuck_ana_mod::endRun(Event& evt, Env& env)
{
	cout << "begin endRun" << endl;

	if (generateMeanAsicDark) {
		// Save stats
		int run = 0;
  		PSTime::Time evtTime;
  		boost::shared_ptr<PSEvt::EventId> eventId = evt.get();
  		if (eventId.get()) {
			run = eventId->run();
		}
		stringstream sstm;
		sstm << "dark" << run << "_cspad2x2_meanAsic";
		string result = sstm.str();
		ofstream outFile(result.c_str()); // modify myPsana1 to r117
		// Print mean Asic
		for (unsigned i = 0; i < meanAsic0.size(); i++) { // number of frames
			outFile << meanAsic0[i] << " ";
		}
		outFile << endl;
		for (unsigned i = 0; i < meanAsic1.size(); i++) { // number of frames
			outFile << meanAsic1[i] << " ";
		}
		outFile << endl;
		for (unsigned i = 0; i < meanAsic2.size(); i++) { // number of frames
			outFile << meanAsic2[i] << " ";
		}
		outFile << endl;
		for (unsigned i = 0; i < meanAsic3.size(); i++) { // number of frames
			outFile << meanAsic3[i] << " ";
		}
		outFile << endl;
	}

	if (generateDarkFluctuation) {
		int run = 0;
  		PSTime::Time evtTime;
  		boost::shared_ptr<PSEvt::EventId> eventId = evt.get();
  		if (eventId.get()) {
			run = eventId->run();
		}
		// Write out files
		stringstream sstm;
		sstm << "r" << run << "_cspad2x2_darkPixelFluctuation" << pickR << "_" << pickC;
		string result = sstm.str();
  		ofstream outFlu(result.c_str()); // output fluorescence for all shots
  		for (unsigned i = 0; i < fluores.size(); i++ ) {
    		outFlu << fluores[i] << " ";
		}
  		outFlu << endl;
		outFlu.close();
	}

	if (generateHistogram) {
		int run = 0;
  		PSTime::Time evtTime;
  		boost::shared_ptr<PSEvt::EventId> eventId = evt.get();
  		if (eventId.get()) {
			run = eventId->run();
		}
		// Write out files
		stringstream sstm;
		sstm << "r" << run << "_cspad2x2_pixelHistogram";
		string result = sstm.str();
  		ofstream outFlu(result.c_str()); // output fluorescence for all shots
		//cout << "hist: " << hist.size() << endl;
  		for (unsigned i = 0; i < hist.size(); i++ ) {
			//cout << hist[i]/count << endl;
    			outFlu << hist[i]/count << " ";
		}
  		outFlu << endl;
		outFlu.close();
	}

	if (correctFluores) {
		int run = 0;
  		PSTime::Time evtTime;
  		boost::shared_ptr<PSEvt::EventId> eventId = evt.get();
  		if (eventId.get()) {
			run = eventId->run();
		}
		// Write out files
		stringstream sstm;
		sstm << "r" << run << "_cspad2x2_fluoresCorrect";
		string result = sstm.str();
  		ofstream outFlu(result.c_str()); // output fluorescence for all shots
  		for (unsigned i = 0; i < fluores.size(); i++ ) {
    		outFlu << fluores[i] << " ";
		}
  		outFlu << endl;
		outFlu.close();
	}

	if (sumSubsectionMinusDarkCommonMode) {
		// Save stats
		int run = 0;
  		PSTime::Time evtTime;
  		boost::shared_ptr<PSEvt::EventId> eventId = evt.get();
  		if (eventId.get()) {
			run = eventId->run();
			runNumber.push_back(run);
		}
		numImages.push_back(nonzeroCount);
		// Calculate mean and stdev fluorescence
		double sum = accumulate(fluores.begin(), fluores.end(), 0.0);		
		double mean = sum / fluores.size();

		double sq_sum = inner_product(fluores.begin(), fluores.end(), fluores.begin(), 0.0);
		double stdev = sqrt(sq_sum / fluores.size() - mean * mean);

		meanFluores.push_back(mean);
		stdFluores.push_back(stdev);

		// Write out files
		stringstream sstm;
		sstm << "r" << run << "_cspad2x2_fluores";
		string result = sstm.str();
  		ofstream outFlu(result.c_str()); // output fluorescence for all shots
  		for (unsigned i = 0; i < fluores.size(); i++ ) {
    		outFlu << fluores[i] << " ";
		}
  		outFlu << endl;
		outFlu.close();

		sstm.clear();
		sstm.str("");
		sstm << "r" << run << "_cspad2x2_photonEnergy";
		result = sstm.str();
		ofstream outEV(result.c_str()); // output photon energy for all shots
  		for (unsigned i = 0; i < photonEnergy.size(); i++ ) {
    		outEV << photonEnergy[i] << " ";
		}
  		outEV << endl;
		outEV.close();

		sstm.clear();
		sstm.str("");
		sstm << "r" << run << "_cspad2x2_pulseEnergy";
		result = sstm.str();
		ofstream outPulseEnergy(result.c_str()); // output pulse energy for all shots
		for (unsigned i = 0; i < pulseEnergy.size(); i++ ) {
    		outPulseEnergy << pulseEnergy[i] << " ";
		}
  		outPulseEnergy << endl;
		outPulseEnergy.close();

		sstm.clear();
		sstm.str("");
		sstm << "r" << run << "_cspad2x2_commonModeDark";
		result = sstm.str();
  		ofstream outCM(result.c_str()); // output fluorescence for all shots
		// Calculate mean common mode dark
		sum = accumulate(meanAsic3.begin(), meanAsic3.end(), 0.0);		
		mean = sum / meanAsic3.size();
  		for (unsigned i = 0; i < meanAsic3.size(); i++ ) {
    		outCM << meanAsic3[i] - mean << " ";
		}
  		outCM << endl;
		outCM.close();
	}

	if (sumSubsectionMinusDark) {
		// Save stats
		int run = 0;
  		PSTime::Time evtTime;
  		boost::shared_ptr<PSEvt::EventId> eventId = evt.get();
  		if (eventId.get()) {
			run = eventId->run();
			runNumber.push_back(run);
		}
		numImages.push_back(nonzeroCount);
		// Calculate mean and stdev fluorescence
		double sum = accumulate(fluores.begin(), fluores.end(), 0.0);		
		double mean = sum / fluores.size();

		double sq_sum = inner_product(fluores.begin(), fluores.end(), fluores.begin(), 0.0);
		double stdev = sqrt(sq_sum / fluores.size() - mean * mean);

		meanFluores.push_back(mean);
		stdFluores.push_back(stdev);

		// Write out files
		stringstream sstm;
		sstm << "r" << run << "_cspad2x2_fluores";
		string result = sstm.str();
  		ofstream outFlu(result.c_str()); // output fluorescence for all shots
  		for (unsigned i = 0; i < fluores.size(); i++ ) {
    		outFlu << fluores[i] << " ";
		}
  		outFlu << endl;
		outFlu.close();

		sstm.clear();
		sstm.str("");
		sstm << "r" << run << "_cspad2x2_photonEnergy";
		result = sstm.str();
		ofstream outEV(result.c_str()); // output photon energy for all shots
  		for (unsigned i = 0; i < photonEnergy.size(); i++ ) {
    		outEV << photonEnergy[i] << " ";
		}
  		outEV << endl;
		outEV.close();

		sstm.clear();
		sstm.str("");
		sstm << "r" << run << "_cspad2x2_pulseEnergy";
		result = sstm.str();
		ofstream outPulseEnergy(result.c_str()); // output pulse energy for all shots
		for (unsigned i = 0; i < pulseEnergy.size(); i++ ) {
    		outPulseEnergy << pulseEnergy[i] << " ";
		}
  		outPulseEnergy << endl;
		outPulseEnergy.close();
	}

	if (generateDark) {
		// Save stats
		int run = 0;
  		PSTime::Time evtTime;
  		boost::shared_ptr<PSEvt::EventId> eventId = evt.get();
  		if (eventId.get()) {
			run = eventId->run();
		}
		stringstream sstm;
		sstm << "dark" << run << "_cspad2x2";
		string result = sstm.str();
		ofstream outFile(result.c_str()); // modify myPsana1 to r117
		// Print average dark subregion CsPad2x2
		for (int j = sy; j < ey; j++) { // height
			for (int i = sx; i < ex; i++) { // width
				outFile << darkSubregion[i-sx][j-sy]/nonzeroCount << " ";
			}
			outFile << endl;
		}

		sstm.clear();
		sstm.str("");
		sstm << "dark" << run << "_cspad2x2_fluores";
		result = sstm.str();
		ofstream outFlu(result.c_str()); // modify myPsana1 to r117
		for (int i = 0; i < nonzeroCount; i++) {
			 outFlu << fluores[i] << " ";
		}
		outFlu << endl;
		outFlu.close();

		sstm.clear();
		sstm.str("");
		sstm << "dark" << run << "_cspad2x2_photonEnergy";
		result = sstm.str();
		ofstream outEnergy(result.c_str()); // modify myPsana1 to r117
		for (int i = 0; i < nonzeroCount; i++) {
			 outEnergy << photonEnergy[i] << " ";
		}
		outEnergy << endl;
		outEnergy.close();

		sstm.clear();
		sstm.str("");
		sstm << "dark" << run << "_cspad2x2_pulseEnergy";
		result = sstm.str();
		ofstream outPulseEnergy(result.c_str()); // output pulse energy for all shots
		for (unsigned i = 0; i < pulseEnergy.size(); i++ ) {
    		outPulseEnergy << pulseEnergy[i] << " ";
		}
  		outPulseEnergy << endl;
		outPulseEnergy.close();
	}
	cout << "done endRun" << endl;
}

/// Method which is called once at the end of the job
void chuck_ana_mod::endJob(Event& evt, Env& env)
{
	cout << "begin endJob" << endl;
	if (sumSubsectionMinusDark) {
		cout << "Writing out stats... " << endl;
		outStats << "Stats: " << endl;
		outStats << "runNumber	numImages	meanAvgPixValue	stdAvgPixValue" << endl;
		for (int i = 0; i < runCount; i++) {
			 outStats << setprecision(8) << runNumber[i] << "\t" << numImages[i] << "\t" << meanFluores[i] << "\t" << stdFluores[i] << endl;
		}
	}

	// Close files
	outStats.close();

	cout << "done endJob" << endl;
}

double chuck_ana_mod::getPulseEnergymJ(Event& evt, Env& env){
		// Gas detector values in mJ
		// get gasdet[4]
		double gmd1=0, gmd2=0;
		double gmd11=0, gmd12=0, gmd21=0, gmd22=0;

		shared_ptr<Psana::Bld::BldDataFEEGasDetEnergy> fee = evt.get(m_srcFee);
		if (fee.get()) {
			gmd11 = fee->f_11_ENRC();
			gmd12 = fee->f_12_ENRC();
			gmd21 = fee->f_21_ENRC();
			gmd22 = fee->f_22_ENRC();
			gmd1 = (gmd11+gmd12)/2;
			gmd2 = (gmd21+gmd22)/2;
			if (verbose) {
				cout << "*** gmd1 , gmd2: " << gmd1 << " , " << gmd2 << endl;  
			}
		} 
		return gmd2;
}

double chuck_ana_mod::getPhotonEnergyeV(Event& evt, Env& env){
		// get EBeam
		// EBeamV0 ~ EBeamV3
		// EBeamV2 not implemented yet
		float charge=0;
		float L3Energy=0;
		float LTUPosX=0; 
		float LTUPosY=0; 
		float LTUAngX=0; 
		float LTUAngY=0; 
		float PkCurrBC2=0;
		
		// Ebeam v0
		shared_ptr<Psana::Bld::BldDataEBeamV0> ebeam0 = evt.get(m_srcBeam);
		if (ebeam0.get()) {
			charge = ebeam0->ebeamCharge();
			L3Energy = ebeam0->ebeamL3Energy();
			LTUPosX = ebeam0->ebeamLTUPosX();
			LTUPosY = ebeam0->ebeamLTUPosY();
			LTUAngX = ebeam0->ebeamLTUAngX();
			LTUAngY = ebeam0->ebeamLTUAngY();
			if (verbose) {
			cout << "* fEbeamCharge0=" << charge << "\n"
					<< "* fEbeamL3Energy0=" << L3Energy << "\n"
					<< "* fEbeamLTUPosX0=" << LTUPosX << "\n"
					<< "* fEbeamLTUPosY0=" << LTUPosY << "\n"
					<< "* fEbeamLTUAngX0=" << LTUAngX << "\n"
					<< "* fEbeamLTUAngY0=" << LTUAngY << endl;
			}
		}

		// Ebeam v1
		shared_ptr<Psana::Bld::BldDataEBeamV1> ebeam1 = evt.get(m_srcBeam);
		if (ebeam1.get()) {
			charge = ebeam1->ebeamCharge();
			L3Energy = ebeam1->ebeamL3Energy();
			LTUPosX = ebeam1->ebeamLTUPosX();
			LTUPosY = ebeam1->ebeamLTUPosY();
			LTUAngX = ebeam1->ebeamLTUAngX();
			LTUAngY = ebeam1->ebeamLTUAngY();
			PkCurrBC2 = ebeam1->ebeamPkCurrBC2();
			if (verbose) {
			cout << "* fEbeamCharge1=" << charge << "\n"
					<< "* fEbeamL3Energy1=" << L3Energy << "\n"
					<< "* fEbeamLTUPosX1=" << LTUPosX << "\n"
					<< "* fEbeamLTUPosY1=" << LTUPosY << "\n"
					<< "* fEbeamLTUAngX1=" << LTUAngX << "\n"
					<< "* fEbeamLTUAngY1=" << LTUAngY << "\n"
					<< "* fEbeamPkCurrBC21=" << PkCurrBC2 << endl;
			}
		}

		// Ebeam v2
		shared_ptr<Psana::Bld::BldDataEBeamV2> ebeam2 = evt.get(m_srcBeam);
		if (ebeam2.get()) {
		charge = ebeam2->ebeamCharge();
		L3Energy = ebeam2->ebeamL3Energy();
		LTUPosX = ebeam2->ebeamLTUPosX();
		LTUPosY = ebeam2->ebeamLTUPosY();
		LTUAngX = ebeam2->ebeamLTUAngX();
		LTUAngY = ebeam2->ebeamLTUAngY();
		PkCurrBC2 = ebeam2->ebeamPkCurrBC2();
		if (verbose) {
			cout << "* fEbeamCharge2=" << charge << "\n"
					<< "* fEbeamL3Energy2=" << L3Energy << "\n"
					<< "* fEbeamLTUPosX2=" << LTUPosX << "\n"
					<< "* fEbeamLTUPosY2=" << LTUPosY << "\n"
					<< "* fEbeamLTUAngX2=" << LTUAngX << "\n"
					<< "* fEbeamLTUAngY2=" << LTUAngY << "\n"
					<< "* fEbeamPkCurrBC22=" << PkCurrBC2 << endl;
			}
		}

		// Ebeam v3
		shared_ptr<Psana::Bld::BldDataEBeamV3> ebeam3 = evt.get(m_srcBeam);
		if (ebeam3.get()) {
		charge = ebeam3->ebeamCharge();
		L3Energy = ebeam3->ebeamL3Energy();
		LTUPosX = ebeam3->ebeamLTUPosX();
		LTUPosY = ebeam3->ebeamLTUPosY();
		LTUAngX = ebeam3->ebeamLTUAngX();
		LTUAngY = ebeam3->ebeamLTUAngY();
		PkCurrBC2 = ebeam3->ebeamPkCurrBC2();
		if (verbose) {
			cout << "* fEbeamCharge3=" << charge << "\n"
					<< "* fEbeamL3Energy3=" << L3Energy << "\n"
					<< "* fEbeamLTUPosX3=" << LTUPosX << "\n"
					<< "* fEbeamLTUPosY3=" << LTUPosY << "\n"
					<< "* fEbeamLTUAngX3=" << LTUAngX << "\n"
					<< "* fEbeamLTUAngY3=" << LTUAngY << "\n"
					<< "* fEbeamPkCurrBC23=" << PkCurrBC2 << endl;
			}
		}

		// Calculate photon energy and wavelength
		double photonEnergyeV=0;
		double wavelengthA=0;
		double peakCurrent = 0;
		double DL2energyGeV = 0;
		if (ebeam0.get()) {
			cout << "***** WARNING *****" << endl;
			cout << "EBeamV0 does not record peak current" << endl;
			cout << "Setting peak current to 0" << endl;
			peakCurrent = 0;
			DL2energyGeV = 0.001*ebeam0->ebeamL3Energy();
		} else if (ebeam1.get()) {
			// Get the present peak current in Amps
			// Get present beam energy [GeV]
			peakCurrent = ebeam1->ebeamPkCurrBC2();
			DL2energyGeV = 0.001*ebeam1->ebeamL3Energy();
		} else if (ebeam2.get()) {
			peakCurrent = ebeam2->ebeamPkCurrBC2();
			DL2energyGeV = 0.001*ebeam2->ebeamL3Energy();
		} else if (ebeam3.get()) {
			peakCurrent = ebeam3->ebeamPkCurrBC2();
			DL2energyGeV = 0.001*ebeam3->ebeamL3Energy();
		}
		
		// get wavelengthA
		// Calculate the resonant photon energy (ie: photon wavelength) 
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
		photonEnergyeV = 44.42*energyProfile*energyProfile;
		// Calculate wavelength in Angstrom
		wavelengthA = 12398.42/photonEnergyeV;
		if (verbose) {
			cout << "peakCurrent: " << peakCurrent << endl;
			cout << "DL2energyGeV: " << DL2energyGeV << endl;
			cout << "photonEnergyeV: " << photonEnergyeV << endl;
			cout << "***** wavelengthA: " << wavelengthA << endl;
		}
	return photonEnergyeV;
}

} // namespace chuck_ana_pkg
