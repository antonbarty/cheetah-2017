#include "Average.hh"

#include "ami/data/DescEntry.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/EntryWaveform.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryProf.hh"
#include "ami/data/EntryFactory.hh"

#include <stdio.h>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace Ami;

Average::Average(unsigned n) : 
  AbsOperator(AbsOperator::Average),
  _n         (n),
  _entry     (0),
  _cache     (0)
{
}

Average::Average(const char*& p, const DescEntry& e) :
  AbsOperator(AbsOperator::Average)
{
  _extract(p, &_n, sizeof(_n));
  _entry = EntryFactory::entry(e);
  _entry->reset();
  if (_n) {
    _cache = EntryFactory::entry(e);
    _cache->desc().aggregate(false);
  }
  else
    _cache = (Entry*)0;
}

Average::~Average()
{
  if (_entry) delete _entry;
  if (_cache) delete _cache;
}

DescEntry& Average::output   () const { return _n ? _cache->desc() : _entry->desc(); }

void*      Average::_serialize(void* p) const
{
  _insert(p, &_n, sizeof(_n));
  return p;
}

Entry&     Average::_operate(const Entry& e) const
{
  if (e.valid()) {

    _entry->valid(e.time());

    switch(e.desc().type()) {
    case DescEntry::TH1F:
      { EntryTH1F& _en = static_cast<EntryTH1F&>(*_entry);
        _en.add(static_cast<const EntryTH1F&>(e));
        if (_n && _en.info(EntryTH1F::Normalization)==_n) {
          static_cast<EntryTH1F*>(_cache)->setto(_en);
          _en.reset();
        }
        break; }
    case DescEntry::TH2F:
      printf("Averaging TH2F not implemented\n");
      break;
    case DescEntry::Prof:
      { EntryProf& _en = static_cast<EntryProf&>(*_entry);
        _en.sum(_en,static_cast<const EntryProf&>(e));
        if (_n && _en.info(EntryProf::Normalization)==_n) {
          static_cast<EntryProf*>(_cache)->setto(_en);
          _en.reset();
        }
        break; }
    case DescEntry::Image:
      { const EntryImage& en = static_cast<const EntryImage&>(e);
        EntryImage& _en = static_cast<EntryImage&>(*_entry);
        const DescImage& d = _en.desc();
        if (d.nframes()) {
          int fn;
#ifdef _OPENMP
#pragma omp parallel private(fn) num_threads(4)
          {
#pragma omp for schedule(dynamic,1) nowait
#else
          {
#endif
            for(fn=0; fn<int(d.nframes()); fn++) {
              const SubFrame& f = d.frame(fn);
              for(unsigned j=f.y; j<f.y+f.ny; j++)
                for(unsigned k=f.x; k<f.x+f.nx; k++)
                  _en.addcontent(en.content(k,j),k,j);      
            }
          }
        }
        else {
          int j;
#ifdef _OPENMP
#pragma omp parallel private(j) num_threads(4)
          {
#pragma omp for schedule(dynamic,128) nowait
#else
          {
#endif
            for(j=0; j<int(d.nbinsy()); j++)
              for(unsigned k=0; k<d.nbinsx(); k++)
                _en.addcontent(en.content(k,j),k,j);
          }
        }
        for(unsigned j=0; j<EntryImage::InfoSize; j++) {
          EntryImage::Info i = (EntryImage::Info)j;
          _en.addinfo(en.info(i),i);
        }
        if (_n && _en.info(EntryImage::Normalization)==_n) {
          static_cast<EntryImage*>(_cache)->setto(_en);
          _en.reset();
        }
        break; }
    case DescEntry::Waveform:
      { const EntryWaveform& en = static_cast<const EntryWaveform&>(e);
        EntryWaveform& _en = static_cast<EntryWaveform&>(*_entry);
        _en.valid(e.time());
        for(unsigned k=0; k<en.desc().nbins(); k++)
          _en.addcontent(en.content(k),k);
        for(unsigned j=0; j<EntryWaveform::InfoSize; j++) {
          EntryWaveform::Info i = (EntryWaveform::Info)j;
          _en.addinfo(en.info(i),i);
        }
        if (_n && _en.info(EntryWaveform::Normalization)==_n) {
          static_cast<EntryWaveform*>(_cache)->setto(_en);
          _en.reset();
        }
        break; }
    default:
      break;
    }
  }

  if (_n) {
    return *_cache;
  }
  else {
    return *_entry;
  }
}

