#ifndef CREATEHID_T_H
#define CREATEHID_T_H
#include <hdf5.h>
#include <stdlib.h>
#include "staticPara.h"
#include "cheetahGlobal.h"
#include "detectorObject.h"
#include "cheetah.h"

void create (cGlobal * , Cxi_para * );
void create (cGlobal *);
void close ();
//hid_t create_group(char* , hid_t , hid_t );
//hid_t create_string_dataset(char * , hid_t );
//hid_t create_dataset3(char *, hid_t , hsize_t dims3[3], hsize_t maxdims3[3], hid_t );
#endif // CREATEHID_T_H
