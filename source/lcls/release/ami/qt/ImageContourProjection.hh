#ifndef AmiQt_ImageContourProjection_hh
#define AmiQt_ImageContourProjection_hh

#include "ami/qt/QtPWidget.hh"

#include <QtCore/QString>
#include <QtCore/QStringList>

class QLineEdit;
class QButtonGroup;

#include <list>

namespace Ami {

  class Cds;
  class Entry;

  namespace Qt {
    class ChannelDefinition;
    class RectangleCursors;
    class Contour;
    class ImageFrame;
    class ProjectionPlot;

    class ImageContourProjection : public QtPWidget {
      Q_OBJECT
    public:
      ImageContourProjection(QWidget* parent,
			     ChannelDefinition* channels[], unsigned nchannels, ImageFrame&);
      ~ImageContourProjection();
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
      void use_xaxis(bool);
      void set_channel(int); // set the source
      void plot        ();   // configure the plot
      void configure_plot();
      void remove_plot (QObject*);
      virtual void setVisible(bool);
    signals:
      void changed();
    private:
      ChannelDefinition** _channels;
      unsigned _nchannels;
      unsigned _channel;

      ImageFrame&       _frame;
      RectangleCursors* _rectangle;

      QLineEdit*    _title;
      QButtonGroup* _axis;
      QButtonGroup* _norm;
      Contour*      _contour;

      std::list<ProjectionPlot*> _pplots;
    };
  };
};

#endif
