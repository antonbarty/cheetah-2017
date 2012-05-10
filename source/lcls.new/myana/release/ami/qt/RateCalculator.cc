#include "ami/qt/RateCalculator.hh"

#include <QtCore/QString>

using namespace Ami::Qt;

RateCalculator::RateCalculator() : QLabel("0"), _entry(0), _last(0)
{
  pthread_mutex_init(&_mutex, NULL);
  connect(this, SIGNAL(changed(QString)), this, SLOT(change(QString)));
}

RateCalculator::~RateCalculator() {}

bool RateCalculator::set_entry(Ami::Entry* entry) {
  pthread_mutex_lock(&_mutex);
  _entries = _last = 0;
  bool result = (_entry = static_cast<Ami::EntryScalar*>(entry));
  pthread_mutex_unlock(&_mutex);
  return result;
}

void RateCalculator::update() {
  pthread_mutex_lock(&_mutex);
  if (_entry && _entry->valid()) {
    _last    = _entry->entries() - _entries;
    _entries = _entry->entries();
    emit changed(QString::number(_last,'f', 0));
  } else {
    emit changed(QString("."));
  }
  pthread_mutex_unlock(&_mutex);
}

void RateCalculator::change(QString text)
{
  setText(text);
}
