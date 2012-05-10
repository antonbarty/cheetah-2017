#ifndef SplitEventQ_hh
#define SplitEventQ_hh

#include <list>

namespace Pds { class Dgram; };

class SplitEventQ {
public:
  SplitEventQ(unsigned depth);
  ~SplitEventQ();
public:
  void        clear();
  bool        cache(Pds::Dgram*);
  std::list<Pds::Dgram*>& queue();
private:
  unsigned _depth;
  unsigned _completed, _cached, _aborted;
  std::list<Pds::Dgram*> _q;
};

#endif
