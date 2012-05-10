#include "Average.hh"

#include "ami/data/DescEntry.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/EntryWaveform.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryProf.hh"
#include "ami/data/EntryFactory.hh"
#include "ami/data/FeatureExpression.hh"

#include <QtCore/QString>

#include <stdio.h>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace Ami;

Average::Average(unsigned n, const char* scale) : 
  AbsOperator(AbsOperator::Average),
  _n         (n),
  _entry     (0),
  _cache     (0),
  _term      (0)
{
  if (scale)
    strncpy(_scale_buffer,scale,SCALE_LEN);
  else
    memset(_scale_buffer,0,SCALE_LEN);
}

Average::Average(const char*& p, const DescEntry& e, FeatureCache& features) :
  AbsOperator(AbsOperator::Average)
{
  _extract(p,_scale_buffer, SCALE_LEN);
  _extract(p, &_n, sizeof(_n));

  _entry = EntryFactory::entry(e);
  _entry->reset();
  if (_n) {
    _cache = EntryFactory::entry(e);
    _cache->desc().aggregate(false);
  }
  else
    _cache = (Entry*)0;

  if (_scale_buffer[0]) {
    QString expr(_scale_buffer);
    FeatureExpression parser;
    _term = parser.evaluate(features,expr);
    if (!_term)
      printf("BinMath failed to parse f %s\n",qPrintable(expr));
  }
  else
    _term = 0;
}

Average::~Average()
{
  if (_entry) delete _entry;
  if (_cache) delete _cache;
  if (_term ) delete _term ;
}

DescEntry& Average::output   () const { return _n ? _cache->desc() : _entry->desc(); }

void*      Average::_serialize(void* p) const
{
  _insert(p, _scale_buffer , SCALE_LEN);
  _insert(p, &_n, sizeof(_n));
  return p;
}

Entry&     Average::_operate(const Entry& e) const
{
  if (e.valid()) {

    const double v = _term ? _term->evaluate() : 1;

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
        const double ped = en.info(EntryImage::Pedestal);
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
                if (_term)
                  for(unsigned k=f.x; k<f.x+f.nx; k++)
                    _en.addcontent(unsigned(ped + (en.content(k,j)-ped)/v),k,j);      
                else 
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
                if (_term)
                  _en.addcontent(unsigned(rint(double(en.content(k,j)/v))),k,j);      
                else
                  _en.addcontent(en.content(k,j),k,j);
          }
        }
        for(unsigned j=0; j<EntryImage::InfoSize; j++) {
          EntryImage::Info i = (EntryImage::Info)j;
          if (i == EntryImage::Pedestal)
            _en.addinfo(en.info(i)/v,i);
          else
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
        if (_term)
          for(unsigned k=0; k<en.desc().nbins(); k++)
            _en.addcontent(en.content(k)/v,k);
        else
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

