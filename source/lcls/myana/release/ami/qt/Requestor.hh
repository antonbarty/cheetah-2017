#ifndef AmiQt_Requestor_hh
#define AmiQt_Requestor_hh

namespace Ami {
  namespace Qt {
    class Requestor {
    public:
      virtual ~Requestor() {}
    public:
      virtual void request_payload() = 0;
      virtual void one_shot(bool) = 0;
    };
  };
};

#endif
