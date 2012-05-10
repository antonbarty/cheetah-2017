#ifndef AmiQt_QHComboBox_hh
#define AmiQt_QHComboBox_hh

#include <QtGui/QComboBox>

class QStringList;
class QColor;

namespace Ami {
  namespace Qt {
    class QHComboBox : public QComboBox {
      Q_OBJECT
    public:
      QHComboBox(const QStringList& v,
		 const QStringList& h);
      QHComboBox(const QStringList& v,
		 const QStringList& h,
		 const QColor&      c);
    public:
      bool event(QEvent* event);
    private:
      const QStringList& _help;
    };
  };
};

#endif
