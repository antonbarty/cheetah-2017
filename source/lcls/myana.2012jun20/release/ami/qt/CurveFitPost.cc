#include "ami/qt/CurveFitPost.hh"

#include "ami/qt/AxisInfo.hh"
#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/Filter.hh"

#include "ami/data/CurveFit.hh"
#include "ami/data/Cds.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/DescCache.hh"

#include <QtCore/QString>

using namespace Ami::Qt;

CurveFitPost::CurveFitPost(unsigned channel, Ami::CurveFit* input) :
  _channel (channel),
  _input   (input)
{
}

CurveFitPost::CurveFitPost(const char*& p)
{
  load(p);
}

CurveFitPost::~CurveFitPost()
{
  delete _input;
}

void CurveFitPost::save(char*& p) const
{
  char* buff = new char[8*1024];
  XML_insert(p, "int"        , "_channel", QtPersistent::insert(p,(int)_channel) );
  XML_insert(p, "QString"    , "_name"   , QtPersistent::insert(p, QString(_input->name())));
  XML_insert(p, "QString"    , "_norm"   , QtPersistent::insert(p, QString(_input->norm())));
  XML_insert(p, "int"        , "_op"     , QtPersistent::insert(p, (int)_input->op()));
  XML_insert(p, "QString"    , "_title"  , QtPersistent::insert(p, QString(_input->output().name())));
  delete[] buff;
}

void CurveFitPost::load(const char*& p)
{
  QString name;
  QString norm;
  int op = 0;
  QString title;
  name.clear();
  norm.clear();
  title.clear();
  XML_iterate_open(p,tag)
    if (tag.name == "_channel")
      _channel = QtPersistent::extract_i(p);
    else if (tag.name == "_name")
      name = QtPersistent::extract_s(p);
    else if (tag.name == "_norm")
      norm = QtPersistent::extract_s(p);
    else if (tag.name == "_op")
      op = QtPersistent::extract_i(p);
    else if (tag.name == "_title")
      title = QtPersistent::extract_s(p);
  XML_iterate_close(CurveFitPost,tag);

  const char *_title = qPrintable(title);
  Ami::DescCache* desc = new Ami::DescCache(_title, _title, Ami::PostAnalysis);
  _input = new Ami::CurveFit(qPrintable(name), op, *desc, qPrintable(norm));
  delete desc;
}

void CurveFitPost::configure(char*& p, unsigned input, unsigned& output,
                             ChannelDefinition* channels[], int* signatures, unsigned nchannels,
                             const AxisInfo& xinfo, ConfigureRequest::Source source)
{
  unsigned channel = _channel;
  unsigned input_signature = signatures[channel];

  ConfigureRequest& r = *new (p) ConfigureRequest(ConfigureRequest::Create,
						  source,
						  input_signature,
						  _output_signature = ++output,
						  *channels[channel]->filter().filter(),
						  *_input);
  p += r.size();
}
