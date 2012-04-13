#include "FeatureTree.hh"

#include "ami/qt/FeatureRegistry.hh"
#include "ami/qt/QtPersistent.hh"

#include <QtGui/QPalette>

using namespace Ami::Qt;

static const QString _separator(":");

static void push(QStandardItem& root, const QString& name, int level)
{
  QStringList name_f = name.split(_separator).mid(level);
  if (name_f.size()==0) {
    root.insertRow(0, new QStandardItem(name) );
    return;
  }

  for(int i=0; i<root.rowCount(); i++) {
    QStandardItem& row = *root.child(i);
    QStringList row_f = row.text().split(_separator).mid(level);

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
          push(row, name, level+row_f.size());
        else {
          //  Create a tree from the row and append
          row.appendRow( new QStandardItem(row.text()) );
          row.appendRow( new QStandardItem(name) );
        }
      }
      else {
        //  Create a new root with these two children
        QStringList common = name.split(_separator).mid(0,level);
        while( name_f.size() && row_f.size() && name_f[0] == row_f[0] ) {
          common.append( name_f.takeFirst() );
          row_f.takeFirst();
        }
        QStandardItem* branch = new QStandardItem( common.join(_separator) );
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


FeatureTree::FeatureTree() :
  QPushButton(),
  _entry     ()
{
  _view.setModel(&_model);

  fill(FeatureRegistry::instance().names());

  connect(this, SIGNAL(clicked()), &_view, SLOT(show()));
  connect(&FeatureRegistry::instance(), SIGNAL(changed()), this, SLOT(change_features()));
  connect(&_view, SIGNAL(clicked(const QModelIndex&)), this, SLOT(set_entry(const QModelIndex&)));
}

FeatureTree::FeatureTree(const QStringList& names, const QStringList& help, const QColor& color) :
  QPushButton(),
  _entry     ()
{
  QPalette newPalette = palette();
  newPalette.setColor(QPalette::Button, color);
  setPalette(newPalette);

  _view.setWindowFlags(::Qt::Dialog);
  _view.setWindowModality(::Qt::WindowModal);
  //  _view.setWindowFlags(::Qt::Popup);
  _view.setModel(&_model);

  fill(names);

  connect(this, SIGNAL(clicked()), &_view, SLOT(show()));
  connect(&_view, SIGNAL(clicked(const QModelIndex&)), this, SLOT(set_entry(const QModelIndex&)));
}

void FeatureTree::fill(const QStringList& names)
{

  if (names.size()) {
    QStandardItem* root = _model.invisibleRootItem();
    root->appendRow( new QStandardItem( names.at(0) ) );
    for(int i=1; i<names.size(); i++) {
      push(*root, names.at(i),0);
    }
  }
  set_entry(_entry);
}

FeatureTree::~FeatureTree()
{
}

void FeatureTree::save(char*& p) const
{
  XML_insert( p, "QString", "_entry",
              QtPersistent::insert(p, _entry) );
}

void FeatureTree::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if (tag.element == "QString")
      set_entry(QtPersistent::extract_s(p));
  XML_iterate_close(FeatureTree,tag);
}

const QString& FeatureTree::entry() const { return _entry; }

void FeatureTree::change_features()
{
  clear();
  fill(FeatureRegistry::instance().names());
}

void FeatureTree::clear()
{
  QStandardItem* root = _model.invisibleRootItem();
  root->removeRows(0,root->rowCount());
}

void FeatureTree::set_entry(const QModelIndex& e) { 
  const QString& entry = _model.itemFromIndex(e)->text();
  set_entry(entry);
  emit activated(entry);
}

void FeatureTree::set_entry(const QString& e) { 
  QPalette p(palette());
  if (FeatureRegistry::instance().names().contains(e))
    p.setColor(QPalette::ButtonText, QColor(0,0,0));
  else
    p.setColor(QPalette::ButtonText, QColor(0xc0,0,0));

  setText(_entry=e);
  setPalette(p);
}

