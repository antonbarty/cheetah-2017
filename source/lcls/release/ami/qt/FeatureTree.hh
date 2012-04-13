#ifndef AmiQt_FeatureTree_hh
#define AmiQt_FeatureTree_hh

#include <QtGui/QTreeView>
#include <QtGui/QPushButton>

#include <QtGui/QStandardItemModel>
#include <QtCore/QString>

class QColor;

namespace Ami {
  namespace Qt {
    class FeatureTree : public QPushButton {
      Q_OBJECT
    public:
      FeatureTree();
      FeatureTree(const QStringList&, const QStringList&, const QColor&);
      ~FeatureTree();
    public:
      void save(char*&) const;
      void load(const char*&);
    public:
      const QString& entry() const;
      void  clear();
      void  fill (const QStringList&);
    public slots:
      void change_features();
      void set_entry(const QModelIndex&);
      void set_entry(const QString&);
    signals:
      void activated(const QString&);
    private:
      QStandardItemModel _model;
      QTreeView          _view;
      QString            _entry;
    };
  };
};

#endif
