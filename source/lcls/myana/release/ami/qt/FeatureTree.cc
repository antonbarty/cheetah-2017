#include "FeatureTree.hh"

#include "ami/qt/FeatureRegistry.hh"

using namespace Ami::Qt;

FeatureTree::FeatureTree(FeatureRegistry* r) :
  QtTree     (":"),
  _registry  (r ? r : &FeatureRegistry::instance())
{
  fill(_registry->names());
  connect(_registry, SIGNAL(changed()), this, SLOT(change_features()));
}

FeatureTree::FeatureTree(const QStringList& names, const QStringList& help, const QColor& color) :
  QtTree     (names,help,color,":"),
  _registry  (0)
{
}

FeatureTree::~FeatureTree()
{
}

void FeatureTree::change_features()
{
  clear();
  fill(_registry->names());
}

bool FeatureTree::_valid_entry(const QString& e) const
{
  return _registry==0 || _registry->names().contains(e);
}
