#ifndef AmiQt_ImageMarker_hh
#define AmiQt_ImageMarker_hh

class QImage;

namespace Ami {
  namespace Qt {
    class ImageMarker {
    public:
      virtual ~ImageMarker() {}
    public:
      virtual void draw(QImage&)=0;
    };
  };
};

#endif
