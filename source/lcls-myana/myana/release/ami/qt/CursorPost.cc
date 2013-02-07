#include "ami/qt/CursorPost.hh"

#include "ami/qt/AxisInfo.hh"
#include "ami/qt/ChannelDefinition.hh"
#include "ami/qt/Filter.hh"

#include "ami/data/BinMath.hh"
#include "ami/data/Cds.hh"
#include "ami/data/DescEntry.hh"

#include <QtCore/QString>

using namespace Ami::Qt;

CursorPost::CursorPost(unsigned         channel,
		       BinMath*         input) :
  _channel (channel),
  _input   (input)
{
}

CursorPost::CursorPost(const char*& p) : 
  _channel(0),
  _input  (0)
{
  load(p);
}

CursorPost::~CursorPost()
{
  if (_input) delete _input;
}

void CursorPost::save(char*& p) const
{
  char* buff = new char[8*1024];
  XML_insert(p, "int"    , "_channel", QtPersistent::insert(p,(int)_channel) );
  XML_insert(p, "BinMath", "_input"  , QtPersistent::insert(p,buff,(char*)_input->serialize(buff)-buff) );
  delete[] buff;
}

void CursorPost::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if (tag.name == "_channel")
      _channel = QtPersistent::extract_i(p);
    else if (tag.name == "_input") {
      if (_input) delete _input;
      const char* b = (const char*)QtPersistent::extract_op(p);
      b += 2*sizeof(uint32_t);
      _input = new BinMath(b);
    }
  XML_iterate_close(CursorPost,tag);
}

void CursorPost::configure(char*& p, unsigned input, unsigned& output,
			   ChannelDefinition* channels[], int* signatures, unsigned nchannels,
			   const AxisInfo& xinfo, ConfigureRequest::Source source)
{
  unsigned channel = _channel;
  unsigned input_signature = signatures[channel];

  // replace cursor values with bin indices
  QString expr(_input->expression());
  QString new_expr;
  { QRegExp match("\\[[^\\]]*\\]");
    int last=0;
    int pos=0;
    while( (pos=match.indexIn(expr,pos)) != -1) {
      QString use = expr.mid(pos+1,match.matchedLength()-2);
      bool ok;
      double v = use.toDouble(&ok);
      unsigned bin=0;
      if (!ok)
	printf("error parsing double %s\n",qPrintable(use));
      else {
	bin = xinfo.tick(v);
      }
      new_expr.append(expr.mid(last,pos-last));
      new_expr.append(QString("[%1]").arg(bin));
      pos += match.matchedLength();
      last = pos;
    }
    new_expr.append(expr.mid(last));
    new_expr.replace(QString("]%1[").arg(BinMath::integrate()),QString(BinMath::integrate()));
    new_expr.replace(QString("]%1[").arg(BinMath::range    ()),QString(BinMath::range    ()));
  }
  QString end_expr;
  { int last=0, next=0, pos=0;
    while( (pos=new_expr.indexOf(BinMath::range(),pos)) != -1) {
      if ( (next=new_expr.lastIndexOf("[",pos))==-1 )
	printf("error parsing range in %s\n",qPrintable(expr));
      else {
	end_expr.append(new_expr.mid(last,next-last));
	last  = new_expr.indexOf("]",pos);
	int a = new_expr.mid(next+1,pos -next-1).toInt();
	int b = new_expr.mid(pos +1,last-pos -1).toInt();
	printf("%s/%d %s/%d\n",
	       qPrintable(new_expr.mid(next+1,pos -next-1)),a,
	       qPrintable(new_expr.mid(pos +1,last-pos -1)),b);
	end_expr.append(QString("(%1)").arg(QString::number(abs(a-b)+1)));
	pos  = ++last;
      }
    }
    end_expr.append(new_expr.mid(last));
  }

  Ami::BinMath op(_input->output(), qPrintable(end_expr));
  
  ConfigureRequest& r = *new (p) ConfigureRequest(ConfigureRequest::Create,
						  source,
						  input_signature,
						  _output_signature = ++output,
						  *channels[channel]->filter().filter(),
						  op);
  p += r.size();
}

