#ifndef AmiQt_FilterSetup_hh
#define AmiQt_FilterSetup_hh

#include "ami/qt/QtPWidget.hh"

class QListWidget;

namespace Ami {
  class ClientManager;
  class DiscoveryRx;

  namespace Qt {
    class FilterSetup : public QtPWidget {
      Q_OBJECT
    public:
      FilterSetup(ClientManager&);
      ~FilterSetup();
    public:
      void     update(const DiscoveryRx&);
      unsigned selected() const;
    public:
      void save(char*& p) const;
      void load(const char*& p);
    public slots:
      void apply();
    private:
      ClientManager& _manager;
      QListWidget*   _list;
    };
  };
};

#endif
