#ifndef AmiQt_PeakPlot_hh
#define AmiQt_PeakPlot_hh

//=========================================================
//
//  PeakPlot for the Analysis and Monitoring Implementation
//
//  Filter configures the server-side filtering
//  Math   configures the server-side operations
//
//=========================================================

#include "ami/qt/QtPWidget.hh"

#include <QtCore/QString>

namespace Ami {
  class Cds;
  namespace Qt {
    class ChannelDefinition;
    class ImageDisplay;
    class ImageXYProjection;
    class ImageRPhiProjection;

    class PeakPlot : public QtPWidget {
      Q_OBJECT
    public:
      PeakPlot(QWidget* parent,
	       const QString&,
	       unsigned input_channel,
	       double threshold_0,
	       double threshold_1);
      PeakPlot(QWidget*, const char*&);
      ~PeakPlot();
    public:
      void save(char*& p) const;
      void load(const char*& p);
      void save_plots(const QString&) const;
    public:
      void configure(char*& p, 
                     unsigned input, 
                     unsigned& output,
                     ChannelDefinition* ch[], 
                     int* signatures, 
                     unsigned nchannels);
      void setup_payload(Cds&);
      void update();
    public slots:
      void update_configuration();
    signals:
      void description_changed();
    private:
      void _layout();
    private:
      QString          _name;
      unsigned         _input;
      double           _threshold_0;
      double           _threshold_1;
      unsigned         _signature;

      enum {NCHANNELS=4};
      ChannelDefinition* _channels[NCHANNELS];
      ImageDisplay*    _frame;
      ImageXYProjection*      _xyproj;
      ImageRPhiProjection*    _rfproj;
      unsigned           _showMask;
    };
  };
};

#endif      
