#include "PrintAction.hh"

#include <QtGui/QPrinter>
#include <QtGui/QPixmap>
#include <QtGui/QLabel>

using namespace Ami::Qt;

PrintAction::PrintAction(QWidget& d) :
  //  QAction("Print",&d),
  QAction("",&d),
  _display(d)
{
  connect(this, SIGNAL(triggered()), this, SLOT(print()));
}

PrintAction::~PrintAction() {}

void PrintAction::print()
{
//   printf("Rendering on printer %s\n",qPrintable(printer()->printerName()));
//   _display.render(printer());
}

QPrinter* PrintAction::printer()
{
  if (!_printer)
    _printer = new QPrinter;
  return _printer;
}

QPrinter* PrintAction::_printer = 0;
