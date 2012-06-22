#include "Variance.hh"

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

Variance::Variance(unsigned n, const char* scale) : 
  AbsOperator(AbsOperator::Variance),
  _n         (n),
  _mom1      (0),
  _mom2      (0),
  _cache     (0),
  _term      (0)
{
  if (scale)
    strncpy(_scale_buffer,scale,SCALE_LEN);
  else
    memset(_scale_buffer,0,SCALE_LEN);
}

Variance::Variance(const char*& p, const DescEntry& e, FeatureCache& features) :
  AbsOperator(AbsOperator::Variance)
{
  _extract(p,_scale_buffer, SCALE_LEN);
  _extract(p, &_n, sizeof(_n));

  _mom1 = EntryFactory::entry(e);
  _mom1->reset();

  _mom2 = EntryFactory::entry(e);
  _mom2->reset();

  _cache = EntryFactory::entry(e);
  _cache->reset();

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

Variance::~Variance()
{
  if (_mom1) delete _mom1;
  if (_mom2) delete _mom2;
  if (_cache) delete _cache;
  if (_term ) delete _term ;
}

DescEntry& Variance::output   () const { return _cache->desc(); }

void*      Variance::_serialize(void* p) const
{
  _insert(p, _scale_buffer , SCALE_LEN);
  _insert(p, &_n, sizeof(_n));
  return p;
}

#define HANDLE_1D(type) {                                               \
    Entry##type& _m1 = static_cast<Entry##type&>(*_mom1);               \
    Entry##type& _m2 = static_cast<Entry##type&>(*_mom2);               \
    for(unsigned i=0; i<_m1.desc().nbins(); i++) {                      \
      double v = static_cast<const Entry##type&>(e).content(i);         \
      _m1.addcontent(v,i);                                              \
      _m2.addcontent(v*v,i);                                            \
    }                                                                   \
    _m1.addinfo(1.,Entry##type::Normalization);                 \
    if (_m1.info(Entry##type::Normalization)>=_n) {             \
      Entry##type& cache = *static_cast<Entry##type*>(_cache);  \
      double s = 1./_m1.info(Entry##type::Normalization);       \
      for(unsigned i=0; i<_m1.desc().nbins(); i++) {            \
        double m = s*_m1.content(i);                            \
        double y = sqrt(s*_m2.content(i)-m*m);                  \
        cache.content(y,i);                                     \
      }                                                         \
      cache.info(1.,Entry##type::Normalization);                \
      if (_n) {                                                 \
        _m1.reset();                                            \
        _m2.reset();                                            \
      }                                                         \
    }                                                           \
    break; }                                                    \
    
Entry&     Variance::_operate(const Entry& e) const
{
  if (e.valid()) {

    const double v = _term ? _term->evaluate() : 1;

    switch(e.desc().type()) {
    case DescEntry::TH1F    : HANDLE_1D(TH1F);
    case DescEntry::Waveform: HANDLE_1D(Waveform);
      //    case DescEntry::Prof    : HANDLE_1D(Prof);
    case DescEntry::TH2F:
      printf("Variance TH2F not implemented\n");
      break;
    case DescEntry::Image:
      { const EntryImage& en = static_cast<const EntryImage&>(e);
        EntryImage& _m1 = static_cast<EntryImage&>(*_mom1);
        EntryImage& _m2 = static_cast<EntryImage&>(*_mom2);
        const DescImage& d = _m1.desc();
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
                for(unsigned k=f.x; k<f.x+f.nx; k++) {
                  double y = en.content(k,j)-ped;
                  _m1.addcontent(unsigned(y+0.5),k,j);      
                  _m2.addcontent(unsigned(y*y+0.5),k,j);      
                }
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
              for(unsigned k=0; k<d.nbinsx(); k++) {
                double y = en.content(k,j)-ped;
                _m1.addcontent(unsigned(y+0.5),k,j);
                _m2.addcontent(unsigned(y*y+0.5),k,j);
              }
          }
          
        }

        _m1.addinfo(1.,EntryImage::Normalization);

        if (_m1.info(EntryImage::Normalization)>=_n) {
          EntryImage* cache = static_cast<EntryImage*>(_cache);
          double s = 1./_m1.info(EntryImage::Normalization);
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
                  for(unsigned k=f.x; k<f.x+f.nx; k++) {
                    double m = s*_m1.content(k,j);
                    double y = sqrt(s*_m2.content(k,j)-m*m);
                    cache->content(unsigned(y+0.5),k,j);
                  }
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
                for(unsigned k=0; k<d.nbinsx(); k++) {
                  double m = s*_m1.content(k,j);
                  double y = sqrt(s*_m2.content(k,j)-m*m);
                  cache->content(unsigned(y+0.5),k,j);
                }
            }
          }
          cache->info(1.,EntryImage::Normalization);
          cache->valid(e.time());
          if (_n) {
            _m1.reset();
            _m2.reset();
          }
        }
        break; }
    default:
      break;
    }
  }

  return *_cache;
}

