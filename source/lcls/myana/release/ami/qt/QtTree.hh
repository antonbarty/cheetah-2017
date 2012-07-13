#ifndef AmiQt_QtTree_hh
#define AmiQt_QtTree_hh

#include <QtGui/QTreeView>
#include <QtGui/QPushButton>

#include <QtGui/QStandardItemModel>
#include <QtCore/QString>

class QColor;

namespace Ami {
  namespace Qt {
    class QtTree : public QPushButton {
      Q_OBJECT
    public:
      QtTree(const QString& separator);
      QtTree(const QStringList&, const QStringList&, const QColor&,
             const QString& separator);
      virtual ~QtTree();
    public:
      void save(char*&) const;
      void load(const char*&);
    public:
      const QString& entry() const;
      void  clear();
      void  fill (const QStringList&);
    public slots:
      void set_entry(const QModelIndex&);
      void set_entry(const QString&);
    signals:
      void activated(const QString&);
    private:
      virtual bool _valid_entry(const QString&) const;
    protected:
      QStandardItemModel _model;
      QTreeView          _view;
      QString            _entry;
      QString            _separator;
    };
  };
};

#endif
