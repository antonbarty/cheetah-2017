#include "QtTree.hh"

#include "ami/qt/QtPersistent.hh"

#include <QtGui/QPalette>

using namespace Ami::Qt;

static void push(QStandardItem& root, 
                 const QString& name, 
                 int level,
                 const QString& separator)
{
  QStringList name_f = name.split(separator,QString::SkipEmptyParts).mid(level);

  if (name_f.size()==0) {
    root.insertRow(0, new QStandardItem(name) );
    return;
  }

  for(int i=0; i<root.rowCount(); i++) {
    QStandardItem& row = *root.child(i);
    QStringList row_f = row.text().split(separator,QString::SkipEmptyParts).mid(level);

    //  If they share any leading fields:
    if (name_f.size() && row_f.size() && 
        name_f[0]==row_f[0] &&
        row.text()!=root.text()) {
      //  If they are equal upto the row's length:
      if (name_f.size()>=row_f.size() &&
          name_f.mid(0,row_f.size()) == row_f) {
        //  If the row has children:
        if (row.rowCount())
          //  push onto the row's tree
          push(row, name, level+row_f.size(),separator);
        else {
          //  Create a tree from the row and append
          row.appendRow( new QStandardItem(row.text()) );
          row.appendRow( new QStandardItem(name) );
        }
      }
      else {
        //  Create a new root with these two children
        QStringList common = name.split(separator,QString::SkipEmptyParts).mid(0,level);
        while( name_f.size() && row_f.size() && name_f[0] == row_f[0] ) {
          common.append( name_f.takeFirst() );
          row_f.takeFirst();
        }
        QStandardItem* branch = new QStandardItem( common.join(separator) );
        root.takeRow(i);
        root.insertRow( i, branch );
        branch->appendRow( &row );
        QStandardItem* leaf = new QStandardItem( name );
        if (name < row.text())
          branch->insertRow(0, leaf);
        else
          branch->appendRow(leaf);
      }
    }
    //  Insert before or after
    else if (name_f[0] < row_f[0]) {
      root.insertRow(i, new QStandardItem(name));
    }
    else
      continue;
    return;
  }
  root.appendRow(new QStandardItem(name));
}

QtTree::QtTree(const QString& separator) :
  QPushButton(),
  _view      (this),
  _entry     (),
  _separator (separator)
{
  _view.setWindowFlags(::Qt::Dialog);
  _view.setWindowModality(::Qt::NonModal);

  _view.setModel(&_model);

  connect(this  , SIGNAL(clicked()), &_view, SLOT(show()));
  connect(&_view, SIGNAL(clicked(const QModelIndex&)), this, SLOT(set_entry(const QModelIndex&)));
}

QtTree::QtTree(const QStringList& names, const QStringList& help, const QColor& color,
               const QString& separator) :
  QPushButton(),
  _view      (this),
  _entry     (),
  _separator (separator)
{

  QPalette newPalette = palette();
  newPalette.setColor(QPalette::Button, color);
  setPalette(newPalette);

  _view.setWindowFlags(::Qt::Dialog);
  _view.setWindowModality(::Qt::NonModal);
  //  _view.setWindowFlags(::Qt::Popup);

  _view.setModel(&_model);

  fill(names);

  connect(this    , SIGNAL(clicked()), &_view, SLOT(show()));
  connect(&_view  , SIGNAL(clicked(const QModelIndex&)), this, SLOT(set_entry(const QModelIndex&)));
}

void QtTree::fill(const QStringList& names)
{

  if (names.size()) {
    QStandardItem* root = _model.invisibleRootItem();
    root->appendRow( new QStandardItem( names.at(0) ) );
    for(int i=1; i<names.size(); i++) {
      push(*root, names.at(i),0,_separator);
    }
  }
  set_entry(_entry);
}

QtTree::~QtTree()
{
}

void QtTree::save(char*& p) const
{
  XML_insert( p, "QString", "_entry",
              QtPersistent::insert(p, _entry) );
}

void QtTree::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if (tag.element == "QString")
      set_entry(QtPersistent::extract_s(p));
  XML_iterate_close(QtTree,tag);
}

const QString& QtTree::entry() const { return _entry; }

void QtTree::clear()
{
  QStandardItem* root = _model.invisibleRootItem();
  root->removeRows(0,root->rowCount());
}

void QtTree::set_entry(const QModelIndex& e) { 
  const QString& entry = _model.itemFromIndex(e)->text();
  set_entry(entry);
  emit activated(entry);
}

void QtTree::set_entry(const QString& e) { 
  QPalette p(palette());
  if (_valid_entry(e)) {
    p.setColor(QPalette::ButtonText, QColor(0,0,0));
    _view.hide();
  }
  else
    p.setColor(QPalette::ButtonText, QColor(0xc0,0,0));

  this->setText(_entry=e);
  this->setPalette(p);
}

bool QtTree::_valid_entry(const QString&) const { return true; }

