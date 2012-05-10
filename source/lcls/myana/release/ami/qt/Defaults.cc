#include "ami/qt/Defaults.hh"

#include <QtGui/QVBoxLayout>
#include <QtGui/QCheckBox>

using namespace Ami::Qt;

Defaults::Defaults() 
{
  setWindowTitle("Defaults");

  setAttribute(::Qt::WA_DeleteOnClose, false);

  _run        = new QCheckBox("Select 'Run'"   );  _run       ->setChecked(true);
  _grid       = new QCheckBox("Show Grid"      );  _grid      ->setChecked(false);
  _minor_grid = new QCheckBox("Show Minor Grid");  _minor_grid->setChecked(false);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget( _run );
  layout->addWidget( _grid       );
  layout->addWidget( _minor_grid );

  setLayout(layout);
}

Defaults::~Defaults()
{
}

bool Defaults::select_run     () const { return _run ->isChecked      (); }
bool Defaults::show_grid      () const { return _grid->isChecked      (); }
bool Defaults::show_minor_grid() const { return _minor_grid->isChecked(); }


void Defaults::save(char*& p) const
{
}

void Defaults::load(const char*& p)
{
}

static Defaults* _instance = 0;

Defaults* Defaults::instance()
{
  if (!_instance)
    _instance = new Defaults;

  return _instance;
}

