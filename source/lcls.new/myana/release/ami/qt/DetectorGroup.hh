#ifndef AmiQt_DetectorGroup_hh
#define AmiQt_DetectorGroup_hh

#include "ami/qt/QtPWidget.hh"

#include <list>

class QButtonGroup;
class QVBoxLayout;
class QCheckBox;

namespace Ami {
  namespace Qt {
    class QtTopWidget;

    class DetectorGroup : public QtPWidget {
      Q_OBJECT
    public:
      DetectorGroup(const QString&,
		    QWidget* parent,
		    const std::list<QtTopWidget*>&);
      DetectorGroup(const DetectorGroup&);
      ~DetectorGroup();
    public:
      void save(char*& p) const;
      void load(const char*& p);
    public:
      void enable (int);
      void disable(int);
    public slots:
      void apply();
      void close();
    private:
      virtual void _init () {}
      virtual void _apply(QtTopWidget&) = 0;
    protected:
      void build(const std::list<QCheckBox*>&);

      const std::list<QtTopWidget*>& _clients;
      std::list<QtTopWidget*>        _snapshot;
      QButtonGroup*                  _buttons;
    };
  };
};

#endif
