/*
 *  data2d.h
 *  wombat3d
 *
 *  Created by Anton Barty on 4/9/10.
 *  Copyright 2010 all rights reserved.
 *
 */

#ifndef DATA2D_INCLUDED
#define DATA2D_INCLUDED

extern const char* const ATTR_NAME_DETECTOR_NAME ;
extern const char* const ATTR_NAME_DETECTOR_ID ;

#define	tData2d	float

class cData2d {
	
public:
	cData2d();
	cData2d(long);
	~cData2d();
	
	void create(long);
	void create(long, long);
	void readHDF5(char *);
	void readHDF5(char *, char*);
	void writeHDF5(char *);
    
	
public:
	long		nx,ny,nn;
	tData2d		*data;
	char        detectorName[1024];
	long        detectorID;
	
private:
	
	
};


#endif
