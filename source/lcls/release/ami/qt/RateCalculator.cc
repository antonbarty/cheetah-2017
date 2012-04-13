#include "ami/qt/RateCalculator.hh"

#include <QtCore/QString>

using namespace Ami::Qt;

RateCalculator::RateCalculator() : QLabel("0"), _entry(0), _last(0)
{
  connect(this, SIGNAL(changed()), this, SLOT(change()));
}

RateCalculator::~RateCalculator() {}

bool RateCalculator::set_entry(Ami::Entry* entry) {
  _entries = _last = 0;
  return (_entry   = static_cast<Ami::EntryScalar*>(entry));
}

void RateCalculator::update() { emit changed(); }

void RateCalculator::change()
{
  if (_entry && _entry->valid()) {
    _last    = _entry->entries() - _entries;
    _entries = _entry->entries();
    setText(QString::number(_last,'f',0));
  }
  else
    setText(QString("."));
}
