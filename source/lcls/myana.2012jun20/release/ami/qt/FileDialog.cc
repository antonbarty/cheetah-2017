#include "FileDialog.hh"

#include <QtGui/QVBoxLayout>

using namespace Ami::Qt;

class MyFileModel : public QDirModel {
public:
  MyFileModel() : QDirModel(QStringList("*.xtc"),QDir::Files,QDir::Name) {}
  ~MyFileModel() {}
public:
  virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const {
    if (role==Qt::DisplayRole) {
      QFileInfo info = fileInfo(index);
      QString   n = info.baseName();
      QDateTime opened = QDateTime::fromString(n.mid(0,15),"yyyyMMdd-hhmmss");
      QDateTime closed = info.lastModified();
      QTime duration;
      duration.addSecs(opened.secsTo(closed));
      QStringList list;
      list << opened.toString  (::Qt::ISODate);
      list << duration.toString(::Qt::ISODate);
      return list;
    }
    else
      return QDirModel::data(index,role);
  }
  virtual QVariant headerData ( int section, 
				Qt::Orientation orientation, 
				int role = Qt::DisplayRole ) const {
    if (role==Qt::DisplayRole && orientation==Qt::Horizontal) {
      switch(section) {
      case 0 : return QVariant(QString("Run Start"));
      case 1 : return QVariant(QString("Duration"));
      default: return QVariant(QString("Unknown"));
      }
    }
    else
      return QDirModel::headerData(section, orientation, role);
  }
};

FileDialog::FileDialog() :
  QDialog(0)
{
  MyFileModel* model = new MyFileModel;
  QTreeView* tree = new QTreeView;
  tree->setModel(model);

  QPushButton* okB     = new QPushButton("OK");
  QPushButton* cancelB = new QPushButton("Cancel");
  QPushButton* skimB   = new QPushButton("Create Skim");

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(new QLabel("Data Acquisition Runs"));
  layout->addWidget(tree);
  { QHBoxLayout* layout1 = new QHBoxLayout;
    layout1->addWidget(okB    );
    layout1->addWidget(cancelB);
    layout1->addWidget(skimB  );
    layout->addLayout(layout1); }
  setLayout(layout);

  connect(okB    , SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancelB, SIGNAL(clicked()), this, SLOT(reject()));
  connect(skimB  , SIGNAL(clicked()), this, SLOT(define_skim()));
}

FileDialog::~FileDialog()
{
}

void FileDialog::define_skim()
{
  
}

