#include "PlotFactory.hh"

#include "ami/data/AbsTransform.hh"
// #include "ami/qt/QtScalar.hh"
#include "ami/qt/QtTH1F.hh"
#include "ami/qt/QtProf.hh"
#include "ami/qt/QtTH2F.hh"
#include "ami/qt/QtImage.hh"
#include "ami/qt/QtChart.hh"
#include "ami/qt/QtWaveform.hh"
#include "ami/data/EntryScalar.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryTH2F.hh"
#include "ami/data/EntryProf.hh"
#include "ami/data/EntryWaveform.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/DescEntry.hh"

#include <stdio.h>

using namespace Ami::Qt;

QtBase* PlotFactory::plot(const QString&    name,
			  const Ami::Entry& entry,
			  const AbsTransform&  x,
			  const AbsTransform&  y,
			  const QColor&     c)
{
#define QTCASE(type) \
  case Ami::DescEntry::type : \
    { b = new Qt##type(name,static_cast<const Ami::Entry##type&>(entry),x,y,c); \
      break; }

  Ami::Qt::QtBase* b;
  switch(entry.desc().type()) {
    //    QTCASE(Scalar);
    QTCASE(TH1F);
    QTCASE(Prof);
    QTCASE(TH2F);
    QTCASE(Waveform);
    QTCASE(Image);
  default : 
    printf("PlotFactory type %d not implemented\n",entry.desc().type()); 
    b = 0; 
    break;
  }
#undef QTCASE
  return b;
}

