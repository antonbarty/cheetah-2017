#ifndef AmiQt_DetectorListItem_hh
#define AmiQt_DetectorListItem_hh

#include <QtGui/QListWidgetItem>

#include <pdsdata/xtc/DetInfo.hh>

class QListWidget;

namespace Ami {
  namespace Qt {
    class DetectorListItem : public QListWidgetItem {
    public:
      DetectorListItem(QListWidget*        parent,
		       const QString&      dlabel,
		       const Pds::DetInfo& dinfo, 
		       unsigned            dchannel);
      ~DetectorListItem();
    public:
      Pds::DetInfo    info;
      unsigned        channel;
    };
  };
};

#endif
