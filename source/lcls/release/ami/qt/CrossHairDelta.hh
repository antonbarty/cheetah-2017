#ifndef AmiQt_CrossHairDelta_hh
#define AmiQt_CrossHairDelta_hh

#include <QtCore/QObject>

class QGridLayout;
class QLineEdit;

namespace Ami {
  namespace Qt {

    class CrossHair;

    class CrossHairDelta : public QObject {
      Q_OBJECT
    public:
      CrossHairDelta(QGridLayout& layout, unsigned row,
		     CrossHair& hair1,
		     CrossHair& hair2);
      ~CrossHairDelta();
    public slots:
      void update();
    private:
      QLineEdit* _column;
      QLineEdit* _row;
      QLineEdit* _value;
      CrossHair& _hair1;
      CrossHair& _hair2;
    };
  };
};

#endif
