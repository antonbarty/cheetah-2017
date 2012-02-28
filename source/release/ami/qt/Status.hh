#ifndef AmiQt_Status_hh
#define AmiQt_Status_hh

#include <QtGui/QWidget>

class QLabel;

namespace Ami {
  namespace Qt {
    class Status : public QWidget {
      Q_OBJECT
    public:
      Status();
      ~Status();
    public:
      enum State { Disconnected, 
		   Connected, 
		   Discovered, 
		   Configured,
		   Described,
		   Requested,
		   Received,
		   Processed };
      void set_state(State);
      State state() const { return _state; }
    public slots:
      void update_state();
    signals:
      void state_changed();
    private:
      State   _state;
      QLabel* _label;
    };
  };
};

#endif
