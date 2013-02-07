#include "ImageColorControl.hh"

#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QDoubleValidator>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QCheckBox>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>

#include <math.h>

using namespace Ami::Qt;

enum { Mono=0, Color=1 };

static int _colorChoice=0;
void ImageColorControl::set_color_choice(int c) { _colorChoice=c; }

static ColorMaps _colorMaps;

static QVector<QRgb>* get_palette_from_colormap(unsigned int colormap[])
{
  QVector<QRgb>* color_table = new QVector<QRgb>(256);
  for (int i = 0; i < 256; i++) {
    unsigned int rgb = colormap[i];
    unsigned int r = (rgb & 0xff0000) >> 16;
    unsigned int g = (rgb & 0xff00) >> 8;
    unsigned int b = (rgb & 0xff);
    color_table->insert(i, qRgb(r, g, b));
  }
  return color_table;
}

static QVector<QRgb>* jet_palette()
{
  return get_palette_from_colormap(_colorMaps.get("jet"));
}

static QVector<QRgb>* monochrome_palette()
{
  return get_palette_from_colormap(_colorMaps.get("gray"));
}

static QVector<QRgb>* thermal_palette()
{
  QVector<QRgb>* color_table = new QVector<QRgb>(256);
  for (int i = 0; i < 43; i++)  // black - red
    color_table->insert(  0+i, qRgb(i*6,0,0));
  for (int i = 0; i < 86; i++)  // red - green
    color_table->insert( 43+i, qRgb(255-i*3,i*3,0));
  for (int i = 0; i < 86; i++)  // green - blue
    color_table->insert(129+i, qRgb(0,255-i*3,i*3));
  for (int i = 0; i < 40; i++)  // blue - violet
    color_table->insert(215+i, qRgb(i*3,0,255-i*3));
  color_table->insert(255, qRgb(255,255,255));
  return color_table;
}

static QVector<QRgb>* color_palette()
{
  return _colorChoice==0 ? jet_palette() : thermal_palette();
}

ImageColorControl::ImageColorControl(QWidget* parent,
				     const QString&  title) :
  QGroupBox(title,parent),
  _scale (0),
  _pedestal(0),
  _scale_min(new QLineEdit),
  _scale_mid(new QLabel),
  _scale_max(new QLabel)
{
  _scale_min->setMaximumWidth(60);
  _scale_min->setValidator(new QDoubleValidator(_scale_min));
  _scale_min->setText(QString::number(_pedestal));

  _scale_min->setAlignment(::Qt::AlignLeft);
  _scale_mid->setAlignment(::Qt::AlignHCenter);
  _scale_max->setAlignment(::Qt::AlignRight);

  setAlignment(::Qt::AlignHCenter);

  QPushButton* autoB = new QPushButton("Reset");
  QPushButton* zoomB = new QPushButton("Zoom");
  QPushButton* panB  = new QPushButton("Pan");

  QImage palette(256,16,QImage::Format_Indexed8);
  { unsigned char* dst = palette.bits();
    for(unsigned k=0; k<256*16; k++) *dst++ = k&0xff; }

  QRadioButton* monoB  = new QRadioButton; 
  palette.setColorTable(*(_color_table = monochrome_palette()));
  QLabel* monoC = new QLabel;
  monoC->setPixmap(QPixmap::fromImage(palette));
  delete _color_table;

  QRadioButton* colorB = new QRadioButton; 
  palette.setColorTable(*(_color_table = color_palette()));
  QLabel* colorC = new QLabel;
  colorC->setPixmap(QPixmap::fromImage(palette));

  _paletteGroup = new QButtonGroup;
  _paletteGroup->addButton(monoB, Mono);
  _paletteGroup->addButton(colorB, Color);

  _logscale = new QCheckBox("Log Scale");
  _logscale->setChecked(false);

  QVBoxLayout* layout = new QVBoxLayout;
  { QHBoxLayout* layout1 = new QHBoxLayout;
    layout1->addStretch();
    layout1->addWidget(zoomB);
    layout1->addWidget(autoB);
    layout1->addWidget(panB);
    layout1->addStretch();
    layout->addLayout(layout1); }
  { QHBoxLayout* layout1 = new QHBoxLayout;
    layout1->addStretch();
    { QGridLayout* layout2 = new QGridLayout;
      layout2->addWidget(monoB,0,0);
      layout2->addWidget(monoC,0,1,1,3);
      layout2->addWidget(colorB,1,0);
      layout2->addWidget(colorC,1,1,1,3);
      layout2->addWidget(_scale_min,2,1,::Qt::AlignLeft);
      layout2->addWidget(_scale_mid,2,2,::Qt::AlignCenter); 
      layout2->addWidget(_scale_max,2,3,::Qt::AlignRight); 
      //  Set the columns to equal (dynamic) size
      layout2->setColumnMinimumWidth(1,60);
      layout2->setColumnMinimumWidth(2,60);
      layout2->setColumnMinimumWidth(3,60);
      layout2->setColumnStretch(1,1);
      layout2->setColumnStretch(2,1);
      layout2->setColumnStretch(3,1);
      layout1->addLayout(layout2); }
    layout1->addStretch();
    layout->addLayout(layout1); }
  layout->addWidget(_logscale,0,::Qt::AlignCenter);
  setLayout(layout);

  connect(autoB , SIGNAL(clicked(bool)), this, SLOT(set_auto(bool)));
  connect(zoomB , SIGNAL(clicked()), this, SLOT(zoom()));
  connect(panB  , SIGNAL(clicked()), this, SLOT(pan ()));
  connect(_paletteGroup, SIGNAL(buttonClicked(int)), this, SLOT(set_palette(int)));
  connect(_scale_min, SIGNAL(editingFinished()), this, SLOT(scale_min_changed()));
  connect(_logscale, SIGNAL(clicked()), this, SIGNAL(windowChanged()));
  connect(this  , SIGNAL(windowChanged()), this, SLOT(show_scale()));

  colorB->setChecked(true);
  show_scale();
}   

ImageColorControl::~ImageColorControl()
{
}

void ImageColorControl::save(char*& p) const
{
  XML_insert(p, "int", "scale", QtPersistent::insert(p,_scale) );
  XML_insert(p, "QButtonGroup", "_paletteGroup", QtPersistent::insert(p,_paletteGroup->checkedId()) );
  XML_insert(p, "double", "_pedestal", QtPersistent::insert(p,double(_pedestal)) );
  XML_insert(p, "QCheckBox", "_logscale", QtPersistent::insert(p,_logscale->isChecked()) );
}

void ImageColorControl::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if (tag.name == "scale")
      _scale = QtPersistent::extract_i(p);
    else if (tag.name == "_paletteGroup") {
      int palette = QtPersistent::extract_i(p);
      _paletteGroup->button(palette)->setChecked(true);
      set_palette(palette);
    }
    else if (tag.name == "_pedestal")
      _pedestal     = QtPersistent::extract_d(p);
    else if (tag.name == "_logscale")
      _logscale->setChecked(QtPersistent::extract_b(p));
  XML_iterate_close(ImageColorControl,tag);

  show_scale();
  emit windowChanged();
}

bool   ImageColorControl::linear() const { return !_logscale->isChecked(); }

float ImageColorControl::pedestal() const { return _pedestal; }

float ImageColorControl::scale() const { return powf(2,0.5*float(-_scale)); }

const QVector<QRgb>& ImageColorControl::color_table() const { return *_color_table; }

void   ImageColorControl::set_palette(int p)
{
  delete _color_table;
  _color_table = (p==Mono) ? monochrome_palette() : color_palette();

  emit windowChanged();
}

void   ImageColorControl::show_scale()
{
  unsigned v = static_cast<unsigned>(0xff*scale());
  if (_logscale->isChecked()) {
    _scale_min->setText(QString::number(_pedestal+1));
    _scale_mid->setText(QString::number(_pedestal+int(sqrt(v)+0.5)));
    _scale_max->setText(QString::number(_pedestal+v));
  }
  else {
    _scale_min->setText(QString::number(_pedestal));
    _scale_mid->setText(QString::number(_pedestal+(v>>1)));
    _scale_max->setText(QString::number(_pedestal+v));
  }
}

void   ImageColorControl::set_auto(bool s)
{
  _scale = 0;
  emit windowChanged();
}

void   ImageColorControl::zoom ()
{
  ++_scale;
  emit windowChanged();
}

void   ImageColorControl::pan ()
{
  --_scale;
  emit windowChanged();
}

void   ImageColorControl::scale_min_changed()
{
  _pedestal = _scale_min->text().toFloat();
  emit windowChanged();
}
