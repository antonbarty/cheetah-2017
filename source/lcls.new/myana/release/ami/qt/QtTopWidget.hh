#ifndef AmiQt_QtTopWidget_hh
#define AmiQt_QtTopWidget_hh

#include "QtPWidget.hh"

#include "pdsdata/xtc/DetInfo.hh"

namespace Ami {
  namespace Qt {
    class QtTopWidget : public QtPWidget {
    public:
      QtTopWidget(QWidget* parent, const Pds::DetInfo& i, unsigned ch) : 
	QtPWidget(parent), info(i), channel(ch) {}
      virtual ~QtTopWidget() {}
    public:
      virtual const QString& title() const = 0;
      virtual void save_setup(char*& p) const { save(p); }
      virtual void load_setup(const char*& p) { load(p); }
      virtual void save_plots(const QString&) const = 0;
      virtual void reset_plots() = 0;
    public:
      const Pds::DetInfo info;
      unsigned           channel;
    };
  };
};

#endif
