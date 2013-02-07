#ifndef AmiQt_Condition_hh
#define AmiQt_Condition_hh

#include <QtGui/QWidget>
#include <QtCore/QString>

namespace Ami {
  class FeatureRange;

  namespace Qt {
    class Condition : public QWidget {
      Q_OBJECT
    public:
      Condition(const QString& name, 
		const QString& label,
		FeatureRange* filter);
      ~Condition();
    public:
      const QString& name () const;
      const QString& label() const;
      FeatureRange*  clone() const;
    public slots:
      void remove();
    signals:
      void removed(const QString&);
    private:
      QString    _name;
      QString    _label;
      FeatureRange*  _value;
    };
  };
};

#endif
