#ifndef AmiQt_ImageScale_hh
#define AmiQt_ImageScale_hh

#include <QtGui/QWidget>
#include <QtCore/QString>

class QLineEdit;

namespace Ami {
  class DescEntry;
  namespace Qt {
    class ImageScale : public QWidget {
      Q_OBJECT
    public:
      ImageScale(const QString& title);
      ~ImageScale();
    public:
      void     prototype(const Ami::DescEntry&);
      double   value(unsigned) const;
      void     value(unsigned,double);
    public slots:
      void     redo_layout();
    signals:
      void     changed();
    private:
      QString    _title;
      QString    _zunits;
      QLineEdit* _input0;
      QLineEdit* _input1;
      bool       _hasGain;
      bool       _hasSigma;
    };
  };
};

#endif
