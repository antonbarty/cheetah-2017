#ifndef AmiQt_DescScan_hh
#define AmiQt_DescScan_hh

#include "ami/qt/FeatureList.hh"

#include <QtGui/QWidget>
#include <QtCore/QString>

class QRadioButton;
class QLineEdit;

namespace Ami {
  namespace Qt {
    class DescScan : public QWidget {
      Q_OBJECT
    public:
      DescScan(const char*, FeatureRegistry* =0);
    public:
      void save(char*&) const;
      void load(const char*&);
    public:
      QRadioButton* button();
      QString  expr() const;
      QString  feature() const;
      unsigned bins() const;
    private:
      QRadioButton* _button;
      QLineEdit*  _bins;
      FeatureList* _features;
    };
  };
};

#endif      
