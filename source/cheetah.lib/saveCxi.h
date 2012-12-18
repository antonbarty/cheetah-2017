#ifndef SAVECXI_H
#define SAVECXI_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>


#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "cheetahmodules.h"
#include "median.h"
#include "staticPara.h"

class CXI{
    public:
        void writeCxi(cEventData *, cGlobal * );
    private:
        void saveArrayIntoDataset3( hid_t& , int , int16_t *  , hsize_t * );
        template <class T> void saveEleIntoDataset1(hid_t & , int , T );
        void extendAllDataset(int , cGlobal* );
        void extend(hid_t & , hsize_t* );

};


#endif // SAVECXI_H
