#ifndef AmiSrv_Filter
#define AmiSrv_Filter

namespace Ami {
  namespace Srv {
    class Filter {
    public:
      virtual ~Filter() {}
    public:
      virtual bool process(const Event&) = 0;
    };
  };
};

#endif
