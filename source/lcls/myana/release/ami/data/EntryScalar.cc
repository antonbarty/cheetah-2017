#include "ami/data/EntryScalar.hh"

using namespace Ami;

EntryScalar::~EntryScalar() {}

EntryScalar::EntryScalar(const Pds::DetInfo& info, unsigned channel,
			 const char* name, 
			 const char* ytitle) :
  _desc(info, channel, name, ytitle),
  _y   (static_cast<double*>(allocate(sizeof(double)*3)))
{
}

EntryScalar::EntryScalar(const DescScalar& desc) :
  _desc(desc),
  _y   (static_cast<double*>(allocate(sizeof(double)*3)))
{
}

const DescScalar& EntryScalar::desc() const {return _desc;}
DescScalar& EntryScalar::desc() {return _desc;}

void EntryScalar::setto(const EntryScalar& entry) 
{
  _y[0] = entry._y[0];
  _y[1] = entry._y[1];
  _y[2] = entry._y[2];
}

void EntryScalar::setto(const EntryScalar& curr, 
			const EntryScalar& prev) 
{
  _y[0] = curr._y[0] - prev._y[0];
  _y[1] = curr._y[1] - prev._y[1];
  _y[2] = curr._y[2] - prev._y[2];
}

void EntryScalar::add(const EntryScalar& entry) 
{
  _y[0] += entry._y[0];
  _y[1] += entry._y[1];
  _y[2] += entry._y[2];
}

