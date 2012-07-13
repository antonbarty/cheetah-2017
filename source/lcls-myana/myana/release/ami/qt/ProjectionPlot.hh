#ifndef AmiQt_ProjectionPlot_hh
#define AmiQt_ProjectionPlot_hh

//=========================================================
//
//  ProjectionPlot for the Analysis and Monitoring Implementation
//
//  Filter configures the server-side filtering
//  Math   configures the server-side operations
//
//=========================================================

#include "ami/qt/QtPWidget.hh"

#include <QtCore/QString>

class QButtonGroup;

namespace Ami {
  class Cds;
  class DescEntry;
  class AbsOperator;
  namespace Qt {
    class ChannelDefinition;
    class CursorsX;
    class PeakFit;
    class WaveformDisplay;
    class ProjectionPlot : public QtPWidget {
      Q_OBJECT
    public:
      ProjectionPlot(QWidget*,
		     const QString&,
		     unsigned input_channel,
		     Ami::AbsOperator*);
      ProjectionPlot(QWidget*,const char*&);
      ~ProjectionPlot();
    public:
      virtual void save(char*& p) const;
      virtual void load(const char*& p);
      void save_plots(const QString&) const;
    public:
      void configure(char*& p, 
		     unsigned input, 
		     unsigned& output);
      void configure(char*& p, 
		     unsigned input, 
		     unsigned& output,
		     ChannelDefinition* ch[], 
		     int* signatures, 
		     unsigned nchannels);
      void setup_payload(Cds&);
      void update();
      void dump(FILE*) const;
    public slots:
      void update_configuration();
    signals:
      void description_changed();
    private:
      void _layout();
    private:
      QString           _name;
      unsigned          _input;
      Ami::AbsOperator* _proj;

      enum {NCHANNELS=4};
      ChannelDefinition* _channels[NCHANNELS];
      WaveformDisplay*   _frame;
      const DescEntry*   _input_entry;
      CursorsX*          _cursors;
      PeakFit*           _peakfit;

      unsigned           _showMask;
    };
  };
};

#endif      
