
import sys
import PyQt4
import PyQt4.QtGui
import PyQt4.QtCore



qtApp = PyQt4.QtGui.QApplication(sys.argv)


#
#   Dialog box to select experiment from drop-down list
#
class expt_select_gui(PyQt4.QtGui.QDialog):
    def __init__(self, explist, parent=None):
        super(expt_select_gui, self).__init__(parent)

        layout = PyQt4.QtGui.QVBoxLayout(self)
        self.setWindowTitle("Cheetah GUI experiment selector")

        # Add a label
        self.label1 = PyQt4.QtGui.QLabel()
        self.label1.setText("Previous experiments")
        layout.addWidget(self.label1)

        # Combo box for experiment list
        self.cb = PyQt4.QtGui.QComboBox()
        #self.cb.addItem("C")
        #self.cb.addItems(["Java", "C#", "Python"])
        self.cb.addItems(explist)
        layout.addWidget(self.cb)

        # Add some user defined buttons
        layout2 = PyQt4.QtGui.QHBoxLayout()
        self.button1 = PyQt4.QtGui.QPushButton("Go to selected experiment",self)
        self.button1.clicked.connect(self.button1_function)
        self.button2 = PyQt4.QtGui.QPushButton("Set up new experiment",self)
        self.button2.clicked.connect(self.button2_function)
        self.button3 = PyQt4.QtGui.QPushButton("Find a different experiment",self)
        self.button3.clicked.connect(self.button3_function)
        self.button4 = PyQt4.QtGui.QPushButton("Cancel",self)
        self.button4.clicked.connect(self.button4_function)
        layout2.addWidget(self.button1)
        layout2.addWidget(self.button3)
        layout2.addWidget(self.button2)
        layout2.addWidget(self.button4)
        layout.addLayout(layout2)

        # Default OK and Cancel buttons
        #self.buttonBox = PyQt4.QtGui.QDialogButtonBox(self)
        #self.buttonBox.setOrientation(PyQt4.QtCore.Qt.Horizontal)
        #self.buttonBox.setStandardButtons(PyQt4.QtGui.QDialogButtonBox.Ok | PyQt4.QtGui.QDialogButtonBox.Cancel)
        #layout.addWidget(self.buttonBox)
        #self.buttonBox.accepted.connect(self.accept)
        #self.buttonBox.rejected.connect(self.reject)


    # Can't use predefined buttons, so need to figure out and return selected action for ourselves
    def button1_function(self):
        self.action = 'goto'
        self.close()

    def button2_function(self):
        self.action = 'setup_new'
        self.close()

    def button3_function(self):
        self.action = 'find'
        self.close()

    def button4_function(self):
        self.action = 'cancel'
        self.close()


    # get selected text from drop down menu
    def getExpt(self):
        action = self.action
        selected_expt = self.cb.currentText()
        result = {
            'action' : action,
            'selected_expt' : selected_expt
        }
        return result

    # static method to create the dialog and return
    @staticmethod
    def get_expt(explist, parent=None):
        dialog = expt_select_gui(explist, parent=parent)
        result = dialog.exec_()
        result = dialog.getExpt()
        return result



#
#   Dialog box to select experiment from drop-down list
#
class run_cheetah_gui(PyQt4.QtGui.QDialog):
    def __init__(self, dialog_info, parent=None):
        super(run_cheetah_gui, self).__init__(parent)

        inifile_list = dialog_info['inifile_list']
        inifile_list = sorted(inifile_list)
        lastini = dialog_info['lastini']
        lasttag = dialog_info['lasttag']


        layout = PyQt4.QtGui.QVBoxLayout(self)
        self.setWindowTitle("Run Cheetah")

        # Add some useful labels
        #self.label1a = PyQt4.QtGui.QLabel()
        #self.label1a.setText("Label")
        #self.label1b = PyQt4.QtGui.QLabel()
        #self.label1b.setText("Command: ../process/process")
        #layout.addWidget(self.label1a)
        #layout.addWidget(self.label1b)


        # Text box for dataset entry
        #TODO: Automatically add last dataset tag
        layout2 = PyQt4.QtGui.QHBoxLayout()
        self.label2 = PyQt4.QtGui.QLabel()
        self.label2.setText("Dataset tag: ")
        self.le = PyQt4.QtGui.QLineEdit(self)
        self.le.setText(lasttag)
        layout2.addWidget(self.label2)
        layout2.addWidget(self.le)
        layout.addLayout(layout2)


        # Combo box for list of .ini files
        #TODO: Automatically select the last ini file
        layout3 = PyQt4.QtGui.QHBoxLayout()
        self.label3 = PyQt4.QtGui.QLabel()
        self.label3.setText("cheetah.ini file: ")
        self.cb = PyQt4.QtGui.QComboBox()
        self.cb.addItem(lastini)
        self.cb.addItems(inifile_list)
        layout3.addWidget(self.label3)
        layout3.addWidget(self.cb)
        layout.addLayout(layout3)


        # Default OK and Cancel buttons
        self.buttonBox = PyQt4.QtGui.QDialogButtonBox(self)
        self.buttonBox.setOrientation(PyQt4.QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(PyQt4.QtGui.QDialogButtonBox.Ok | PyQt4.QtGui.QDialogButtonBox.Cancel)
        self.buttonBox.accepted.connect(self.accept)
        self.buttonBox.rejected.connect(self.reject)
        layout.addWidget(self.buttonBox)


    # get selected text
    def getCheetahIni(self):
        selection = {
            'dataset' : self.le.text(),
            'inifile': self.cb.currentText()
        }
        return selection

    # static method to create the dialog and return
    @staticmethod
    def cheetah_dialog(dialog_info, parent=None):
        dialog = run_cheetah_gui(dialog_info, parent=parent)
        result = dialog.exec_()
        selection = dialog.getCheetahIni()
        return (selection, result == PyQt4.QtGui.QDialog.Accepted)

