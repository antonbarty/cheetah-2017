#ifndef AmiQt_QtChannelMath_hh
#define AmiQt_QtChannelMath_hh

#include "ami/qt/QtBase.hh"

#include <list>

namespace Ami {
  namespace Qt {
    class Term;
    class ChannelRef;
    class QtChannelMath : public QtBase {
    public:
      QtChannelMath(QtBase*, const std::list<ChannelRef*>& inputs, Term* t);
      ~QtChannelMath();
    public:
      void        dump  (FILE*) const;
      void        attach(QwtPlot*);
    public:
      void        update()        ;
      void        xscale_update() ;
      void        yscale_update() ;
    private:
      QtBase* _base;
      const std::list<ChannelRef*>& _inputs;
      Term* _term;
    };
  };
};

#endif
