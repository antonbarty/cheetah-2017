#ifndef AmiQt_FeatureTree_hh
#define AmiQt_FeatureTree_hh

#include "ami/qt/QtTree.hh"

class QColor;

namespace Ami {
  namespace Qt {
    class FeatureRegistry;

    class FeatureTree : public QtTree {
      Q_OBJECT
    public:
      FeatureTree(FeatureRegistry* =0);
      FeatureTree(const QStringList& names, const QStringList& help, const QColor& color);
      ~FeatureTree();
    public slots:
      void change_features();
    private:
      bool _valid_entry(const QString&) const;
    private:
      FeatureRegistry*   _registry;
    };
  };
};

#endif
