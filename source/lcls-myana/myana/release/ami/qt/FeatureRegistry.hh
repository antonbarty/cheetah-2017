#ifndef AmiQt_FeatureRegistry_hh
#define AmiQt_FeatureRegistry_hh

#include <QtCore/QObject>

#include <QtCore/QStringList>

#include "ami/data/FeatureCache.hh"
#include "ami/service/Semaphore.hh"

#include <list>
#include <string>

namespace Ami {
  class DiscoveryRx;

  namespace Qt {
    class FeatureRegistry : public QObject {
      Q_OBJECT
    public:
      static FeatureRegistry& instance(Ami::ScalarSet t=Ami::PreAnalysis);
    public:
      void               insert  (const std::vector<std::string>&);
    public:
      QStringList        names   () const;
      const QStringList& help    () const;
      int                index   (const QString&) const;
    public:
      void               force   ();
    signals:
      void changed();  // discovered a change
    private:
      FeatureRegistry();
      ~FeatureRegistry();
    private:
      QStringList _names;
      QStringList _help;
      mutable Ami::Semaphore _sem;
    };
  };
};

#endif
