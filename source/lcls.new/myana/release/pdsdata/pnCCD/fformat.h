/*************************************************************************

 Copyright (C) 2007-2009 by Peter Holl and Nils Kimmel.
 All rights reserved.
 
 This code implementation is the intellectual property of

 Peter Holl    <pxh@hll.mpg.de> , MPI semiconductor laboratory,
                                  PNSensor GmbH

 Nils Kimmel   <nik@hll.mpg.de> , MPI semiconductor laboratory,
                                  Max-Planck-Institut fuer extra-
                                  terrestrische Physik

 By copying, distributing or modifying the Program (or any work
 based on the Program) you indicate your acceptance of this statement,
 and all its terms.

*************************************************************************/


#ifndef FFORMAT_H
#define FFORMAT_H


#define VERSION		6	// format version
				// 6:	since Jul-20-2005
				//	extended headersizes to add additional
				//	information after Version5 header:
				//	unsigned short for width, maxHeight, start, height

				/* 5:	since Jun-10-94, 17:00 */
				/*	frameheader contains 8 byte data */
				/*	currently interpreted as */
				/*	temperature voltage measurement */

				/* 4:	since Mar-11-94, 16:00 */
				/*	for timing and window modes */
				/*	info byte instead of info word, */
				/*	starting line stored in first byte */

				/* 3:	since Jan-20-94, 12:00 */
				/*	dataSetID appended*/

				/* 2:	an index number has been added to */
				/*	the frame header */

#define INFO_CALIB	0x01	/* flag set, if calibration frame */
#define INFO_PACKED	0x02	/* flag set, if pixels are packed */
#define INFO_INCOMPLETE	0x04	/* flag set, if frame isn't complete */
#define INFO_INCMP_LINE	0x08	/* flag set, if last line stored isn't complete */
#define INFO_DEFA_TIME	0x10	/* flag set, if tv_sec is set by DEFA clock */

/* just for laziness */
typedef short		pxType;

// structure type for file header, contains static information
typedef	struct
	{
	unsigned short	myLength;	/* number of bytes in file header */
	unsigned short	fhLength;	/* number of bytes in frame header */
	unsigned char	nCCDs;		/* number of CCD to be read out */
	unsigned char	width;		/* number of channels */
	unsigned char	maxHeight;	/* (maximum) number of lines */
	unsigned char	version;	/* format version */
		 char	dataSetID[80];	/* identification of data */

// following lines added for Version 6
	unsigned short	the_width;	// the true number of channels
	unsigned short	the_maxHeight;	// the true (maximum) number of lines
		 char	fill[932];	// reserve space
	} fileHeaderType;	// size should be 1024 bytes

/* structure type for frame header, contains static information for each frame */
typedef	struct
	{
	unsigned char	start;		/* starting line, indicates window mode if not */
	unsigned char	info;		/* info byte */
	unsigned char	id;		/* CCD id */
	unsigned char	height;		/* number of lines in following frame */
	unsigned int	tv_sec;		/* start data taking time in seconds */		// PxH from long to int, 17-Aug-07
	unsigned int	tv_usec;	/* start data taking time in microseconds */	// PxH from long to int, 17-Aug-07
	unsigned int	index;		/* index number */				// PxH from long to int, 17-Aug-07
	double	temp;		/* temperature voltage */

// following lines added for Version 6
	unsigned short	the_start;	// the true starting line, indicates window mode if > 0
	unsigned short	the_height;	// the true number of lines in following frame
        unsigned int    external_id;    // Frame ID from an external trigger, e.g. the bunch ID at DESY/FLASH
        double          aux_value;      // External data value, e.g. the bunch/pulse energy at DESY/FLASH

        char            fill[24];       // Dummy bytes to fill the frame header up to a size of 64 bytes
	} frameHeaderType;	        // size should be 64 bytes */

// following important for shared memory operations
typedef struct
	{
	frameHeaderType		frH;
	pxType			*px;	// size is calculated dynamically
//	pxType			px[16384]; // 256 x 64 to be on the save side
	} shmBfrType;

/* following is defined but not really used */
typedef struct
	{
	unsigned	word1 : 12;
	unsigned	word2 : 12;
	unsigned	word3 : 12;
	unsigned	word4 : 12;
	} packedDataType;


/* function prototypes */
shmBfrType	*storeShm(shmBfrType *);

#endif // FFORMAT_H

