#include "ami/data/DescScan.hh"

#include <string.h>

using namespace Ami;

#include <stdio.h>


DescScan::DescScan(const char* name, 
		   const char* xtitle, 
		   const char* ytitle, 
		   unsigned nbins,
		   const char* weight,
                   bool scatter) :
  DescEntryW( name, xtitle, ytitle, weight, Scan, sizeof(DescScan)),
  _nbins  (nbins),
  _details(scatter ? 1:0)
{
}

DescScan::DescScan(const Pds::DetInfo& info,
 		   unsigned channel,
 		   const char* name, 
 		   const char* xtitle, 
 		   const char* ytitle, 
 		   unsigned nbins,
                   bool scatter) :
  DescEntryW(info, channel, name, xtitle, ytitle, "", Scan, sizeof(DescScan)),
  _nbins(nbins),
  _details(scatter ? 1:0)
{
}

void DescScan::params(unsigned nbins)
{
  _nbins = nbins;
}
