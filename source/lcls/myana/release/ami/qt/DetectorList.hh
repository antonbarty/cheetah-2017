#ifndef AmiQt_DetectorList_hh
#define AmiQt_DetectorList_hh

#include <QtGui/QComboBox>

#include <pdsdata/xtc/DetInfo.hh>

class QStringList;

namespace Ami {
  namespace Qt {
    class DetectorSelect;
    class DetectorList : public QComboBox {
      Q_OBJECT
    public:
      DetectorList(DetectorSelect* parent, const QStringList&, const Pds::DetInfo&, unsigned n);
      ~DetectorList();
    public slots:
      void start_detector(int);
    private:
      DetectorSelect* _parent;
      Pds::DetInfo    _info;
    };
  };
};

#endif
