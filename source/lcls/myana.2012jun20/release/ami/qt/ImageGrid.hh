#ifndef AmiQt_ImageGrid_hh
#define AmiQt_ImageGrid_hh

#include <QtGui/QLabel>

namespace Ami {
  namespace Qt {
    class ImageGrid : public QLabel {
    public:
      enum Axis { X, Y };
      enum Origin { TopLeft, Center };
      ImageGrid( Axis, Origin, double y0, double dy, unsigned ny );
      ~ImageGrid();
    public:
      void set_scale(double y0, double dy);
      void resize   (unsigned ny);
    private:
      void _fill();
    private:
      Axis     _axis;
      Origin   _origin;
      double   _y0; // value of bin0
      double   _dy; // length of axis
      unsigned _ny; // display pixels
    };
  };
};

#endif
