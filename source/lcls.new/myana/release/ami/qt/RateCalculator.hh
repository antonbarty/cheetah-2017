#ifndef AmiQt_RateCalculator_hh
#define AmiQt_RateCalculator_hh

#include "ami/data/EntryScalar.hh"
#include <QtGui/QLabel>

namespace Ami {
  namespace Qt {
    class RateCalculator : public QLabel {
      Q_OBJECT
    public:
      RateCalculator();
      ~RateCalculator();
    public:
      bool set_entry(Ami::Entry* entry);
      void update();
    public slots:
      void change(QString text);
    signals:
      void changed(QString text);
    private:
      Ami::EntryScalar* _entry;
      double   _last;
      double   _entries;
      pthread_mutex_t _mutex;
    };
  };
};

#endif
