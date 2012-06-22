#include "ami/qt/DetectorList.hh"
#include "ami/qt/DetectorSelect.hh"

#include <QtCore/QStringList>

using namespace Ami::Qt;

DetectorList::DetectorList(DetectorSelect* parent, const QStringList& names, const Pds::DetInfo& info, unsigned n) :
  _parent(parent),
  _info(info)
{
  // populate the box
  for(unsigned i=0; i<n; i++)
    addItem(names[i]);
  connect(this, SIGNAL(activated(int)), this, SLOT(start_detector(int)));
}

DetectorList::~DetectorList()
{
}

void DetectorList::start_detector(int n)
{
  _parent->start_detector(_info,n);
}
