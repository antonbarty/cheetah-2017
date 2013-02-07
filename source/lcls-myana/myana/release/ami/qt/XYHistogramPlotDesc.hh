#ifndef AmiQt_XYHistogramPlotDesc_hh
#define AmiQt_XYHistogramPlotDesc_hh

#include <QtGui/QWidget>

class QButtonGroup;

namespace Ami {
  class XYHistogram;
  namespace Qt {
    class DescTH1F;
    class RectangleCursors;

    class XYHistogramPlotDesc : public QWidget {
    public:
      XYHistogramPlotDesc(QWidget* parent,
                          const RectangleCursors& r);
      ~XYHistogramPlotDesc();
    public:
      void save(char*& p) const;
      void load(const char*& p);
    public:
      Ami::XYHistogram* desc(const char*) const;
    private:
      const RectangleCursors& _rectangle;
      DescTH1F* _desc;
    };
  };
};

#endif
