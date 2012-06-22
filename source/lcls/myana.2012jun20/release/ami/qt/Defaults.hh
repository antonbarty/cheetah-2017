#ifndef AmiQt_Defaults_hh
#define AmiQt_Defaults_hh

#include "ami/qt/QtPWidget.hh"

class QCheckBox;

namespace Ami {
  namespace Qt {
    class Defaults : public QtPWidget {
    public:
      bool select_run     () const;
      bool show_grid      () const;
      bool show_minor_grid() const;

    public:
      static Defaults* instance();
    private:
      Defaults();
      ~Defaults();
    public:
      void save(char*& p) const;
      void load(const char*& p);
    private:
      QCheckBox* _run;
      QCheckBox* _grid;
      QCheckBox* _minor_grid;
    };
  };
};

#endif
