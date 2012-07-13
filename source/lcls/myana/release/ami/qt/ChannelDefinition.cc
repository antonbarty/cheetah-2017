#include "ChannelDefinition.hh"

#include "ami/qt/Display.hh"
#include "ami/qt/ChannelMath.hh"
#include "ami/qt/Filter.hh"
#include "ami/qt/Transform.hh"
#include "ami/qt/PlotFactory.hh"
#include "ami/qt/Path.hh"
#include "ami/qt/FeatureCalculator.hh"
#include "ami/qt/FeatureRegistry.hh"

#include "ami/data/AbsOperator.hh"
#include "ami/data/Reference.hh"
#include "ami/data/Average.hh"
#include "ami/data/Variance.hh"
#include "ami/data/Single.hh"
#include "ami/data/Cds.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/DescEntry.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EntryFactory.hh"
#include "ami/data/EntryRefOp.hh"

#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QButtonGroup>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QIntValidator>
#include <QtGui/QComboBox>


#define bold(t) #t

using namespace Ami::Qt;

enum { _None, _Single, _Average, _Variance, _Math, _Reference };
enum { REFERENCE_BUFFER_SIZE = 0x1000 };

static const unsigned NOT_INIT = (unsigned)-1;

ChannelDefinition::ChannelDefinition(QWidget* parent,
				     const QString& name, 
				     const QStringList& names,
				     Display& frame, 
				     const QColor& color, 
				     bool init,
                                     QStringList refnames) :
  QtPWidget        (parent),
  _name            (name),
  _frame           (frame),
  _color           (color),
  _filter          (new Filter   (this,name)),
  _operator        (0),
  _transform       (new Transform(this,QString("%1 : Y Transform").arg(name),"y")),
  _math            (new ChannelMath(names)),
  _interval        (new QLineEdit),
  _output_signature(NOT_INIT),
  _changed         (false),
  _show            (false),
  _plot            (0),
  _scale           (new QLineEdit),
  _operator_is_ref (false),
  _configured_ref  (false)
{
  setWindowTitle(_name);
  setAttribute(::Qt::WA_DeleteOnClose, false);

  _plot_grp = new QButtonGroup;
  QRadioButton* noneB     = new QRadioButton(bold(None));  
  QRadioButton* singleB   = new QRadioButton(bold(Single));  
  QRadioButton* averageB  = new QRadioButton(bold(Averaged));
  QRadioButton* varianceB = new QRadioButton(bold(Variance));
  QRadioButton* mathB     = new QRadioButton(bold(Math));
  QRadioButton* refB      = new QRadioButton(bold(Reference));
  _plot_grp->addButton(noneB    , _None); 
  _plot_grp->addButton(singleB  , _Single); 
  _plot_grp->addButton(averageB , _Average);
  _plot_grp->addButton(varianceB, _Variance);
  _plot_grp->addButton(mathB    , _Math);
  _plot_grp->addButton(refB     , _Reference);
  refB->setEnabled(false);

  new QIntValidator(_interval);

  QPushButton* loadB = new QPushButton("Load");

  QPushButton* filterB  = new QPushButton("Filter");
  QPushButton* yfuncB   = new QPushButton("Y Transform");
  QPushButton* applyB   = new QPushButton("OK");
  QPushButton* closeB   = new QPushButton("Close");
  QPushButton* scaleB   = new QPushButton("Enter");

  QVBoxLayout* l = new QVBoxLayout;
  { if (!refnames.empty()) {
      _refBox = new QComboBox;
      _refBox->addItems(refnames);
      _refBox->setCurrentIndex(0);
      l->addWidget(_refBox);
    }
    else {
      _refBox = 0;
    }
    QGroupBox* plot_box = new QGroupBox("Plot");
    QVBoxLayout* layout = new QVBoxLayout;
    { QHBoxLayout* layout1 = new QHBoxLayout;
      layout1->addWidget(new QLabel("Normalize to"));
      layout1->addWidget(_scale);
      layout1->addWidget(scaleB);
      layout->addLayout(layout1); }
    layout->addWidget(noneB);
    layout->addWidget(singleB);
    { QGridLayout* layout1 = new QGridLayout;
      layout1->addWidget(averageB ,0,0);
      layout1->addWidget(varianceB,1,0);
      layout1->addWidget(new QLabel("Events"),0,1,2,1);
      layout1->addWidget(_interval,0,2,2,1);
      layout->addLayout(layout1); }
    { QHBoxLayout* layout1 = new QHBoxLayout;
      layout1->addWidget(mathB);
      layout1->addStretch();
      layout1->addWidget(_math);
      layout->addLayout(layout1); }
    { QHBoxLayout* layout1 = new QHBoxLayout;
      layout1->addWidget(refB);
      layout1->addWidget(loadB);
      layout->addLayout(layout1); }
    plot_box->setLayout(layout);
    l->addWidget(plot_box); }
  { QHBoxLayout* layout = new QHBoxLayout;
    layout->addStretch();
    layout->addWidget(filterB);
    layout->addWidget(yfuncB);
    layout->addStretch();
    l->addLayout(layout); }
  { QHBoxLayout* layout = new QHBoxLayout;
    layout->addStretch();
    layout->addWidget(applyB);
    layout->addWidget(closeB);
    layout->addStretch(); 
    l->addLayout(layout); }
  setLayout(l);

  connect(this    , SIGNAL(reference_loaded(bool)), refB, SLOT(setEnabled(bool)));
  
  connect(loadB   , SIGNAL(clicked()), this, SLOT(load_reference()));
  connect(filterB , SIGNAL(clicked()), _filter, SLOT(show()));
  connect(yfuncB  , SIGNAL(clicked()), _transform , SLOT(show()));
  connect(applyB  , SIGNAL(clicked()), this, SLOT(apply()));
  connect(_filter , SIGNAL(changed()), this, SLOT(apply()));
  connect(closeB  , SIGNAL(clicked()), this, SLOT(hide()));
  connect(scaleB  , SIGNAL(clicked()), this, SLOT(set_scale()));

  noneB  ->setChecked(!init);
  singleB->setChecked(init);
  apply();
  show_plot(init);
}
	  
ChannelDefinition::~ChannelDefinition()
{
}

void ChannelDefinition::save(char*& p) const
{
  XML_insert( p, "QtPWidget", "self", QtPWidget::save(p) );

  XML_insert( p, "QButtonGroup", "_plot_grp", QtPersistent::insert(p,_plot_grp->checkedId()) );
  XML_insert( p, "QLineEdit"   , "_interval", QtPersistent::insert(p,_interval->text()) );
  XML_insert( p, "ChannelMath" , "_math"    , QtPersistent::insert(p,_math->expr()) );
  XML_insert( p, "QString"     , "_ref_file", QtPersistent::insert(p,_plot_grp->button(_Reference)->isEnabled() ? _ref_file : QString("")) );
  XML_insert( p, "bool"        , "_show"    , QtPersistent::insert(p,_show) );
  XML_insert( p, "QComboBox"   , "_refBox"  , QtPersistent::insert(p,_refBox ? _refBox->currentIndex() : -1) );
  XML_insert( p, "Filter"      , "_filter"  , _filter   ->save(p) );
  XML_insert( p, "Transform"   , "_transform",_transform->save(p) );
  XML_insert( p, "QLineEdit"   , "_scale"   , QtPersistent::insert(p,_scale->text()) );
}

void ChannelDefinition::load(const char*& p)
{
  int  id   = 0;
  bool show = false;

  XML_iterate_open(p,tag)
    if      (tag.element == "QtPWidget")
      QtPWidget::load(p);
    else if (tag.name == "_plot_grp")
      id = QtPersistent::extract_i(p);
    else if (tag.name == "_interval")
      _interval->setText(QtPersistent::extract_s(p));
    else if (tag.name == "_math")
      _math->expr(QtPersistent::extract_s(p));
    else if (tag.name == "_ref_file") {
      QString rfile = QtPersistent::extract_s(p);
      if (!rfile.isEmpty()) {
        _ref_file = rfile;
        _plot_grp->button(_Reference)->setEnabled(true);
      }
      else
        _plot_grp->button(_Reference)->setEnabled(false);
    }
    else if (tag.name == "_show")
      show = QtPersistent::extract_b(p);
    else if (tag.name == "_refBox") {
      int index = QtPersistent::extract_i(p);
      if (_refBox && index >= 0)
        _refBox->setCurrentIndex(index);
    }
    else if (tag.name == "_filter")
      _filter   ->load(p);
    else if (tag.name == "_transform")
      _transform->load(p);
    else if (tag.name == "_scale")
      _scale    ->setText(QtPersistent::extract_s(p));
  XML_iterate_close(ChannelDefinition,tag);

  _plot_grp->button(id)->setChecked(true);

  apply();

  emit show_plot_changed(show);
}

void ChannelDefinition::load_reference()
{
  QString file = Path::loadReferenceFile(_name);

  if (file.isNull()) {
    printf("load_reference file is null\n");
    return;
  }
  
  _ref_file = file;
  _plot_grp->button(_Reference)->setEnabled(true);
  _plot_grp->button(_Reference)->setChecked(true);
  apply();
}
	  
void ChannelDefinition::show_plot(bool s) 
{
  //  This should be a slot on the display (QwtPlot)
  if (s != _show && _plot!=0) {
    if (s) _frame.show(_plot);
    else   _frame.hide(_plot);
  }
  _show = s;
}

/**
 **  Prepend operator with channel selection
 */
void ChannelDefinition::apply()
{
  // Form _operator
  if (_operator) delete _operator;

  const char* scale = qPrintable(_scale->text());

  switch(_mode = _plot_grp->checkedId()) {
  case _Single:
    if (_refBox) {
      _operator = new EntryRefOp(_refBox->currentIndex());
      _operator->next(new Single(scale));
    }
    else
      _operator = new Single(scale);
    _operator_is_ref  = false;
    break;
  case _Average     : 
    if (_refBox) {
      _operator = new EntryRefOp(_refBox->currentIndex());
      _operator->next(new Average(_interval->text().toInt(),scale));
    }
    else
      _operator = new Average(_interval->text().toInt(),scale);
    _operator_is_ref  = false;
    break;
  case _Variance    : 
    if (_refBox) {
      _operator = new EntryRefOp(_refBox->currentIndex());
      _operator->next(new Variance(_interval->text().toInt(),scale));
    }
    else
      _operator = new Variance(_interval->text().toInt(),scale);
    _operator_is_ref  = false;
    break;
  case _Reference:
    _operator = new Reference(qPrintable(_ref_file));
    _operator_is_ref  = true;
    break;
  case _Math:
  default:
    //  Don't request anything from the server?
    _operator = 0;
    _operator_is_ref  = false;
    break;
  }

  _changed = true;
  emit changed();

  emit newplot(_mode!=_None);
}

int ChannelDefinition::configure(char*& p, unsigned input, unsigned& output,
				 ChannelDefinition* channels[], int* signatures, int nchannels,
				 ConfigureRequest::Source source)
{
  //  if (_mode==_None) 
  //    ;
  //  else if (_mode==_Math) {
  if (_mode==_Math) {
    if (!_math->resolve(channels, signatures, nchannels, *_filter->filter())) {
      printf("ChannelDefinition::_math resolve failed\n");
      return -1;
    }

    _output_signature = ++output;

    ConfigureRequest* r;
    if (_refBox) {
      _operator = new EntryRefOp(_refBox->currentIndex());
      _operator->next(&_math->op());
      r = new (p) ConfigureRequest(ConfigureRequest::Create,
                                   source,
                                   input,
                                   _output_signature,
                                   _math->filter(),
                                   *_operator);
      _operator->next(0);
    }
    else {
      r = new (p) ConfigureRequest(ConfigureRequest::Create,
                                   source,
                                   input,
                                   _output_signature,
                                   _math->filter(),
                                   _math->op());
    }
    p += r->size();
    _configured_ref = false;
    return _output_signature;
  }
  else if (_operator) {
    _output_signature = ++output;
    ConfigureRequest& r = *new (p) ConfigureRequest(ConfigureRequest::Create,
						    source,
						    input,
						    _output_signature,
						    *_filter->filter(),
						    *_operator);
    p += r.size();
    _configured_ref = _operator_is_ref;
    return _output_signature;
  }
  return -1;
}


Ami::AbsTransform& ChannelDefinition::transform()
{
  return *_transform;
}

void ChannelDefinition::setup_payload(Cds& cds)
{
  Entry* entry = cds.entry(_output_signature);
  if (entry) {
    _plot=PlotFactory::plot(_name,*entry,_frame.xtransform(),transform(),_color);
    _frame.add(_plot, _show);
    /*
     * OK, a little bit of weirdness here.  If we read in .dat files as references,
     * they look different than the other waveforms and screw everything up.  But
     * if we just ignore the non-zero start points, we screw up the projection
     * waveforms.  Therefore, we keep track if we are a Reference, and if this
     * is the response to configuring a Reference, we ignore it here.
     *
     * If someone only displays unchanging References in a window, they deserve what
     * they are going to get!
     */
    if (_show && !_configured_ref) 
      _frame.prototype(&entry->desc());
  }
  else if (_output_signature!=NOT_INIT)
    printf("%s output_signature %d not found\n",qPrintable(_name),_output_signature);
}

void ChannelDefinition::set_scale()
{
  FeatureCalculator* c = new FeatureCalculator("%1 : Scale", FeatureRegistry::instance());
  if (c->exec()==QDialog::Accepted) {
    _scale->setText(c->result());
  }
  delete c;
}

unsigned  ChannelDefinition::output_signature() const { return _output_signature; }
