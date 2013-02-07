#ifndef AmiQt_ZoomPlot_hh
#define AmiQt_ZoomPlot_hh

//=========================================================
//
//  ZoomPlot for the Analysis and Monitoring Implementation
//
//  Filter configures the server-side filtering
//  Math   configures the server-side operations
//
//=========================================================

#include "ami/qt/QtPWidget.hh"

#include <QtCore/QString>

class QPrinter;

namespace Ami {
  class Cds;
  namespace Qt {
    class ChannelDefinition;
    class ImageDisplay;
    class ZoomPlot : public QtPWidget {
      Q_OBJECT
    public:
      ZoomPlot(QWidget*,
	       const QString&,
	       unsigned input_channel,
	       unsigned x0, 
	       unsigned y0,
	       unsigned x1,
	       unsigned y1);
      ZoomPlot(QWidget*,const char*& p);
      ~ZoomPlot();
    public:
      void save(char*& p) const;
      void load(const char*& p);
    public:
      void configure(char*& p, 
		     unsigned input, 
		     unsigned& output,
		     ChannelDefinition* ch[], 
		     int* signatures, 
		     unsigned nchannels);
      void setup_payload(Cds&);
      void update();
    private:
      QString          _name;
      unsigned         _input;
      unsigned         _signature;
      unsigned         _x0, _y0, _x1, _y1;
      ImageDisplay*    _frame;
    };
  };
};

#endif      
