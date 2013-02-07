#ifndef AmiQt_DescTH2F_hh
#define AmiQt_DescTH2F_hh

#include <QtGui/QWidget>

class QRadioButton;
class QLineEdit;
class QLayout;

namespace Ami {
  namespace Qt {
    class DescTH2F : public QWidget {
      Q_OBJECT
    public:
      DescTH2F(QLayout*);
    public:
      void save(char*&) const;
      void load(const char*&);
    public:
      QRadioButton* td_button();
      QRadioButton* im_button();
      unsigned xbins() const;
      double   xlo  () const;
      double   xhi  () const;
      unsigned ybins() const;
      double   ylo  () const;
      double   yhi  () const;
    public:
      void xbins(unsigned);
      void xlo  (double);
      void xhi  (double);
      void ybins(unsigned);
      void ylo  (double);
      void yhi  (double);
    public slots:
      void validate();
    private:
      QRadioButton* _td_button;
      QRadioButton* _im_button;
      QLineEdit *_xbins, *_xlo, *_xhi;
      QLineEdit *_ybins, *_ylo, *_yhi;
    };
  };
};

#endif      
