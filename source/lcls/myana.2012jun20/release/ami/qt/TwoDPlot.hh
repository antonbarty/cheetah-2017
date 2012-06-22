#ifndef AmiQt_TwoDPlot_hh
#define AmiQt_TwoDPlot_hh

//=========================================================
//
//  TwoDPlot for the Analysis and Monitoring Implementation
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
    class ImageXYProjection;
    class ImageRPhiProjection;
    class ImageContourProjection;
    class ImageDisplay;
    class TwoDPlot : public QtPWidget {
      Q_OBJECT
    public:
      TwoDPlot(QWidget*,
               const QString&,
               Ami::AbsOperator*);
      TwoDPlot(QWidget*,const char*&);
      ~TwoDPlot();
    public:
      virtual void save(char*& p) const;
      virtual void load(const char*& p);
      void save_plots(const QString&) const;
    public:
      void configure(char*& p, 
		     unsigned input, 
		     unsigned& output);
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
      ImageDisplay*      _frame;
      const DescEntry*   _input_entry;

      ImageXYProjection*      _xyproj;
      ImageRPhiProjection*    _rfproj;
      ImageContourProjection* _cntproj;

      unsigned           _showMask;
    };
  };
};

#endif      
