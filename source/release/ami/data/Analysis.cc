#include "Analysis.hh"

#include "ami/data/Cds.hh"
#include "ami/data/AbsOperator.hh"
#include "ami/data/AbsFilter.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/Entry.hh"

#include "ami/data/FilterFactory.hh"
#include "ami/data/OperatorFactory.hh"

using namespace Ami;

Analysis::Analysis(unsigned      id, 
		   const Entry&  input, 
		   unsigned      output, 
		   Cds&          cds, 
		   FeatureCache& cache,
		   const char*&  p) :
  _id    (id),
  _input (input),
  _cds   (cds)
{
  FilterFactory filters(cache);
  _filter = filters.deserialize(p);

  OperatorFactory operators(cache);
  _op = operators.deserialize(p, input, cds, output);
  Entry& output_entry = (*_op)(input);
  output_entry.invalid();
  _cds.add(&output_entry,output);
}

Analysis::~Analysis() 
{
  _cds.remove(_cds.entry(output().signature()));
}

unsigned   Analysis::id() const { return _id; }

void   Analysis::analyze()
{
  if (_input.valid() && _filter->accept())
    (*_op)(_input);
}

DescEntry& Analysis::output () const
{
  return _op->output();
}
