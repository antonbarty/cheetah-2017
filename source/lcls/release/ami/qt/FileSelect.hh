#ifndef AmiQt_FileSelect_hh
#define AmiQt_FileSelect_hh

#include <QtGui/QWidget>

class QString;
class QListWidget;
class QPushButton;

namespace Ami {
  namespace Qt {
    class FileSelect : public QWidget {
      Q_OBJECT
    public:
      FileSelect(QWidget* parent,
		 const QStringList& paths);
      ~FileSelect();
    public:
      QStringList paths() const;
    public slots:
      void change_path_list(const QStringList&);
      void select_run(int);
    signals:
      void run_selected();
    private:
      QStringList  _paths;
      QString      _run;
      QListWidget* _list;
    };
  };
};

#endif
