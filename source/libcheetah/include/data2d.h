/*
 *  data2d.h
 *  wombat3d
 *
 *  Created by Anton Barty on 4/9/10.
 *  Copyright 2010 all rights reserved.
 *
 */

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
	
	
private:
	
	
};


