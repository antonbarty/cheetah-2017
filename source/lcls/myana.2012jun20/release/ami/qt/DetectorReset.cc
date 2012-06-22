#include "ami/qt/DetectorReset.hh"
#include "ami/qt/QtTopWidget.hh"

using namespace Ami::Qt;

DetectorReset::DetectorReset(QWidget* parent,
			     const std::list<QtTopWidget*>& clients) :
  DetectorGroup("Reset Plots",parent,clients) 
{
}

DetectorReset::DetectorReset(const DetectorReset& clone) :
  DetectorGroup(clone)
{
}

DetectorReset::~DetectorReset()
{
}

void DetectorReset::_apply(QtTopWidget& client)
{
  client.reset_plots();
}
