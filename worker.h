/*
 *  worker.h
 *  cspad_cryst
 *
 *  Created by Anton Barty on 6/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */




/*
 *	Global variables
 */
typedef struct {

		
	// Run information
	unsigned	runNumber;

	
	// Thread management
	long			nThreads;
	long			nActiveThreads;
	pthread_t		*threadID;
	pthread_mutex_t	nActiveThreads_mutex;


	// Detector geometry
	long			pix_nx;
	long			pix_ny;
	long			pix_nn;
	float			*pix_x;
	float			*pix_y;
	float			*pix_z;
	float			pix_dx;
	unsigned		module_rows;
	unsigned		module_cols;
	long			image_nx;
	long			image_nn;
	
} tGlobal;



/*
 *	Structure used for passing information to worker threads
 */
typedef struct {
	
	// Reference to common global structure
	tGlobal		*pGlobal;
	int			busy;
	
	// cspad data
	int			cspad_fail;
	float		quad_temperature[4];
	uint16_t	*quad_data[4];
	uint16_t	*raw_data;
	uint16_t	*image;
	
	
	// Beamline data, etc
	int			seconds;
	int			nanoSeconds;
	unsigned	fiducial;
	char		timeString[1024];
	bool		beamOn;
	unsigned	runNumber;
	
	double		gmd11;
	double		gmd12;
	double		gmd21;
	double		gmd22;
	
	double		fEbeamCharge;		// in nC
	double		fEbeamL3Energy;		// in MeV
	double		fEbeamLTUPosX;		// in mm
	double		fEbeamLTUPosY;		// in mm
	double		fEbeamLTUAngX;		// in mrad
	double		fEbeamLTUAngY;		// in mrad
	double		fEbeamPkCurrBC2;	// in Amps
	
	double		photonEnergyeV;		// in eV
	double		wavelengthA;		// in Angstrom
	
	double		phaseCavityTime1;
	double		phaseCavityTime2;
	double		phaseCavityCharge1;
	double		phaseCavityCharge2;

	
	
	// Thread management
	int		threadID;

	char	filename[1024];
	
	// Memory protection
	pthread_mutex_t	*nActiveThreads_mutex;
	
	
} tThreadInfo;



/*
 *	Stuff from Garth's original code
 */

// Static variables
using namespace std;
static CspadCorrector*      corrector;
static Pds::CsPad::ConfigV1 configV1;
static Pds::CsPad::ConfigV2 configV2;
static unsigned             configVsn;
static unsigned             quadMask;
static unsigned             asicMask;

static const unsigned  ROWS = 194;
static const unsigned  COLS = 185;

static uint32_t nevents = 0;

#define ERROR(...) fprintf(stderr, __VA_ARGS__)
#define STATUS(...) fprintf(stderr, __VA_ARGS__)



/*
 *	Function prototypes
 */
void *worker(void *);
void writeHDF5(tThreadInfo*, tGlobal*);
void writeSimpleHDF5(const char*, const void*, int, int, int);
void globalConfiguration(tGlobal*);
void setupThreads(tGlobal*);
void readDetectorGeometry(tGlobal*);
void assemble2Dimage(tThreadInfo*, tGlobal*);



