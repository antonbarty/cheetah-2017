#include "FileSelect.hh"

#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>

#include <glob.h>
#include <libgen.h>
#include <string>

using namespace Ami::Qt;

FileSelect::FileSelect(QWidget* parent,
		       const QStringList& paths) :
  QWidget(parent),
  _list  (new QListWidget)
{
  QVBoxLayout* l = new QVBoxLayout;
  l->addWidget(_list);
  setLayout(l);

  connect(_list,  SIGNAL(currentRowChanged(int)),
	  this,   SLOT  (select_run(int)));

  change_path_list(paths);
}

FileSelect::~FileSelect()
{
}

QStringList FileSelect::paths() const
{
  QStringList v;
  if (!_run.isEmpty()) {

    char dbuff[64];
    sprintf(dbuff,"/*-r%s-s*.xtc",qPrintable(_run));

    glob_t g;
    for(QStringList::const_iterator it=_paths.constBegin(); 
	it != _paths.constEnd(); it++) {

      std::string gpath(qPrintable(*it));
      gpath += dbuff;
      printf("Trying %s\n",gpath.c_str());
      glob(gpath.c_str(),0,0,&g);

      for(unsigned i=0; i<g.gl_pathc; i++)
	v << QString(g.gl_pathv[i]);
    }
    globfree(&g);
  }
  return v;
}

void FileSelect::change_path_list(const QStringList& paths)
{
  _paths = paths;

  disconnect(_list, SIGNAL(currentRowChanged(int)),
	     this,  SLOT  (select_run(int)));

  _list->clear();

  char dbuff[64];
  sprintf(dbuff,"/*-r*-s*.xtc");

  QStringList runs;

  for(QStringList::const_iterator it=_paths.constBegin(); it != _paths.constEnd(); it++) {
    glob_t g;
    std::string gpath(qPrintable(*it));
    gpath += dbuff;
    printf("Trying %s\n",gpath.c_str());
    glob(gpath.c_str(),0,0,&g);
    for(unsigned i=0; i<g.gl_pathc; i++) {
      QString s(basename(g.gl_pathv[i]));
      int p = s.indexOf("-r");
      if (p >= 0) {
	QString run = s.mid(p+2,
			    s.indexOf("-s")-p-2);
	if (!runs.contains(run))
	  runs << run;
      }
    }
    globfree(&g);
  }

  runs.sort();
  _list->addItems(runs);
  _run = QString("");

  printf("Found %d runs\n",runs.size());

  connect(_list, SIGNAL(currentRowChanged(int)),
	  this,  SLOT  (select_run(int)));
}

void FileSelect::select_run(int row)
{
  if (row>=0) {
    _run = _list->currentItem()->text();
    emit run_selected();
  }
  else
    _run = QString("");
}
