#include "ami/qt/DetectorGroup.hh"
#include "ami/qt/QtTopWidget.hh"

#include <QtGui/QButtonGroup>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>

using namespace Ami::Qt;

DetectorGroup::DetectorGroup(const QString& label,
			     QWidget*     parent,
			     const std::list<QtTopWidget*>& clients) :
  QtPWidget(parent),
  _clients (clients),
  _buttons (new QButtonGroup)
{
  setWindowTitle(label);
  setAttribute(::Qt::WA_DeleteOnClose, false);

  std::list<QCheckBox*> boxes;
  for(std::list<QtTopWidget*>::const_iterator it = _clients.begin();
      it != _clients.end(); it++) {
    QCheckBox* button = new QCheckBox((*it)->title(),this);
    button->setChecked(false);
    button->setEnabled(false);
    boxes.push_back(button);
  }

  _buttons->setExclusive(false);

  build(boxes);
}

DetectorGroup::DetectorGroup(const DetectorGroup& clone) :
  _clients (clone._clients),
  _buttons (new QButtonGroup)
{
  setWindowTitle(clone.windowTitle());
  setAttribute(::Qt::WA_DeleteOnClose, false);

  //
  //  Construct a new list of check boxes, 
  //  preserving the state of boxes for existing clients
  //
  std::list<QCheckBox*> newlist;
  for(std::list<QtTopWidget*>::const_iterator it = _clients.begin();
      it != _clients.end(); it++) {
    QCheckBox* newbox = new QCheckBox((*it)->title(),this);
    for(QList<QAbstractButton*>::const_iterator bit = clone._buttons->buttons().begin();
	bit != clone._buttons->buttons().end(); bit++) {
      QAbstractButton* pbit = *bit;
      if (pbit!=0 && pbit->text()==(*it)->title()) {
 	newbox->setChecked(pbit->isChecked());
 	break;
      }
    }
    newlist.push_back(newbox);
  }

  _buttons->setExclusive(false);

  build(newlist);

  setVisible(clone.isVisible());
  
  QPoint p = pos();

  move(clone.pos());

//   printf("%s pos %d,%d -> %d,%d [%d,%d]\n",
// 	 qPrintable(windowTitle()),
// 	 p.x(),p.y(),
// 	 pos().x(),pos().y(),
// 	 clone.pos().x(),clone.pos().y());
}

void DetectorGroup::build(const std::list<QCheckBox*>& boxes)
{
  _snapshot = _clients;

  QVBoxLayout* l = new QVBoxLayout;

  QVBoxLayout* client_layout = new QVBoxLayout;

  QStringList names;
  for(std::list<QCheckBox*>::const_iterator it = boxes.begin();
      it != boxes.end(); it++)
    names.append((*it)->text());
  names.sort();

  int i=0;
  for(QList<QString>::const_iterator it = names.begin();
      it != names.end(); it++, i++) {
    for(std::list<QCheckBox*>::const_iterator bit = boxes.begin();
	bit != boxes.end(); bit++) {
      if ((*bit)->text()==(*it)) {
	client_layout->addWidget(*bit);
	_buttons->addButton(*bit,i);
      }
    }
  }

  l->addLayout(client_layout);

  { QHBoxLayout* layout = new QHBoxLayout;
    QPushButton* applyB = new QPushButton("Apply");
    QPushButton* closeB = new QPushButton("Close");
    layout->addWidget(applyB);
    layout->addWidget(closeB);
    l->addLayout(layout); 
    connect(applyB, SIGNAL(clicked()), this, SLOT(apply()));
    connect(closeB, SIGNAL(clicked()), this, SLOT(close())); }

  setLayout(l);
}

DetectorGroup::~DetectorGroup()
{
}

void DetectorGroup::save(char*& p) const
{
  XML_insert(p, "QtPWidget", "self", QtPWidget::save(p) );
  int i=0;
  for(std::list<QtTopWidget*>::const_iterator it = _snapshot.begin();
      it != _snapshot.end(); it++,i++) {
    QAbstractButton* box = _buttons->button(i);
    QtPersistent::insert(p, box->text());
    QtPersistent::insert(p, box->isChecked());
  }
  QtPersistent::insert(p, QString("EndGroup"));
}

void DetectorGroup::load(const char*& p)
{
  QtPWidget::load(p);
  QString name = QtPersistent::extract_s(p);
  while(name!=QString("EndGroup")) {
    int i=0;
    for(std::list<QtTopWidget*>::const_iterator it = _snapshot.begin();
	it != _snapshot.end(); it++,i++) {
      if ((*it)->title() == name) {
	QAbstractButton* box = _buttons->button(i);
	box->setChecked(QtPersistent::extract_b(p));
	break;
      }
    }
    name = QtPersistent::extract_s(p);
  }
}

void DetectorGroup::enable(int i)
{
  QAbstractButton* box = _buttons->button(i);
  box->setEnabled(true);
  box->setChecked(true);
}

void DetectorGroup::disable(int i)
{
  QAbstractButton* box = _buttons->button(i);
  box->setEnabled(false);
  box->setChecked(false);
}

void DetectorGroup::apply()
{
  _init();
  int i=0;
  for(std::list<QtTopWidget*>::const_iterator it = _snapshot.begin();
      it != _snapshot.end(); it++,i++) {
    QAbstractButton* box = _buttons->button(i);
    if (box->isEnabled() && box->isChecked()) {
      QtTopWidget* p = *it;
      _apply(*p);
    }
  }
}

void DetectorGroup::close()
{
  hide();
}
