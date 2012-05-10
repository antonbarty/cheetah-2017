#include "ami/qt/EnvPost.hh"

#include "ami/data/EnvPlot.hh"
#include "ami/data/Cds.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/AbsFilter.hh"
#include "ami/data/FilterFactory.hh"
#include "ami/data/DescCache.hh"
#include "ami/qt/QtPersistent.hh"

using namespace Ami::Qt;

EnvPost::EnvPost(const Ami::AbsFilter& filter,
                 DescCache*       desc,
                 Ami::ScalarSet   set) :
  _filter  (filter.clone()),
  _desc    (desc),
  _set     (set),
  _output_signature(0)
{
}

EnvPost::EnvPost(const char*& p)
{
  XML_iterate_open(p,tag)
    if (tag.name == "_filter") {
      Ami::FilterFactory factory;
      const char* b = (const char*)QtPersistent::extract_op(p);
      _filter = factory.deserialize(b);
    }
    else if (tag.name == "_desc") {
      _desc = new DescCache(*(DescCache*)QtPersistent::extract_op(p));
    }
    else if (tag.name == "_set") {
      _set = Ami::ScalarSet(QtPersistent::extract_i(p));
    }
  XML_iterate_close(EnvPost,tag);
}

EnvPost::~EnvPost()
{
  delete _desc;
}

void EnvPost::save(char*& p) const
{
  char* buff = new char[8*1024];
  XML_insert( p, "AbsFilter", "_filter", QtPersistent::insert(p, buff, (char*)_filter->serialize(buff)-buff) );
  XML_insert( p, "DescCache", "_desc", QtPersistent::insert(p, _desc, _desc->size()) );
  XML_insert( p, "ScalarSet", "_set" , QtPersistent::insert(p, int(_set)) );
  delete[] buff;
}

void EnvPost::load(const char*& p)
{
}

void EnvPost::configure(char*& p, unsigned input, unsigned& output)
{
  Ami::EnvPlot op(*_desc);

  ConfigureRequest& r = *new (p) ConfigureRequest(ConfigureRequest::Create,
						  ConfigureRequest::Discovery,
						  input,
						  _output_signature = ++output,
						  *_filter, op, _set);
  p += r.size();
}

