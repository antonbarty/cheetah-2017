#ifndef AmiQt_PrintAction_hh
#define AmiQt_PrintAction_hh

#include <QtGui/QAction>

class QPrinter;
class QWidget;

namespace Ami {
  namespace Qt {
    class PrintAction : public QAction {
      Q_OBJECT
    public:
      PrintAction(QWidget& d);
      ~PrintAction();
    public:
      static QPrinter* printer();
    public slots:
      void print();
    private:
      QWidget&  _display;
      static QPrinter* _printer;
    };
  };
};

#endif
