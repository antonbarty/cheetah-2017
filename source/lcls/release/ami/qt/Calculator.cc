#include "Calculator.hh"
#include "QHComboBox.hh"
#include "FeatureTree.hh"

#include <QtGui/QLineEdit>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QGridLayout>
#include <QtGui/QToolButton>
#include <QtGui/QPushButton>
#include <QtGui/QListView>
#include <QtGui/QHelpEvent>
#include <QtGui/QToolTip>

#include <math.h>

#include "Calculator.hh"

namespace Ami {
  namespace Qt {
    class CalculatorButton : public QToolButton {
    public:
      CalculatorButton(const QString &text, 
		       const QString &help,
		       const QColor &color,
		       QWidget *parent = 0) :
	QToolButton(parent),
	_help      (help)
      {
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	setText(text);
	
	QPalette newPalette = palette();
	newPalette.setColor(QPalette::Button, color);
	setPalette(newPalette);
      }
      ~CalculatorButton() {}
    public:
      QSize sizeHint() const {
	QSize size = QToolButton::sizeHint();
	size.rheight() += 10;
	size.rwidth() = qMax(size.width(), size.height());
	return size;
      }
      QString text() const { 
	QString t = QToolButton::text();
	t.replace("&&","&");
	return t;
      }
      bool event(QEvent* event)
      {
	if (event->type() == QEvent::ToolTip) {
 	  if (!_help.isEmpty()) {
 	    QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
 	    const QPoint& p = helpEvent->globalPos();
 	    QToolTip::showText(p, _help);
 	  }
 	  else
 	    QToolTip::hideText();
	}
	return QToolButton::event(event);
      }
    private:
      QString _help;
    };
  };
};

using namespace Ami::Qt;


static unsigned lengthAtBegin(const QStringList& list, const QString& str)
{
  for(QStringList::const_iterator it = list.constBegin(); 
      it!=list.constEnd(); it++)
    if (str.startsWith(*it))
      return it->size();
  return 0;
}

static unsigned lengthAtEnd(const QStringList& list, const QString& str)
{
  for(QStringList::const_iterator it = list.constBegin(); 
      it!=list.constEnd(); it++)
    if (str.endsWith(*it))
      return it->size();
  return 0;
}

static unsigned numberAtEnd(const QString& str)
{
  QRegExp digits("[0-9\\.]*");
  QString rev;
  for(int k=0; k<str.size(); k++)
    rev.push_front(str[k]);
  int index = digits.indexIn(rev);
  int len   = digits.matchedLength();
  return index==0 ? len : 0;
}


Calculator::Calculator(const QString&     title,
		       const QString&     reset,
		       const QStringList& variables,
                       const QStringList& var_var_ops,
                       const QStringList& var_con_ops,
		       const QStringList& variables_help) :
  QDialog (0),
  _reset  (reset),
  _variables(variables),
  _varvarops(var_var_ops),
  _varconops(var_con_ops),
  _display  (new QLineEdit(""))
{
  setWindowTitle(title);

  _display->setText(reset);
  _display->setReadOnly(true);
  _display->setAlignment(::Qt::AlignRight);
//     display->setMaxLength(15);
//
//   QFont font = display->font();
//   font.setPointSize(font.pointSize() + 8);
//   display->setFont(font);

  QColor backspaceColor(225, 185, 135);
  QColor operatorColor(155, 175, 195);
  QColor digitColor(150, 205, 205);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(_display);

  QGridLayout *buttonLayout = new QGridLayout;
  buttonLayout->setSizeConstraint(QLayout::SetFixedSize);

  buttonLayout->addWidget(createButton(tr("DEL"), backspaceColor, SLOT(backspaceClicked()))   , 0, 0, 1, 2);
  buttonLayout->addWidget(createButton(tr("CLR")    , backspaceColor, SLOT(clear()))              , 0, 2, 1, 2);
  buttonLayout->addWidget(createButton(tr("RST"), backspaceColor.light(120), SLOT(clearAll())), 0, 4, 1, 2);

  int row=1;
  if (_varconops.size()) {
    // digits in the middle
    const int NumDigitButtons=10;
    for (int i = 1; i < NumDigitButtons; ++i) {
      row = ((9 - i) / 3) + 1;
      int column = ((i - 1) % 3) + 1;
      buttonLayout->addWidget(createButton(QString::number(i), digitColor, SLOT(digitClicked())), row, column);
    }
    buttonLayout->addWidget(createButton(tr("0")   , digitColor, SLOT(digitClicked())), 4, 1);
    buttonLayout->addWidget(createButton(tr(".")   , digitColor, SLOT(pointClicked())), 4, 2);
    buttonLayout->addWidget(createButton(tr("()")  , operatorColor, SLOT(parenthesesClicked())), 4, 3);
    //  buttonLayout->addWidget(createButton(tr("\261"), digitColor, SLOT(changeSignClicked())), 5, 3);
  }

  // operators on the right
  row=1;
  for(QStringList::const_iterator it = var_var_ops.constBegin();
      it!=var_var_ops.constEnd(); it++)
    buttonLayout->addWidget(createButton(*it, operatorColor, SLOT(varvarClicked())),
			    row++,4);
  
  row=1;
  for(QStringList::const_iterator it = var_con_ops.constBegin(); 
      it!=var_con_ops.constEnd(); it++)
    buttonLayout->addWidget(createButton(*it, operatorColor, SLOT(varconClicked())),
			  row++,5);

  const int max_calc_row = 25;
  int max_row = 4;
  if (max_row < var_var_ops.size()) max_row = var_var_ops.size();
  if (max_row < var_con_ops.size()) max_row = var_con_ops.size();

  if (variables.size() > max_calc_row) {
    FeatureTree* box = new FeatureTree(variables, variables_help, operatorColor.light(120));
    buttonLayout->addWidget(box, max_row+1, 0, 1, 5);
    QFont f = box->font();
    f.setPointSize(f.pointSize()+4);
    box->setFont(f);
    connect(box, SIGNAL(activated(const QString&)), this, SLOT(variableClicked(const QString&)));
  }
  else if (variables.size() > max_row) {  // variables in a box on the bottom
    QHComboBox* box = new QHComboBox(variables, variables_help, operatorColor.light(120));
    buttonLayout->addWidget(box, max_row+1, 0, 1, 5);
    QFont f = box->font();
    f.setPointSize(f.pointSize()+4);
    box->setFont(f);
    connect(box, SIGNAL(activated(const QString&)), this, SLOT(variableClicked(const QString&)));
  } 
  else {   // variables on the left
    for(int row=0; row<variables.size(); row++)
      buttonLayout->addWidget(createButton(variables[row],
					   row < variables_help.size() ?
					   variables_help[row] : QString("No Help"),
					   operatorColor.light(120), 
					   SLOT(variableClicked())),
			      row+1,0);
  }

  layout->addLayout(buttonLayout);
  
  QHBoxLayout* hlayout = new QHBoxLayout;
  QPushButton* applyB  = new QPushButton("Apply");
  QPushButton* cancelB = new QPushButton("Cancel");
  connect(applyB , SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancelB, SIGNAL(clicked()), this, SLOT(reject()));
  hlayout->addWidget(applyB);
  hlayout->addWidget(cancelB);
  layout->addLayout(hlayout);

  setLayout(layout);
}

void Calculator::digitClicked()
{
  QString text = _display->text();

  if (!(text.isEmpty() ||
	numberAtEnd(text) ||
	lengthAtEnd(_varconops,text)))
    return;

  //  CalculatorButton* clickedButton = qobject_cast<CalculatorButton* >(sender());
  QToolButton* clickedButton = qobject_cast<QToolButton* >(sender());
  int digitValue = clickedButton->text().toInt();
  if (digitValue == 0) {  // prevent superfluous "0" digits
    int sz = text.size();
    if (text.endsWith("0") && !numberAtEnd(text.mid(0,sz-1)))
      return;
  }

  _display->setText(text + QString::number(digitValue));
}

void Calculator::pointClicked()
{
  QString text = _display->text();

  if (!(text.isEmpty() ||
	numberAtEnd(text) ||
	lengthAtEnd(_varconops,text)))
    return;

  unsigned len = numberAtEnd(text);
  printf("point %d numberAtEnd\n",len);
  if (!len)
    text.append(tr("0"));
  else if (text.right(len).contains(tr(".")))
    return;

  text.append(tr("."));
  _display->setText(text);
}

/*
void Calculator::changeSignClicked()
{
  QString text = display->text();
  double value = text.toDouble();
  
  if (value > 0.0) {
    text.prepend(tr("-"));
  } else if (value < 0.0) {
    text.remove(0, 1);
  }
  display->setText(text);
}
*/

void Calculator::variableClicked()
{
  variableClicked(qobject_cast<QToolButton* >(sender())->text());
}

void Calculator::variableClicked(const QString& v)
{
  if (!_variables.contains(v))
    return;

  QString text = _display->text();

  int len;
  if ((len = lengthAtEnd(_variables,text)))
    text.chop(len);
  else if (!(text.isEmpty() ||
             lengthAtEnd(_varvarops,text) ||
             lengthAtEnd(_varconops,text)))
    return;

  text.append(v);
  _display->setText(text);
}

void Calculator::varvarClicked()
{
  QString text = _display->text();

  if (text.isEmpty() ||
      lengthAtEnd(_varvarops,text) ||
      lengthAtEnd(_varconops,text) ||
      numberAtEnd(text))
    return;

  //  CalculatorButton* clickedButton = qobject_cast<CalculatorButton* >(sender());
  QToolButton* clickedButton = qobject_cast<QToolButton* >(sender());
  text.append(clickedButton->text());
  _display->setText(text);
}

void Calculator::varconClicked()
{
  QString text = _display->text();

  if (text.isEmpty() ||
      lengthAtEnd(_varvarops,text) ||
      lengthAtEnd(_varconops,text))
    return;

  //  CalculatorButton* clickedButton = qobject_cast<CalculatorButton* >(sender());
  QToolButton* clickedButton = qobject_cast<QToolButton* >(sender());
  text.append(clickedButton->text());
  _display->setText(text);
}

void Calculator::backspaceClicked()
{
  QString text = _display->text();

  unsigned len;

  if ((len = numberAtEnd(text))) 
    text.chop(1);
  else if ((len = lengthAtEnd(_variables,text)))
    text.chop(len);
  else if ((len = lengthAtEnd(_varvarops,text)))
    text.chop(len);
  else if ((len = lengthAtEnd(_varconops,text)))
    text.chop(len);

  _display->setText(text);
}

void Calculator::clear()
{
  QString text = _display->text();
  unsigned len = numberAtEnd(text);
  if (len) {
    text.chop(len);
    _display->setText(text);
  }
}

void Calculator::parenthesesClicked()
{
  int i         = _display->selectionStart();
  QString sel   = _display->selectedText();
  QString begin = _display->text().mid(0,i);
  if (i > 0) {
    if (!(lengthAtEnd(_varvarops,begin) ||
	  lengthAtEnd(_varconops,begin) ||
	  begin.endsWith("(")))
      return;
  }
  QString end = _display->text().mid(i + sel.size());
  if (i + sel.size() < _display->text().size()) {
    if (!(lengthAtBegin(_varvarops,end) ||
	  lengthAtBegin(_varconops,end) ||
	  end.startsWith(")")))
      return;
  }
  if (sel.count("(") != sel.count(")"))
    return;

  _display->setText(begin + "(" + sel + ")" + end);
}

void Calculator::clearAll()
{
  _display->setText(_reset);
}

CalculatorButton* Calculator::createButton(const QString &text, 
					   const QColor &color,
					   const char *member)
{
  CalculatorButton* button = new CalculatorButton(text, QString(), color);
  QFont f = button->font();
  f.setPointSize(f.pointSize()+4);
  button->setFont(f);
  connect(button, SIGNAL(clicked()), this, member);
  return button;
}

CalculatorButton* Calculator::createButton(const QString &text, 
					   const QString &help,
					   const QColor &color,
					   const char *member)
{
  CalculatorButton* button = new CalculatorButton(text, help, color);
  QFont f = button->font();
  f.setPointSize(f.pointSize()+4);
  button->setFont(f);
  connect(button, SIGNAL(clicked()), this, member);
  return button;
}

QString Calculator::result() const { return _display->text(); }

