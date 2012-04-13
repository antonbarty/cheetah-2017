#include "DetectorListItem.hh"

using namespace Ami::Qt;

DetectorListItem::DetectorListItem(QListWidget*        parent,
				   const QString&      dlabel,
				   const Pds::DetInfo& dinfo, 
				   unsigned            dchannel) :
  QListWidgetItem(dlabel, parent),
  info           (dinfo),
  channel        (dchannel) 
{
  setTextAlignment(::Qt::AlignHCenter);
}

DetectorListItem::~DetectorListItem() 
{
}
