#ifndef Ami_ConfigureRequest_hh
#define Ami_ConfigureRequest_hh

#include "ami/data/AbsFilter.hh"
#include "ami/data/AbsOperator.hh"

namespace Ami {

  class AbsFilter;
  class AbsOperator;

  class ConfigureRequest {
  public:
    enum State { Create, Destroy, SetOpt };
    enum Source { Discovery, Analysis, Summary, User, Filter, Hidden };
    ConfigureRequest(State        state, 
		     Source       source,
		     int          input,  // signature
		     int          output, // signature
		     const AbsFilter&   filter,
		     const AbsOperator& op) :
      _state(state), _source(source), _input(input), _output(output)
    {
      char* e = (char*)op.serialize(filter.serialize(this+1));
      _size = e - (char*)this;
    }
    ConfigureRequest(State        state,
		     Source       source) : 
      _state(state), _source(source), _input(-1), _output(-1), _size(sizeof(*this))
    {
    }
    ConfigureRequest(int          input,
                     unsigned     options) :
      _state (SetOpt),
      _source(Discovery),
      _input (input),
      _output(-1),
      _size  (sizeof(*this)+sizeof(uint32_t))
    {
      *reinterpret_cast<uint32_t*>(this+1)=options;
    }
    ConfigureRequest(Source       source,
                     unsigned     options) :
      _state (SetOpt),
      _source(source),
      _input (-1),
      _output(-1),
      _size  (sizeof(*this)+sizeof(uint32_t))
    {
      *reinterpret_cast<uint32_t*>(this+1)=options;
    }
  public:
    State  state () const { return State (_state); }
    Source source() const { return Source(_source); }
    int    input () const { return _input; }
    int    output() const { return _output; }
    int    size  () const { return _size; }
  private:
    uint32_t _state;
    uint32_t _source;
    int32_t  _input;
    int32_t  _output;
    int32_t  _size;
  };

};

#endif
