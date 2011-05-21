# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'CommandLine.ui'
#
# Created: Fri May 13 21:40:23 2011
#      by: PyQt4 UI code generator 4.7.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_CommandLine(object):
    def setupUi(self, CommandLine):
        CommandLine.setObjectName("CommandLine")
        CommandLine.resize(577, 232)
        CommandLine.setFeatures(QtGui.QDockWidget.AllDockWidgetFeatures)
        self.dockWidgetContents = QtGui.QWidget()
        self.dockWidgetContents.setObjectName("dockWidgetContents")
        self.verticalLayout = QtGui.QVBoxLayout(self.dockWidgetContents)
        self.verticalLayout.setObjectName("verticalLayout")
        self.cmdHist = QtGui.QTextEdit(self.dockWidgetContents)
        self.cmdHist.setTextInteractionFlags(QtCore.Qt.TextSelectableByKeyboard|QtCore.Qt.TextSelectableByMouse)
        self.cmdHist.setObjectName("cmdHist")
        self.verticalLayout.addWidget(self.cmdHist)
        self.horizontalLayout_3 = QtGui.QHBoxLayout()
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        self.cmdEntry = QtGui.QLineEdit(self.dockWidgetContents)
        self.cmdEntry.setObjectName("cmdEntry")
        self.horizontalLayout_3.addWidget(self.cmdEntry)
        self.runButton = QtGui.QPushButton(self.dockWidgetContents)
        self.runButton.setObjectName("runButton")
        self.horizontalLayout_3.addWidget(self.runButton)
        self.verticalLayout.addLayout(self.horizontalLayout_3)
        CommandLine.setWidget(self.dockWidgetContents)

        self.retranslateUi(CommandLine)
        QtCore.QObject.connect(self.cmdEntry, QtCore.SIGNAL("returnPressed()"), self.runButton.animateClick)
        QtCore.QMetaObject.connectSlotsByName(CommandLine)

    def retranslateUi(self, CommandLine):
        CommandLine.setWindowTitle(QtGui.QApplication.translate("CommandLine", "Command Window", None, QtGui.QApplication.UnicodeUTF8))
        self.runButton.setText(QtGui.QApplication.translate("CommandLine", "Run", None, QtGui.QApplication.UnicodeUTF8))

