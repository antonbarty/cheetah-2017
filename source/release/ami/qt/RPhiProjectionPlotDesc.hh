#ifndef AmiQt_RPhiProjectionPlotDesc_hh
#define AmiQt_RPhiProjectionPlotDesc_hh

#include <QtGui/QWidget>

class QButtonGroup;

namespace Ami {
  class DescEntry;
  class RPhiProjection;
  namespace Qt {
    class AnnulusCursors;

    class RPhiProjectionPlotDesc : public QWidget {
    public:
      RPhiProjectionPlotDesc(QWidget* parent,
			     const AnnulusCursors& r);
      ~RPhiProjectionPlotDesc();
    public:
      void save(char*& p) const;
      void load(const char*& p);
    public:
      Ami::RPhiProjection* desc(const char*) const;
    private:
      const AnnulusCursors& _annulus;
      QButtonGroup* _axis;
      QButtonGroup* _norm;
    };
  };
};

#endif
