#ifndef AmiQt_DescProf_hh
#define AmiQt_DescProf_hh

#include <QtGui/QWidget>
#include <QtCore/QString>

class QComboBox;
class QRadioButton;
class QLineEdit;

namespace Ami {
  namespace Qt {
    class DescProf : public QWidget {
      Q_OBJECT
    public:
      DescProf(const char*);
    public:
      void save(char*&) const;
      void load(const char*&);
    public slots:
      void calc();
    public:
      QRadioButton* button();
      unsigned bins() const;
      double   lo  () const;
      double   hi  () const;
      QString  expr() const;
      QString  feature() const;
    private:
      QRadioButton* _button;
      QLineEdit *_bins, *_lo, *_hi;
      QLineEdit* _expr;
    };
  };
};

#endif      
