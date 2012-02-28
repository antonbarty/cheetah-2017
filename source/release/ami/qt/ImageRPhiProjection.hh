#ifndef AmiQt_ImageRPhiProjection_hh
#define AmiQt_ImageRPhiProjection_hh

#include "ami/qt/QtPWidget.hh"

#include <QtCore/QString>
#include <QtCore/QStringList>

class QLineEdit;
class QButtonGroup;
//class QCheckBox;
class QTabWidget;

#include <list>

namespace Ami {

  class AbsOperator;
  class Cds;
  class Entry;

  namespace Qt {
    class AnnulusCursors;
    class ChannelDefinition;
    class ImageFrame;
    class ProjectionPlot;
    class CursorPlot;
    class RPhiProjectionPlotDesc;
    class ImageIntegral;

    class ImageRPhiProjection : public QtPWidget {
      Q_OBJECT
    public:
      ImageRPhiProjection(QWidget* parent,
			  ChannelDefinition* channels[], unsigned nchannels, ImageFrame&);
      ~ImageRPhiProjection();
    public:
      void save(char*& p) const;
      void load(const char*& p);
      void save_plots(const QString&) const;
    public:
      void configure(char*& p, unsigned input, unsigned& output,
		     ChannelDefinition* ch[], int* signatures, unsigned nchannels);
      void setup_payload(Cds&);
      void update();
    public slots:
      void set_channel(int); // set the source
      void plot        ();   // configure the plot
      void configure_plot();
      void remove_plot (QObject*);
      virtual void setVisible(bool);
      void update_range();
    signals:
      void changed();
    private:
      ChannelDefinition** _channels;
      unsigned _nchannels;
      unsigned _channel;

      ImageFrame&  _frame;
      AnnulusCursors* _annulus;

      QLineEdit* _title;
//       QButtonGroup* _axis;
//       QButtonGroup* _norm;
//       QCheckBox*    _transform;

//       Ami::AbsOperator* _operator;

      QTabWidget*   _plot_tab;
      RPhiProjectionPlotDesc* _projection_plot;
      ImageIntegral*          _integral_plot;

      std::list<ProjectionPlot*> _pplots;
      std::list<CursorPlot*>     _cplots;
    };
  };
};

#endif
