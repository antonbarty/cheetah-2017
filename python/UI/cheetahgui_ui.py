# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'cheetahgui.ui'
#
# Created by: PyQt4 UI code generator 4.11.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName(_fromUtf8("MainWindow"))
        MainWindow.resize(776, 855)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(MainWindow.sizePolicy().hasHeightForWidth())
        MainWindow.setSizePolicy(sizePolicy)
        self.centralwidget = QtGui.QWidget(MainWindow)
        self.centralwidget.setObjectName(_fromUtf8("centralwidget"))
        self.gridLayout = QtGui.QGridLayout(self.centralwidget)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setSizeConstraint(QtGui.QLayout.SetDefaultConstraint)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setSizeConstraint(QtGui.QLayout.SetDefaultConstraint)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.button_refresh = QtGui.QPushButton(self.centralwidget)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.button_refresh.sizePolicy().hasHeightForWidth())
        self.button_refresh.setSizePolicy(sizePolicy)
        self.button_refresh.setMaximumSize(QtCore.QSize(100, 25))
        self.button_refresh.setObjectName(_fromUtf8("button_refresh"))
        self.horizontalLayout.addWidget(self.button_refresh)
        self.button_runCheetah = QtGui.QPushButton(self.centralwidget)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.button_runCheetah.sizePolicy().hasHeightForWidth())
        self.button_runCheetah.setSizePolicy(sizePolicy)
        self.button_runCheetah.setMaximumSize(QtCore.QSize(100, 25))
        self.button_runCheetah.setObjectName(_fromUtf8("button_runCheetah"))
        self.horizontalLayout.addWidget(self.button_runCheetah)
        self.button_index = QtGui.QPushButton(self.centralwidget)
        self.button_index.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.button_index.sizePolicy().hasHeightForWidth())
        self.button_index.setSizePolicy(sizePolicy)
        self.button_index.setMaximumSize(QtCore.QSize(100, 25))
        self.button_index.setObjectName(_fromUtf8("button_index"))
        self.horizontalLayout.addWidget(self.button_index)
        self.button_viewhits = QtGui.QPushButton(self.centralwidget)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.button_viewhits.sizePolicy().hasHeightForWidth())
        self.button_viewhits.setSizePolicy(sizePolicy)
        self.button_viewhits.setMaximumSize(QtCore.QSize(100, 25))
        self.button_viewhits.setObjectName(_fromUtf8("button_viewhits"))
        self.horizontalLayout.addWidget(self.button_viewhits)
        self.button_virtualpowder = QtGui.QPushButton(self.centralwidget)
        self.button_virtualpowder.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.button_virtualpowder.sizePolicy().hasHeightForWidth())
        self.button_virtualpowder.setSizePolicy(sizePolicy)
        self.button_virtualpowder.setMaximumSize(QtCore.QSize(100, 25))
        self.button_virtualpowder.setObjectName(_fromUtf8("button_virtualpowder"))
        self.horizontalLayout.addWidget(self.button_virtualpowder)
        self.button_peakogram = QtGui.QPushButton(self.centralwidget)
        self.button_peakogram.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.button_peakogram.sizePolicy().hasHeightForWidth())
        self.button_peakogram.setSizePolicy(sizePolicy)
        self.button_peakogram.setMaximumSize(QtCore.QSize(100, 25))
        self.button_peakogram.setObjectName(_fromUtf8("button_peakogram"))
        self.horizontalLayout.addWidget(self.button_peakogram)
        spacerItem = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.table_status = QtGui.QTableWidget(self.centralwidget)
        self.table_status.setLineWidth(3)
        self.table_status.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
        self.table_status.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOn)
        self.table_status.setRowCount(10)
        self.table_status.setObjectName(_fromUtf8("table_status"))
        self.table_status.setColumnCount(10)
        item = QtGui.QTableWidgetItem()
        self.table_status.setHorizontalHeaderItem(0, item)
        item = QtGui.QTableWidgetItem()
        self.table_status.setHorizontalHeaderItem(1, item)
        item = QtGui.QTableWidgetItem()
        self.table_status.setHorizontalHeaderItem(2, item)
        item = QtGui.QTableWidgetItem()
        self.table_status.setHorizontalHeaderItem(3, item)
        item = QtGui.QTableWidgetItem()
        self.table_status.setHorizontalHeaderItem(4, item)
        item = QtGui.QTableWidgetItem()
        self.table_status.setHorizontalHeaderItem(5, item)
        item = QtGui.QTableWidgetItem()
        self.table_status.setHorizontalHeaderItem(6, item)
        item = QtGui.QTableWidgetItem()
        self.table_status.setHorizontalHeaderItem(7, item)
        item = QtGui.QTableWidgetItem()
        self.table_status.setHorizontalHeaderItem(8, item)
        item = QtGui.QTableWidgetItem()
        self.table_status.setHorizontalHeaderItem(9, item)
        self.table_status.horizontalHeader().setCascadingSectionResizes(False)
        self.table_status.horizontalHeader().setDefaultSectionSize(90)
        self.table_status.horizontalHeader().setHighlightSections(False)
        self.verticalLayout.addWidget(self.table_status)
        self.horizontalLayout2 = QtGui.QHBoxLayout()
        self.horizontalLayout2.setObjectName(_fromUtf8("horizontalLayout2"))
        self.verticalLayout.addLayout(self.horizontalLayout2)
        self.gridLayout.addLayout(self.verticalLayout, 1, 0, 1, 1)
        MainWindow.setCentralWidget(self.centralwidget)
        self.menuBar = QtGui.QMenuBar(MainWindow)
        self.menuBar.setGeometry(QtCore.QRect(0, 0, 776, 22))
        self.menuBar.setObjectName(_fromUtf8("menuBar"))
        self.menuFile = QtGui.QMenu(self.menuBar)
        self.menuFile.setObjectName(_fromUtf8("menuFile"))
        self.menuCheetah = QtGui.QMenu(self.menuBar)
        self.menuCheetah.setObjectName(_fromUtf8("menuCheetah"))
        self.menuAnalysis = QtGui.QMenu(self.menuBar)
        self.menuAnalysis.setObjectName(_fromUtf8("menuAnalysis"))
        self.menu_masks = QtGui.QMenu(self.menuBar)
        self.menu_masks.setObjectName(_fromUtf8("menu_masks"))
        self.menuPowder = QtGui.QMenu(self.menuBar)
        self.menuPowder.setObjectName(_fromUtf8("menuPowder"))
        self.menuLogs = QtGui.QMenu(self.menuBar)
        self.menuLogs.setObjectName(_fromUtf8("menuLogs"))
        self.menuUtilities = QtGui.QMenu(self.menuBar)
        self.menuUtilities.setObjectName(_fromUtf8("menuUtilities"))
        MainWindow.setMenuBar(self.menuBar)
        self.menu_file_autorefresh = QtGui.QAction(MainWindow)
        self.menu_file_autorefresh.setObjectName(_fromUtf8("menu_file_autorefresh"))
        self.menu_file_refreshtable = QtGui.QAction(MainWindow)
        self.menu_file_refreshtable.setObjectName(_fromUtf8("menu_file_refreshtable"))
        self.menu_file_startcrawler = QtGui.QAction(MainWindow)
        self.menu_file_startcrawler.setObjectName(_fromUtf8("menu_file_startcrawler"))
        self.menu_file_command = QtGui.QAction(MainWindow)
        self.menu_file_command.setObjectName(_fromUtf8("menu_file_command"))
        self.menu_file_quit = QtGui.QAction(MainWindow)
        self.menu_file_quit.setObjectName(_fromUtf8("menu_file_quit"))
        self.menu_mask_badpixdark = QtGui.QAction(MainWindow)
        self.menu_mask_badpixdark.setObjectName(_fromUtf8("menu_mask_badpixdark"))
        self.menu_mask_badpixbright = QtGui.QAction(MainWindow)
        self.menu_mask_badpixbright.setObjectName(_fromUtf8("menu_mask_badpixbright"))
        self.actionDefault_particle_display_settings = QtGui.QAction(MainWindow)
        self.actionDefault_particle_display_settings.setObjectName(_fromUtf8("actionDefault_particle_display_settings"))
        self.actionHistogram_clip = QtGui.QAction(MainWindow)
        self.actionHistogram_clip.setCheckable(True)
        self.actionHistogram_clip.setChecked(True)
        self.actionHistogram_clip.setObjectName(_fromUtf8("actionHistogram_clip"))
        self.actionAuto_scale_levels = QtGui.QAction(MainWindow)
        self.actionAuto_scale_levels.setCheckable(True)
        self.actionAuto_scale_levels.setChecked(True)
        self.actionAuto_scale_levels.setObjectName(_fromUtf8("actionAuto_scale_levels"))
        self.menu_cheetah_processselected = QtGui.QAction(MainWindow)
        self.menu_cheetah_processselected.setObjectName(_fromUtf8("menu_cheetah_processselected"))
        self.menu_cheetah_relabel = QtGui.QAction(MainWindow)
        self.menu_cheetah_relabel.setObjectName(_fromUtf8("menu_cheetah_relabel"))
        self.menu_cheetah_autorun = QtGui.QAction(MainWindow)
        self.menu_cheetah_autorun.setObjectName(_fromUtf8("menu_cheetah_autorun"))
        self.menu_mask_combine = QtGui.QAction(MainWindow)
        self.menu_mask_combine.setObjectName(_fromUtf8("menu_mask_combine"))
        self.menu_mask_makecspadgain = QtGui.QAction(MainWindow)
        self.menu_mask_makecspadgain.setObjectName(_fromUtf8("menu_mask_makecspadgain"))
        self.menu_mask_translatecspadgain = QtGui.QAction(MainWindow)
        self.menu_mask_translatecspadgain.setObjectName(_fromUtf8("menu_mask_translatecspadgain"))
        self.menu_analysis_peakogram = QtGui.QAction(MainWindow)
        self.menu_analysis_peakogram.setObjectName(_fromUtf8("menu_analysis_peakogram"))
        self.menu_analysis_saturation = QtGui.QAction(MainWindow)
        self.menu_analysis_saturation.setObjectName(_fromUtf8("menu_analysis_saturation"))
        self.menu_analysis_hitrate = QtGui.QAction(MainWindow)
        self.menu_analysis_hitrate.setObjectName(_fromUtf8("menu_analysis_hitrate"))
        self.menu_analysis_resolution = QtGui.QAction(MainWindow)
        self.menu_analysis_resolution.setObjectName(_fromUtf8("menu_analysis_resolution"))
        self.menu_powder_hits = QtGui.QAction(MainWindow)
        self.menu_powder_hits.setObjectName(_fromUtf8("menu_powder_hits"))
        self.menu_powder_hits_det = QtGui.QAction(MainWindow)
        self.menu_powder_hits_det.setObjectName(_fromUtf8("menu_powder_hits_det"))
        self.menu_powder_blank = QtGui.QAction(MainWindow)
        self.menu_powder_blank.setObjectName(_fromUtf8("menu_powder_blank"))
        self.menu_powder_blank_det = QtGui.QAction(MainWindow)
        self.menu_powder_blank_det.setObjectName(_fromUtf8("menu_powder_blank_det"))
        self.menu_powder_peaks_hits = QtGui.QAction(MainWindow)
        self.menu_powder_peaks_hits.setObjectName(_fromUtf8("menu_powder_peaks_hits"))
        self.menu_powder_peaks_blank = QtGui.QAction(MainWindow)
        self.menu_powder_peaks_blank.setObjectName(_fromUtf8("menu_powder_peaks_blank"))
        self.menu_log_batch = QtGui.QAction(MainWindow)
        self.menu_log_batch.setObjectName(_fromUtf8("menu_log_batch"))
        self.menu_log_cheetah = QtGui.QAction(MainWindow)
        self.menu_log_cheetah.setObjectName(_fromUtf8("menu_log_cheetah"))
        self.menu_log_cheetahstatus = QtGui.QAction(MainWindow)
        self.menu_log_cheetahstatus.setObjectName(_fromUtf8("menu_log_cheetahstatus"))
        self.menu_mask_maker = QtGui.QAction(MainWindow)
        self.menu_mask_maker.setObjectName(_fromUtf8("menu_mask_maker"))
        self.menu_utilities_psocake = QtGui.QAction(MainWindow)
        self.menu_utilities_psocake.setObjectName(_fromUtf8("menu_utilities_psocake"))
        self.menu_powder_darkcal = QtGui.QAction(MainWindow)
        self.menu_powder_darkcal.setObjectName(_fromUtf8("menu_powder_darkcal"))
        self.menu_mask_view = QtGui.QAction(MainWindow)
        self.menu_mask_view.setObjectName(_fromUtf8("menu_mask_view"))
        self.menu_file_newgeometry = QtGui.QAction(MainWindow)
        self.menu_file_newgeometry.setObjectName(_fromUtf8("menu_file_newgeometry"))
        self.menu_utilities_calibman = QtGui.QAction(MainWindow)
        self.menu_utilities_calibman.setObjectName(_fromUtf8("menu_utilities_calibman"))
        self.menu_powder_copydarkcal = QtGui.QAction(MainWindow)
        self.menu_powder_copydarkcal.setObjectName(_fromUtf8("menu_powder_copydarkcal"))
        self.menuFile.addAction(self.menu_file_command)
        self.menuFile.addAction(self.menu_file_startcrawler)
        self.menuFile.addAction(self.menu_file_newgeometry)
        self.menuFile.addSeparator()
        self.menuFile.addAction(self.menu_file_quit)
        self.menuCheetah.addAction(self.menu_cheetah_processselected)
        self.menuCheetah.addAction(self.menu_cheetah_relabel)
        self.menuCheetah.addAction(self.menu_cheetah_autorun)
        self.menuAnalysis.addAction(self.menu_analysis_hitrate)
        self.menuAnalysis.addAction(self.menu_analysis_resolution)
        self.menuAnalysis.addAction(self.menu_analysis_saturation)
        self.menuAnalysis.addAction(self.menu_analysis_peakogram)
        self.menu_masks.addAction(self.menu_mask_maker)
        self.menu_masks.addAction(self.menu_mask_badpixdark)
        self.menu_masks.addAction(self.menu_mask_badpixbright)
        self.menu_masks.addAction(self.menu_mask_combine)
        self.menu_masks.addAction(self.menu_mask_makecspadgain)
        self.menu_masks.addAction(self.menu_mask_translatecspadgain)
        self.menu_masks.addAction(self.menu_mask_view)
        self.menuPowder.addAction(self.menu_powder_hits)
        self.menuPowder.addAction(self.menu_powder_hits_det)
        self.menuPowder.addAction(self.menu_powder_blank)
        self.menuPowder.addAction(self.menu_powder_blank_det)
        self.menuPowder.addAction(self.menu_powder_peaks_hits)
        self.menuPowder.addAction(self.menu_powder_peaks_blank)
        self.menuPowder.addSeparator()
        self.menuPowder.addAction(self.menu_powder_darkcal)
        self.menuPowder.addAction(self.menu_powder_copydarkcal)
        self.menuLogs.addAction(self.menu_log_batch)
        self.menuLogs.addAction(self.menu_log_cheetah)
        self.menuLogs.addAction(self.menu_log_cheetahstatus)
        self.menuUtilities.addAction(self.menu_utilities_calibman)
        self.menuUtilities.addAction(self.menu_utilities_psocake)
        self.menuBar.addAction(self.menuFile.menuAction())
        self.menuBar.addAction(self.menuCheetah.menuAction())
        self.menuBar.addAction(self.menuPowder.menuAction())
        self.menuBar.addAction(self.menu_masks.menuAction())
        self.menuBar.addAction(self.menuAnalysis.menuAction())
        self.menuBar.addAction(self.menuUtilities.menuAction())
        self.menuBar.addAction(self.menuLogs.menuAction())

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(_translate("MainWindow", "Cheetah GUI", None))
        self.button_refresh.setText(_translate("MainWindow", "Refresh", None))
        self.button_runCheetah.setText(_translate("MainWindow", "Run Cheetah", None))
        self.button_index.setText(_translate("MainWindow", "Turbo index", None))
        self.button_viewhits.setText(_translate("MainWindow", "View hits", None))
        self.button_virtualpowder.setText(_translate("MainWindow", "Powder", None))
        self.button_peakogram.setText(_translate("MainWindow", "Peakogram", None))
        self.table_status.setSortingEnabled(True)
        item = self.table_status.horizontalHeaderItem(0)
        item.setText(_translate("MainWindow", "Run", None))
        item = self.table_status.horizontalHeaderItem(1)
        item.setText(_translate("MainWindow", "Dataset", None))
        item = self.table_status.horizontalHeaderItem(2)
        item.setText(_translate("MainWindow", "XTC", None))
        item = self.table_status.horizontalHeaderItem(3)
        item.setText(_translate("MainWindow", "Cheetah", None))
        item = self.table_status.horizontalHeaderItem(4)
        item.setText(_translate("MainWindow", "CrystFEL", None))
        item = self.table_status.horizontalHeaderItem(5)
        item.setText(_translate("MainWindow", "H5 Directory", None))
        item = self.table_status.horizontalHeaderItem(6)
        item.setText(_translate("MainWindow", "Nprocessed", None))
        item = self.table_status.horizontalHeaderItem(7)
        item.setText(_translate("MainWindow", "Nhits", None))
        item = self.table_status.horizontalHeaderItem(8)
        item.setText(_translate("MainWindow", "Nindex", None))
        item = self.table_status.horizontalHeaderItem(9)
        item.setText(_translate("MainWindow", "Hitrate%", None))
        self.menuFile.setTitle(_translate("MainWindow", "File", None))
        self.menuCheetah.setTitle(_translate("MainWindow", "Cheetah", None))
        self.menuAnalysis.setTitle(_translate("MainWindow", "Analysis", None))
        self.menu_masks.setTitle(_translate("MainWindow", "Masks", None))
        self.menuPowder.setTitle(_translate("MainWindow", "Powder", None))
        self.menuLogs.setTitle(_translate("MainWindow", "Logs", None))
        self.menuUtilities.setTitle(_translate("MainWindow", "Utilities", None))
        self.menu_file_autorefresh.setText(_translate("MainWindow", "Auto refresh table", None))
        self.menu_file_refreshtable.setText(_translate("MainWindow", "Refresh table", None))
        self.menu_file_startcrawler.setText(_translate("MainWindow", "Start crawler", None))
        self.menu_file_command.setText(_translate("MainWindow", "Unlock command operations", None))
        self.menu_file_quit.setText(_translate("MainWindow", "Quit", None))
        self.menu_mask_badpixdark.setText(_translate("MainWindow", "Make bad pixel mask from darkcal", None))
        self.menu_mask_badpixbright.setText(_translate("MainWindow", "Make bad pixel mask from brightfield", None))
        self.actionDefault_particle_display_settings.setText(_translate("MainWindow", "Default particle display settings", None))
        self.actionHistogram_clip.setText(_translate("MainWindow", "Histogram clip", None))
        self.actionAuto_scale_levels.setText(_translate("MainWindow", "Clamp histogram scale ", None))
        self.menu_cheetah_processselected.setText(_translate("MainWindow", "Process selected runs", None))
        self.menu_cheetah_relabel.setText(_translate("MainWindow", "Label or relabel dataset", None))
        self.menu_cheetah_autorun.setText(_translate("MainWindow", "Autorun when new data is ready", None))
        self.menu_mask_combine.setText(_translate("MainWindow", "Combine masks", None))
        self.menu_mask_makecspadgain.setText(_translate("MainWindow", "Create CSPAD gain map", None))
        self.menu_mask_translatecspadgain.setText(_translate("MainWindow", "Translate CSPAD gain map", None))
        self.menu_analysis_peakogram.setText(_translate("MainWindow", "Peakogram", None))
        self.menu_analysis_saturation.setText(_translate("MainWindow", "Saturation check", None))
        self.menu_analysis_hitrate.setText(_translate("MainWindow", "Hit rate plot", None))
        self.menu_analysis_resolution.setText(_translate("MainWindow", "Resolution plot", None))
        self.menu_powder_hits.setText(_translate("MainWindow", "Virtual powder hits (class 1, background subtracted)", None))
        self.menu_powder_hits_det.setText(_translate("MainWindow", "Virtual powder hits (class 1, detector corrected only)", None))
        self.menu_powder_blank.setText(_translate("MainWindow", "Virtual powder blanks (class 0, background subtracted)", None))
        self.menu_powder_blank_det.setText(_translate("MainWindow", "Virtual powder blanks (class 0, detector corrected only)", None))
        self.menu_powder_peaks_hits.setText(_translate("MainWindow", "Peakfinder virtual powder hits (class 1)", None))
        self.menu_powder_peaks_blank.setText(_translate("MainWindow", "Peakfinder virtual powder blanks (class 0)", None))
        self.menu_log_batch.setText(_translate("MainWindow", "Batch log file", None))
        self.menu_log_cheetah.setText(_translate("MainWindow", "Cheetah log file", None))
        self.menu_log_cheetahstatus.setText(_translate("MainWindow", "Cheetah status file", None))
        self.menu_mask_maker.setText(_translate("MainWindow", "Mask maker", None))
        self.menu_utilities_psocake.setText(_translate("MainWindow", "psocake (Chuck)", None))
        self.menu_powder_darkcal.setText(_translate("MainWindow", "View darkcal", None))
        self.menu_mask_view.setText(_translate("MainWindow", "View mask file", None))
        self.menu_file_newgeometry.setText(_translate("MainWindow", "Change geometry file", None))
        self.menu_utilities_calibman.setText(_translate("MainWindow", "calibman (pcds)", None))
        self.menu_powder_copydarkcal.setText(_translate("MainWindow", "Copy darkcal.h5 to calib/darkcal", None))

