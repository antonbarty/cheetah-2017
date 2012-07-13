#ifndef AmiQt_QtPWidget_hh
#define AmiQt_QtPWidget_hh

#include "QtPersistent.hh"

#include <QtGui/QWidget>

namespace Ami {
  namespace Qt {
    class QtPWidget : public QWidget { // , public QtPersistent {
      Q_OBJECT
    public:
      QtPWidget();
      QtPWidget(QWidget* parent);
      virtual ~QtPWidget();
    public:
      virtual void save(char*& p) const;
      virtual void load(const char*& p);
    };
  };
};

#endif
