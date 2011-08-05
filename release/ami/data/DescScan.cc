#include "ami/data/DescScan.hh"

#include <string.h>

using namespace Ami;

#include <stdio.h>


DescScan::DescScan(const char* name, 
		   const char* xtitle, 
		   const char* ytitle, 
		   unsigned nbins,
		   const char* weight) :
  DescEntryW( name, xtitle, ytitle, weight, Scan, sizeof(DescScan)),
  _nbins(nbins)
{
}

DescScan::DescScan(const Pds::DetInfo& info,
 		   unsigned channel,
 		   const char* name, 
 		   const char* xtitle, 
 		   const char* ytitle, 
 		   unsigned nbins) :
  DescEntryW(info, channel, name, xtitle, ytitle, "", Scan, sizeof(DescScan)),
  _nbins(nbins)
{
}

void DescScan::params(unsigned nbins)
{
  _nbins = nbins;
}
