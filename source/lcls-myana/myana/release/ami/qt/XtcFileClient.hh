#ifndef AmiQt_XtcFileClient_hh
#define AmiQt_XtcFileClient_hh

#include <QtGui/QApplication>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QFileDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QPlastiqueStyle>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

#include "ami/app/XtcClient.hh"
#include "ami/qt/RunTree.hh"
#include "ami/service/Routine.hh"
#include "ami/service/Task.hh"
#include "pdsdata/ana/XtcRun.hh"
#include "pdsdata/xtc/Dgram.hh"
#include "pdsdata/xtc/ProcInfo.hh"

#include <errno.h>

using namespace Ami;
using namespace Pds;
using namespace Pds::Ana;
using namespace std;

namespace Ami {
  namespace Qt {
    class XtcFileClient : public QWidget, public Routine {
      Q_OBJECT
    public:
      XtcFileClient(QGroupBox* groupBox, XtcClient& client, const char* curdir, bool testMode);
      ~XtcFileClient();
      void configure();
    private slots:
      void selectDir();
      void selectRun(int);
      void runClicked();
      void stopClicked();
      void hzSliderChanged(int);
      void runSliderChanged(int value);
      void printDgram(const Dgram dg);
      void setStatus(const QString s);
      void setStatusLabelText(const QString s);
      void setEnabled(QWidget* widget, bool enabled);
      void updateDirLabel();
      void updateRunCombo();
      void updateRun();
    signals:
      void _printDgram(const Dgram dg);
      void _setStatusLabelText(const QString s);
      void _setEnabled(QWidget* widget, bool enabled);
      void _updateDirLabel();
      void _updateRunCombo();
      void _updateRun();
    private:
      void routine();
      void insertTransition(TransitionId::Value transition);
      void getPathsForRun(QStringList& list, QString run);
      void configure_run();
      void CHECK(int line);
      void runSliderSetRange(double start, double end);
      void runSliderSetValue(double value);
      void do_configure(QString runName);
      void run();
      void seekTime(double time);
      void setDir(QString dir);
      XtcRun _run;
      bool _runValid;
      XtcClient _client;
      QString _curdir;
      bool _testMode;
      Task* _task;  // thread for Qt
      QPushButton* _dirSelect;
      QLabel* _dirLabel;
      QComboBox* _runCombo;
      QStringList _runList;
      QString _runName;
      QPushButton* _runButton;
      QPushButton* _stopButton;
      QPushButton* _exitButton;
      QLabel* _startLabel;
      QLabel* _damageLabel;

      QSlider* _runSlider;
      QLabel* _runSliderLabel;
      double _runSliderLastMoved;
      int _runSliderMovedTo; // Qt sets this to _runSlider->value() if user moved slider
      bool _runSliderBeingSet;

      QLabel* _hzLabel;
      QSlider* _hzSlider;
      QLabel* _hzSliderLabel;
      int _hzSliderValue;

      QLabel* _statusLabel;
      bool _running;
      bool _stopped;
      int _dgCount;
      double _executionStart;
      uint32_t _damageMask;
      unsigned _damageCount;
      unsigned long long _payloadTotal;
      double _start;
      double _end;
      double _length;
    };
  };
}

#endif
