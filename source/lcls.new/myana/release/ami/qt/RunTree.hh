#ifndef AmiQt_RunTree_hh
#define AmiQt_RunTree_hh

#include "ami/qt/QtTree.hh"

class QColor;

namespace Ami {
  namespace Qt {
    class RunTree : public QtTree {
      Q_OBJECT
    public:
      RunTree();
      ~RunTree();
    public:
      void addItems(const QStringList&);
    public:
      const QString& currentText() const;
    signals:
      void currentIndexChanged(int);
    public slots:
      void changeIndex(const QString&);
    };
  };
};

#endif
