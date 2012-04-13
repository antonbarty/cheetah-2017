#include "FeatureRegistry.hh"
#include "Path.hh"

#include "ami/data/FeatureCache.hh"
#include "ami/data/Discovery.hh"

using namespace Ami::Qt;

static FeatureRegistry* _instance=0;

FeatureRegistry& FeatureRegistry::instance() 
{
  if (!_instance) _instance = new FeatureRegistry;
  return *_instance;
}

void               FeatureRegistry::insert(const DiscoveryRx& rx)
{
  _sem.take();
  _names.clear();
  _help .clear();
  for(unsigned k=0; k<rx.features(); k++) {
    _names << QString(rx.feature_name(k));
    _help  << QString();
  }

  FILE* f = Path::helpFile();
  if (f) {
    char* line = new char[256];
    while( fgets(line, 256, f) ) {
      if (line[0]=='#') continue;  // comment field
      static const char* delim = " \t";
      char* p = line;
      strsep(&p,delim);
      if (p) {
	p += strspn(p,delim);
	*strchrnul(p,'\n') = 0;
	QString name(line);
	int index = _names.indexOf(name);
	if (index >= 0)
	  _help[index] = QString(p);
      }
    }
    delete[] line;
    fclose(f);
  }

  _sem.give();
  emit changed();
}

QStringList FeatureRegistry::names () const
{
  _sem.take();
  QStringList n(_names);
  _sem.give();
  return n;
}

const QStringList& FeatureRegistry::help() const
{
  return _help;
}

int FeatureRegistry::index(const QString& name) const
{
  const_cast<FeatureRegistry*>(this)->_sem.take();
  int v = _names.indexOf(name);
  const_cast<FeatureRegistry*>(this)->_sem.give();
  return v;
}

FeatureRegistry::FeatureRegistry() : _sem(Ami::Semaphore::FULL) {}

FeatureRegistry::~FeatureRegistry() {}
