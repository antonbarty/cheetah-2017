#ifndef AmiQt_ImageColorControl_hh
#define AmiQt_ImageColorControl_hh

#include "ami/qt/QtPersistent.hh"

#include <QtGui/QGroupBox>

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QPixmap>
#include "ColorMaps.hh"

class QButtonGroup;
class QCheckBox;
class QLabel;
class QLineEdit;

namespace Ami {
  namespace Qt {
    class ImageColorControl : public QGroupBox, 
			      public QtPersistent {
      Q_OBJECT
    public:
      ImageColorControl(QWidget*, const QString& title);
      ~ImageColorControl();
    public:
      void save(char*& p) const;
      void load(const char*& p);
    public:
      bool   linear  () const;
      float  pedestal() const;
      float  scale   () const;
      const QVector<QRgb>& color_table() const;
    public:
      static void set_color_choice(int);
    public slots:
      void show_scale();
      void set_auto(bool);
      void zoom();
      void pan ();
      void set_palette(int);
      void scale_min_changed();
    signals:
      void windowChanged();
    private:
      int     _scale;
      float   _pedestal;
      QString _title;
      QVector<QRgb>* _color_table;
      QLineEdit* _scale_min;
      QLabel* _scale_mid;
      QLabel* _scale_max;
      QButtonGroup* _paletteGroup;
      QCheckBox* _logscale;
    };
  };
};

#endif
