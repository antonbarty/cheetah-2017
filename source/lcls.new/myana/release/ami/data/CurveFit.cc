#include "CurveFit.hh"

#include "ami/data/FeatureCache.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EntryScalar.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/EntryTH1F.hh"
#include "ami/data/EntryProf.hh"
#include "ami/data/EntryScan.hh"
#include "ami/data/EntryCache.hh"
#include "ami/data/EntryWaveform.hh"
#include "ami/data/EntryFactory.hh"

#include "ami/data/Cds.hh"
#include "ami/data/Expression.hh"
#include "ami/data/FeatureExpression.hh"

#include <stdio.h>

using namespace Ami;

int CurveFit::calcnxt = 0;
struct CurveFit::recent CurveFit::calc[CurveFit::CALC_LEN];

CurveFit::CurveFit(const char *name, int op, const DescEntry& output, const char *norm) :
  AbsOperator(AbsOperator::CurveFit),
  _input(0),
  _op(op),
  _entry(0),
  _fterm(0),
  _nterm(0)
{
  strncpy(_name, name, NAME_LEN);
  memcpy (_desc_buffer, &output, output.size());
  if (!norm)
      _norm[0] = 0;
  else
      strncpy(_norm, norm, NORM_LEN);
}

CurveFit::CurveFit(const char*& p, const DescEntry& input, FeatureCache& features) :
    AbsOperator     (AbsOperator::CurveFit),
    _input          (&input),
    _fterm(0)
{
    FILE *fp;
    int cnt;
    double t, d;

    _extract(p, _name, NAME_LEN);
    _extract(p, _desc_buffer, DESC_LEN);
    _extract(p, &_op, sizeof(_op));
    _extract(p, _norm, NORM_LEN);

    const DescEntry& o = *reinterpret_cast<const DescEntry*>(_desc_buffer);

    _entry = EntryFactory::entry(o);

    _refdata.clear();
    _refstart = 0.0;
    _refend = 0.0;
    if (!(fp = fopen(_name, "r")))
        return;
    cnt = 0;
    while (fscanf(fp, "%lg %lg\n", &t, &d) == 2) {
        _refdata.push_back(d);
        if (cnt++)
            _refend = t;
        else
            _refstart = t;
    }
    fclose(fp);
#if 0
    printf("Read text file %s with %d datapoints\n", _name, cnt);
#endif
    if (cnt > 1)
        _refbw = (_refend - _refstart) / (cnt - 1);
    else
        _refbw = 0.0;

    if (_norm[0]) {
        FeatureExpression parser;
        _nterm = parser.evaluate(features,QString(_norm));
        if (!_nterm)
            printf("CurveFit failed to parse f %s\n",_norm);
    } else
        _nterm = 0;

    if (o.type() == DescEntry::Prof ||
        o.type() == DescEntry::Scan) {
        QString expr(o.xtitle());
        FeatureExpression parser;
        _fterm = parser.evaluate(features,expr);
        if (!_fterm)
            printf("CurveFit failed to parse f %s\n",qPrintable(expr));
    }
}

CurveFit::~CurveFit()
{
   if (_entry) delete _entry;
   if (_input) {
       for (int i = 0; i < CALC_LEN; i++)
           if (_input == calc[i].input) {
               calc[i].input = 0;
               break;
           }
   }
}

DescEntry& CurveFit::output   () const 
{
  return _entry ? _entry->desc() : *reinterpret_cast<DescEntry*>(const_cast<char*>(_desc_buffer)); 
}

void*      CurveFit::_serialize(void* p) const
{
  _insert(p, _name , NAME_LEN);
  _insert(p, _desc_buffer, DESC_LEN);
  _insert(p, &_op, sizeof(_op));
  _insert(p, _norm , NORM_LEN);

  return p;
}

Entry&     CurveFit::_operate(const Entry& e) const
{
    if (!e.valid())
        return *_entry;

    Pds::ClockTime now = e.time();
    int slot;
    double x, y = 0.0;
    bool damaged = false;

    // Look for our cache slot.
    for (slot = 0; slot < CALC_LEN; slot++)
        if (calc[slot].input == _input)
            break;
    if (slot == CALC_LEN || !(calc[slot].time == now)) {
        /*
         * We don't have a slot, or the time has passed.  If we don't have a slot,
         * get one.
         */
        if (slot == CALC_LEN) {
            slot = calcnxt++;
            if (calcnxt == CALC_LEN)
                calcnxt = 0;
        }

        const EntryWaveform& entry = static_cast<const EntryWaveform&>(e);
        const DescWaveform& d = entry.desc();
        double binw = (d.xup() - d.xlow()) / d.nbins();

        /* Do we want to compare binw to _refbw? */

        int fs = _refdata.size();
        int xs = d.nbins();
        double f2, x2, xvar;
        double Lmin, cmin;
        int omin, i;

        for (f2 = 0.0, i = 0; i < fs; i++)
            f2 += _refdata[i] * _refdata[i];

        for (xvar = 0.0, x2 = 0.0, i = 0; i < xs; i++) {
            double x = entry.content(i);
            x2 += x * x;
            xvar += x;
        }
#if 0
        printf("avg = %lg, variance = %lg, stddev = %lg\n", xvar/xs, 
               (x2 - xvar * xvar / xs)/(xs - 1),
               sqrt((x2 - xvar * xvar / xs)/(xs - 1)));
#endif
        xvar = (x2 - xvar * xvar / xs)/(xs - 1); /* Might not be numerically stable. */

        /* Trivial, no overlap case! */
        omin = -fs;
        cmin = 0.0;
        Lmin = x2;

        /* Iterate over all possible overlaps. */
        for (i = -fs + 1; i < xs; i++) {
            double xf = 0.0;
            for (int j = (i < 0) ? -i : 0; j < fs; j++)
                if (i + j >= xs)
                    break;
                else
                    xf += entry.content(i+j) * _refdata[j];
            double c = xf / f2;
            double L = x2 - 2 * c * xf + c * c * f2;
#if 0
            printf("%5d: c=%12.8lf, L=%12.8lf%s\n", i, c, L, L < Lmin ? " *" : "");
#endif
            if (L < Lmin) {
                omin = i;
                cmin = c;
                Lmin = L;
            }
        }

#if 0
        printf("omin = %d, cmin=%lg, Lmin=%lg, xvar=%lg, chi2=%lg\n", omin, cmin, Lmin, xvar, Lmin/xvar);
#endif
        calc[slot].input = _input;
        calc[slot].time = now;
        calc[slot].vals[scale] = cmin;
        calc[slot].vals[shift] = omin * binw - _refstart;
        calc[slot].vals[chi2]  = Lmin / xvar;
#if 0
        printf("Wrote %d for %p:%d:%d op=%d\n", slot, _input, now.seconds(), now.nanoseconds(), _op);
#endif
    } else {
#if 0
        printf("Hit %d for %p:%d:%d op=%d\n", slot, _input, now.seconds(), now.nanoseconds(), _op);
#endif
    }

    /* Get the cached value */
    y = calc[slot].vals[_op];

    /* Normalize it */
    if (_nterm)
        y = y / _nterm->evaluate();

    /* Send it on its way! */
    switch(_entry->desc().type()) {
    case DescEntry::Scalar:  
        { EntryScalar* en = static_cast<EntryScalar*>(_entry);
            en->addcontent(y);
            break; }
    case DescEntry::TH1F:
        { EntryTH1F* en = static_cast<EntryTH1F*  >(_entry);
            en->addcontent(1.,y); 
            en->addinfo(1.,EntryTH1F::Normalization);
            break; }
    case DescEntry::Prof:
        if (!_fterm)
            return *_entry;
        x = _fterm->evaluate();
        if (!damaged) {
            EntryProf *ep = static_cast<EntryProf*>(_entry);
            ep->addy(y, x);
            ep->addinfo(1., EntryProf::Normalization);
        }
        break;
    case DescEntry::Scan:    
        if (!_fterm)
            return *_entry;
        x = _fterm->evaluate();
        if (!damaged) {
            EntryScan *es = static_cast<EntryScan*>(_entry);
            es->addy(y, x);
            es->addinfo(1., EntryScan::Normalization);
        }
        break;
    case DescEntry::Cache:
        { EntryCache* en = static_cast<EntryCache*>(_entry);
            en->set(y,false);
            break; }
    case DescEntry::Waveform:
    case DescEntry::TH2F:
    case DescEntry::Image:
    default:
        printf("CurvFit::_operator no implementation for type %d\n",_entry->desc().type());
        break;
    }
    _entry->valid(now);
    return *_entry;
}
