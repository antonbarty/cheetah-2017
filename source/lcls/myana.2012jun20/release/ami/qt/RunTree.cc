#include "RunTree.hh"

using namespace Ami::Qt;

RunTree::RunTree() :
  QtTree("")
{
  connect(this, SIGNAL(activated(const QString&)), this, SLOT(changeIndex(const QString&)));
}

RunTree::~RunTree()
{
}

void RunTree::addItems(const QStringList& names)
{
  fill(names);
 
  QStandardItem* root = _model.invisibleRootItem();
  int row=0;
  for(QStandardItem* child; (child=root->child(row))!=NULL; row++)
    _view.setExpanded(child->index(),true);
}

const QString& RunTree::currentText() const
{
  return entry(); 
}

void RunTree::changeIndex(const QString&)
{
  emit currentIndexChanged(0);
}
