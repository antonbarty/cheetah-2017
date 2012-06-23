#ifndef AmiQt_DetectorButton_hh
#define AmiQt_DetectorButton_hh

#include <QtGui/QPushButton>

#include <pdsdata/xtc/DetInfo.hh>

namespace Ami {
  namespace Qt {
    class DetectorSelect;
    class DetectorButton : public QPushButton {
      Q_OBJECT
    public:
      DetectorButton(DetectorSelect* parent, const char* name, const Pds::DetInfo&);
      ~DetectorButton();
    public slots:
      void start_detector();
    private:
      DetectorSelect* _parent;
      Pds::DetInfo    _info;
    };
  };
};

#endif
