/*
 * runPtycho.cc:
 *
 * Main program for performing ptychography
 *
 * Copyright © 2012 European XFEL GmbH
 * Copyright © 2012 Deutsches Elektronen-Synchrotron DESY,
 *                  a research centre of the Helmholtz Association.
 *
 * Author list:
 * Chun Hong Yoon	chun.hong.yoon@desy.de		2012
 * Klaus Giewekemeyer	klaus.giewekemeyer@xfel.eu	2012
 *
 * This file is part of Ptychology.
 *
 * Ptychology is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Ptychology is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Ptychology.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "libptycho/data2d.h"
#include "libptycho/Complex_2D.h"
#include "libptycho/Double_2D.h"
#include "libptycho/io.h"

using namespace std;

int main()
{
	//const static char * data_file_name = "./images/myImg.tiff";
    	const static char * data_file_name = "./images/test00001_00000.tif";
	//get the data from file
	Double_2D data;

	//read the data into an array
	int status = read_tiff(data_file_name, data);
	if(!status){
		cout << "failed.. exiting"  << endl;
	    return(1);
	}

	const static char * temp_name = "./images/temp.tif";
	write_tiff(temp_name, data);

	for(int i = 0; i < 1000; i++){
		for(int j = 0; j < 1000; j++){
			if (data.get(i,j) != 0){
			cout << i << "," << j << ": " << data.get(i,j) << endl;
			}
		}
	}
    return 0;
}
