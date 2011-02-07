/*
 *  worker.h
 *  cspad_cryst
 *
 *  Created by Anton Barty on 6/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */



/*
 *	Stuff from Garth's sample code
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




/*
 *	Structure used for passing information to worker threads
 */
typedef struct {
	
	// cspad data

	
	
	// Beamline data, etc
	int			seconds;
	int			nanoSeconds;
	unsigned	fiducial;
	char		timeString[1024];
	bool		beamOn;
	
	double		gmd11;
	double		gmd12;
	double		gmd21;
	double		gmd22;
	
	double		ft1;
	double		ft2;
	double		c1;
	double		c2;

	
	
	// Thread management
	int		threadID;
	char	filename[1024];
	
	// Memory protection
	pthread_mutex_t	*newModel_mutex;
	pthread_mutex_t	*newModelCounter_mutex;
	
	
} tThreadInfo;


static int hdf5_write(const char*, const void*, int, int, int);
