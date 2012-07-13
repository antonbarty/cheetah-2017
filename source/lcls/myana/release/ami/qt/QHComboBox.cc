#include "ami/qt/QHComboBox.hh"

#include <QtGui/QListView>
#include <QtGui/QHelpEvent>
#include <QtGui/QToolTip>

namespace Ami {
  namespace Qt {
    class QHListView : public QListView {
    public:
      QHListView(const QStringList& help) : _help(help) {}
    public:
      bool viewportEvent(QEvent* event) {
	if (event->type() == QEvent::ToolTip) {
	  QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
	  int index = indexAt(helpEvent->pos()).row();
	  if (_help.size()<=index || _help[index].isEmpty())
	    QToolTip::hideText();
	  else {
	    QToolTip::showText(helpEvent->globalPos(), _help[index]);
	  }
	}
	return QListView::viewportEvent(event);
      }
    private:
      const QStringList& _help;
    };
  };
};

using namespace Ami::Qt;

QHComboBox::QHComboBox(const QStringList& v,
		       const QStringList& h) :
  _help(h)
{
  setView(new QHListView(h)); 
	  
  addItems(v);
  setCurrentIndex(0);
}

QHComboBox::QHComboBox(const QStringList& v,
		       const QStringList& h,
		       const QColor&      c) :
    _help(h)
{
  QPalette newPalette = palette();
  newPalette.setColor(QPalette::Button, c);
  setPalette(newPalette);

  QHListView* l = new QHListView(h);
  l->setPalette(newPalette);
  setView(l);
  
  addItems(v);
  setCurrentIndex(0);
}

bool Ami::Qt::QHComboBox::event(QEvent* event) {
  if (event->type() == QEvent::ToolTip) {
    int index = currentIndex();
    if (index < 0 || _help.size()<=index || _help[index].isEmpty())
      QToolTip::hideText();
    else {
      QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
      const QPoint& p = helpEvent->globalPos();
      QToolTip::showText(p, _help[index]);
    }
  }
  return QComboBox::event(event);
}
