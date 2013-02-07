#ifndef AmiQt_PlotFactory_hh
#define AmiQt_PlotFactory_hh

class QColor;
class QString;

namespace Ami {
  class AbsTransform;
  class Entry;
  namespace Qt {
    class QtBase;
    class PlotFactory {
    public:
      static QtBase* plot(const QString& title,
			  const Entry&, 
			  const AbsTransform& x, 
			  const AbsTransform& y,
			  const QColor&);
    };
  };
};

#endif
