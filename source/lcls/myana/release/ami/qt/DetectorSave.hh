#ifndef AmiQt_DetectorSave_hh
#define AmiQt_DetectorSave_hh

#include "ami/qt/DetectorGroup.hh"

namespace Ami {
  namespace Qt {
    class DetectorSave : public DetectorGroup {
    public:
      DetectorSave(QWidget* parent,
		   const std::list<QtTopWidget*>&);
      DetectorSave(const DetectorSave&);
      ~DetectorSave();
    private:
      void _init ();
      void _apply(QtTopWidget&);
    private:
      QString _prefix;
    };
  };
};

#endif
