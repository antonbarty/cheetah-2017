#ifndef AmiQt_Filter_hh
#define AmiQt_Filter_hh

#include "ami/qt/QtPWidget.hh"

class QComboBox;
class QButtonGroup;
class QLineEdit;
class QVBoxLayout;

namespace Ami {
  class AbsFilter;

  namespace Qt {
    class Condition;
    class FeatureTree;
    class Filter : public QtPWidget {
      Q_OBJECT
    public:
      Filter(QWidget* parent,const QString&);
      ~Filter();
    public:
      void save(char*& p) const;
      void load(const char*& p);
    public:
      const Ami::AbsFilter* filter() const;
    public slots:
      void add   ();
      void remove(const QString&);
      void calc  ();
      void apply ();
      void clear ();
    signals:
      void changed();
    private:
      void _apply();
    private:
      QString    _name;
      QLineEdit* _expr;
      QLineEdit* _cond_name;
      QComboBox* _bld_box;
      FeatureTree* _features;
      QLineEdit* _lo_rng;
      QLineEdit* _hi_rng;
      QVBoxLayout* _clayout;
      std::list<Condition*> _conditions;

      AbsFilter* _filter;
    };
  };
};

#endif
