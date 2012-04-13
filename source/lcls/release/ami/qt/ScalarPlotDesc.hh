#ifndef AmiQt_ScalarPlotDesc_hh
#define AmiQt_ScalarPlotDesc_hh

#include "ami/qt/FeatureList.hh"

#include <QtGui/QWidget>

class QButtonGroup;
class QCheckBox;
class QString;

namespace Ami {
  class DescEntry;
  namespace Qt {
    class DescTH1F;
    class DescChart;
    class DescProf;
    class DescScan;

    class ScalarPlotDesc : public QWidget {
    public:
      enum Type { TH1F, vT, vF, vS };
      ScalarPlotDesc(QWidget* parent);
      ~ScalarPlotDesc();
    public:
      void save(char*& p) const;
      void load(const char*& p);
    public:
      DescEntry*  desc(const char*) const;
      const char* expr(const QString& e) const;
    protected:
      QCheckBox*    _xnorm;
      QCheckBox*    _ynorm;
      FeatureList*   _vnorm;

      QCheckBox*    _weightB;
      FeatureList*  _vweight;

      QButtonGroup* _plot_grp;
      DescTH1F*   _hist;
      DescChart*  _vTime;
      DescProf*   _vFeature;
      DescScan*   _vScan;
    };
  };
};

#endif
