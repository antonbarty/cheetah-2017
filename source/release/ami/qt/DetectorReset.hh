#ifndef AmiQt_DetectorReset_hh
#define AmiQt_DetectorReset_hh

#include "ami/qt/DetectorGroup.hh"

namespace Ami {
  namespace Qt {
    class DetectorReset : public DetectorGroup {
    public:
      DetectorReset(QWidget*,
		    const std::list<QtTopWidget*>&);
      DetectorReset(const DetectorReset& clone);
      ~DetectorReset();
    private:
      void _apply(QtTopWidget&);
    };
  };
};

#endif
