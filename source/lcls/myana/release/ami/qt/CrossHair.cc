#include "ami/qt/CrossHair.hh"

#include "ami/qt/ImageGridScale.hh"
#include "ami/qt/ImageFrame.hh"
#include "ami/qt/AxisInfo.hh"

#include <QtGui/QCursor>
#include <QtGui/QBitmap>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QDoubleValidator>
#include <QtGui/QGridLayout>

#include <list>

static const int crosshair_size = 4;

static void _balance(unsigned&,unsigned,unsigned&,
                     unsigned&,unsigned,unsigned&);

namespace Ami {
  namespace Qt {
    class CrossHairLocation : public QLineEdit {
    public:
      CrossHairLocation() : QLineEdit("0") 
      {
	setMaximumWidth(40); 
	setValidator(new QDoubleValidator(this));
      }
      CrossHairLocation(const CrossHairLocation& c) : QLineEdit(c.text())
      {
	setMaximumWidth(40); 
	setValidator(new QDoubleValidator(this));
      }
      ~CrossHairLocation() {}
    public:
      double value() const { return text().toDouble(); }
    };
  };
};

using namespace Ami::Qt;

static void dumpCursorSize(::Qt::CursorShape shape)
{
  QPixmap c = QCursor(shape).pixmap();
  printf("cursor[%d] pixmap (%d,%d)\n",
	 shape,c.width(),c.height());
}

CrossHair::CrossHair(ImageGridScale& parent, QGridLayout& layout, unsigned row, bool grab) :
  _parent      ( parent ),
  _frame       ( parent.frame() ),
  _column_edit ( new CrossHairLocation ),
  _row_edit    ( new CrossHairLocation ),
  _value       ( new CrossHairLocation ),
  _visible     ( grab ),
  _scalex      ( 1 ),
  _scaley      ( 1 )
{
  _value->setMaximumWidth(80);
  _value->setReadOnly(true);
  
//   QToolButton* grab_button = new QToolButton;
//   grab_button->setIcon(QCursor(::Qt::ArrowCursor).pixmap());
//   QPushButton* grab_button = new QPushButton(QCursor(::Qt::ArrowCursor).pixmap(),
// 					     QString());
  layout.addWidget(_column_edit, row, 1, ::Qt::AlignHCenter);
  layout.addWidget(_row_edit   , row, 2, ::Qt::AlignHCenter);
  layout.addWidget(_value      , row, 3, ::Qt::AlignHCenter);

  if (grab) {
    QPushButton* grab_button = new QPushButton(QChar(0x271b));
    grab_button->setMaximumWidth(20);

    layout.addWidget( grab_button, row, 4, ::Qt::AlignHCenter);
    
    connect(grab_button, SIGNAL(clicked()), this, SLOT(grab_cursor()));
  }

  _frame.add_marker(*this);

  connect(_column_edit, SIGNAL(editingFinished()), this, SIGNAL(changed()));
  connect(_row_edit   , SIGNAL(editingFinished()), this, SIGNAL(changed()));
}

CrossHair::~CrossHair()
{
  _frame.remove_marker(*this);
}

double CrossHair::column() const { return _column_edit->value(); }
double CrossHair::row   () const { return _row_edit   ->value(); }

void CrossHair::grab_cursor()
{
  _frame.set_cursor_input(this);
}

void CrossHair::mousePressEvent  (double x, double y) 
{
  _column_edit->setText(QString::number(x*_scalex));
  _row_edit   ->setText(QString::number(y*_scaley));
  _frame.set_cursor_input(0);
  emit changed();
}

void CrossHair::draw(QImage& img) 
{
  double x = _column_edit->value()/_scalex;
  double y = _row_edit   ->value()/_scaley;
  
  const AxisInfo& xinfo = *_frame.xinfo();
  const AxisInfo& yinfo = *_frame.yinfo();
  
  unsigned jct = unsigned(xinfo.tick(x+0));
  unsigned kct = unsigned(yinfo.tick(y+0));
  _value->setText(QString::number(_frame.value(jct,kct)));

  if (!_visible) return;

  unsigned jlo = unsigned(xinfo.tick(x-crosshair_size));
  unsigned jhi = unsigned(xinfo.tick(x+crosshair_size));
  unsigned klo = unsigned(yinfo.tick(y-crosshair_size));
  unsigned khi = unsigned(yinfo.tick(y+crosshair_size));
  
  _balance(jlo,jct,jhi,
           klo,kct,khi);

  const unsigned char c = 0xff;
  { unsigned char* cc0 = img.scanLine(kct) + jlo;
    for(unsigned j=jlo; j<=jhi; j++)
      *cc0++ = c; }
  { for(unsigned k=klo; k<=khi; k++)
      *(img.scanLine(k) + jct) = c; }
}

void CrossHair::layoutHeader(QGridLayout& layout)
{
  layout.addWidget(new QLabel("x"), 0, 1, ::Qt::AlignHCenter);
  layout.addWidget(new QLabel("y"), 0, 2, ::Qt::AlignHCenter);
  layout.addWidget(new QLabel("value" ), 0, 3, ::Qt::AlignHCenter);
}

//
//  Displayed values reflect multiplication by scale
//

void CrossHair::set_scale(double sx,double sy) 
{
  _column_edit->setText(QString::number(_column_edit->value()*sx/_scalex));
  _row_edit   ->setText(QString::number(_row_edit   ->value()*sy/_scaley));
  _scalex = sx; 
  _scaley = sy; 
  emit changed();
}

void CrossHair::setVisible(bool l) { _visible = l; }


void _balance(unsigned& jlo, unsigned jct, unsigned& jhi,
              unsigned& klo, unsigned kct, unsigned& khi)
{
  unsigned sz = jhi-jct;
  if (jct-jlo < sz) sz=jct-jlo;
  if (khi-kct < sz) sz=khi-kct;
  if (kct-klo < sz) sz=kct-klo;

  jhi = jct+sz;
  jlo = jct-sz;
  khi = kct+sz;
  klo = kct-sz;
}
