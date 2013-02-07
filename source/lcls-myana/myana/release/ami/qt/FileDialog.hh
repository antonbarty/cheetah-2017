#ifndef AmiQt_FileDialog_hh
#ifndef AmiQt_FileDialog_hh

#include <QtGui/QDialog>

using namespace Ami {
  using namespace Qt {
    class FileDialog : public QDialog {
    public:
      FileDialog();
      ~FileDialog();
    public:
      const QString& selectedFile() const;
    private:
      QString _selectedFile;
    };
  };
};

#endif
