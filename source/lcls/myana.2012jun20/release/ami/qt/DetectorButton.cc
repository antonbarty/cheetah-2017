#include "ami/qt/DetectorButton.hh"
#include "ami/qt/DetectorSelect.hh"

using namespace Ami::Qt;

DetectorButton::DetectorButton(DetectorSelect* parent, const char* name, const Pds::DetInfo& info) :
  QPushButton(name, parent),
  _parent(parent),
  _info(info)
{
  connect(this, SIGNAL(clicked()), this, SLOT(start_detector()));
}

DetectorButton::~DetectorButton()
{
}

void DetectorButton::start_detector()
{
  _parent->start_detector(_info,0);
}
