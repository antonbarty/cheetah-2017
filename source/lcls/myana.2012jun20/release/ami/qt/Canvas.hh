#ifndef AmiQt_Canvas_hh
#define AmiQt_Canvas_hh

#include <QtGui/QWidget>

namespace Ami {
  namespace Qt {
    class Client;
    class Canvas : public QWidget {
      Q_OBJECT
    public:
      Canvas(Client&);
      ~Canvas();
    };
  };
};

#endif
      
   
