#ifndef AmiQt_AnnulusCursors_hh
#define AmiQt_AnnulusCursors_hh

#include <QtGui/QWidget>
#include "ami/qt/Cursors.hh"
#include "ami/qt/ImageMarker.hh"

class QLineEdit;

namespace Ami {
  namespace Qt {
    class ImageFrame;
    class AnnulusCursors : public QWidget,
			   public Cursors,
			   public ImageMarker {
      Q_OBJECT
    public:
      AnnulusCursors(ImageFrame&);
      ~AnnulusCursors();
    public:
      void save(char*& p) const;
      void load(const char*& p);
    public:
      double xcenter() const { return _xc; }
      double ycenter() const { return _yc; }
      double r_inner() const { return _r0; }
      double r_outer() const { return _r1; }
      double phi0   () const { return _f0; }
      double phi1   () const { return _f1; }
      unsigned nrbins() const;
    public:   // ImageMarker interface
      void draw(QImage&);
    public slots:
      void grab_center();
      void grab_limits();
      void update_edits();
    public:  // Cursors interface
      void mousePressEvent  (double,double);
      void mouseMoveEvent   (double,double);
      void mouseReleaseEvent(double,double);
    private:
      void _set_edits ();
    signals:
      void changed();
    private:
      ImageFrame& _frame;
      enum Cursor { None, Center, Limits, NumberOf };
      Cursor _active;
      double _xc,_yc;
      double _r0,_r1;
      double _f0,_f1;
      QLineEdit* _edit_xc;
      QLineEdit* _edit_yc;
      QLineEdit* _edit_inner;
      QLineEdit* _edit_outer;
      QLineEdit* _edit_phi0;
      QLineEdit* _edit_phi1;
    };
  };
};

#endif
