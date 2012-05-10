#ifndef AmiQt_Contour_hh
#define AmiQt_Contour_hh

#include "ami/qt/ImageMarker.hh"
#include "ami/data/ContourProjection.hh"
#include <QtGui/QWidget>

#include "ami/data/Contour.hh"

class QLabel;
class QLineEdit;

namespace Ami {
  namespace Qt {
    class ImageFrame;
    class RectangleCursors;

    class Contour : public QWidget, 
		    public ImageMarker {
    public:
      Contour(const char* x, const char* y,
	      Ami::ContourProjection::Axis a,
	      const ImageFrame&,
	      const RectangleCursors&);
      ~Contour();
    public:
      void draw(QImage&);
      void setup(const char*,const char*,Ami::ContourProjection::Axis);
      void save(char*& p) const;
      void load(const char*& p);
    public:
      Ami::Contour value() const;
    private:
      const ImageFrame&       _image;
      const RectangleCursors& _frame;
      Ami::ContourProjection::Axis _axis;
      QLabel*    _x[Ami::Contour::MaxOrder+1];
      QLineEdit* _c[Ami::Contour::MaxOrder+1];
      QLineEdit* _discrimLevelEdit;
      double     _discrimLevel;
    };
  };
};

#endif
