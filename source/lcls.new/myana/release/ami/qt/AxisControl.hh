#ifndef AmiQt_AxisControl_hh
#define AmiQt_AxisControl_hh

#include <QtGui/QWidget>

#include <QtCore/QString>

class QLineEdit;
class QPushButton;

namespace Ami {
  namespace Qt {
    class AxisInfo;
    class AxisControl : public QWidget {
      Q_OBJECT
    public:
      AxisControl(QWidget*, const QString& title, bool lSlim=true);
      ~AxisControl();
    public:
      void save(char*& p) const;
      void load(const char*& p);
    public:
      void update(const AxisInfo&);
    public:
      bool   isLog () const;
      bool   isAuto() const;
      double loEdge() const;
      double hiEdge() const;
    public slots:
      void changeLoEdge(const QString&);
      void changeHiEdge(const QString&);
      void auto_scale(bool);
      void log_scale (bool);
      void updateInfo();
    signals:
      void windowChanged();
    private:
      QString _title;
      const AxisInfo* _info;
      QLineEdit*      _loBox;
      QLineEdit*      _hiBox;
      QPushButton*    _autoB;
      QPushButton*    _logB;
    };
  };
};

#endif
