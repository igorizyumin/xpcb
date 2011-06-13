# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'EditLineDialog.ui'
#
# Created: Sun Jun 12 17:40:50 2011
#      by: PyQt4 UI code generator 4.8.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_EditLineDialog(object):
    def setupUi(self, EditLineDialog):
        EditLineDialog.setObjectName(_fromUtf8("EditLineDialog"))
        EditLineDialog.resize(293, 113)
        self.verticalLayout = QtGui.QVBoxLayout(EditLineDialog)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.label = QtGui.QLabel(EditLineDialog)
        self.label.setObjectName(_fromUtf8("label"))
        self.horizontalLayout.addWidget(self.label)
        self.widthBox = QtGui.QDoubleSpinBox(EditLineDialog)
        self.widthBox.setButtonSymbols(QtGui.QAbstractSpinBox.NoButtons)
        self.widthBox.setDecimals(0)
        self.widthBox.setMinimum(0.0)
        self.widthBox.setMaximum(1000.0)
        self.widthBox.setProperty(_fromUtf8("value"), 10.0)
        self.widthBox.setObjectName(_fromUtf8("widthBox"))
        self.horizontalLayout.addWidget(self.widthBox)
        spacerItem = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.label_6 = QtGui.QLabel(EditLineDialog)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_6.sizePolicy().hasHeightForWidth())
        self.label_6.setSizePolicy(sizePolicy)
        self.label_6.setObjectName(_fromUtf8("label_6"))
        self.horizontalLayout.addWidget(self.label_6)
        self.unitsBox = QtGui.QComboBox(EditLineDialog)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.unitsBox.sizePolicy().hasHeightForWidth())
        self.unitsBox.setSizePolicy(sizePolicy)
        self.unitsBox.setObjectName(_fromUtf8("unitsBox"))
        self.unitsBox.addItem(_fromUtf8(""))
        self.unitsBox.addItem(_fromUtf8(""))
        self.horizontalLayout.addWidget(self.unitsBox)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.buttonBox = QtGui.QDialogButtonBox(EditLineDialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.verticalLayout.addWidget(self.buttonBox)
        self.label.setBuddy(self.widthBox)
        self.label_6.setBuddy(self.unitsBox)

        self.retranslateUi(EditLineDialog)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), EditLineDialog.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), EditLineDialog.reject)
        QtCore.QMetaObject.connectSlotsByName(EditLineDialog)
        EditLineDialog.setTabOrder(self.widthBox, self.unitsBox)
        EditLineDialog.setTabOrder(self.unitsBox, self.buttonBox)

    def retranslateUi(self, EditLineDialog):
        EditLineDialog.setWindowTitle(QtGui.QApplication.translate("EditLineDialog", "Line Properties", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("EditLineDialog", "Line &Width", None, QtGui.QApplication.UnicodeUTF8))
        self.widthBox.setSuffix(QtGui.QApplication.translate("EditLineDialog", " mil", None, QtGui.QApplication.UnicodeUTF8))
        self.label_6.setText(QtGui.QApplication.translate("EditLineDialog", "&Units", None, QtGui.QApplication.UnicodeUTF8))
        self.unitsBox.setItemText(0, QtGui.QApplication.translate("EditLineDialog", "mils", None, QtGui.QApplication.UnicodeUTF8))
        self.unitsBox.setItemText(1, QtGui.QApplication.translate("EditLineDialog", "mm", None, QtGui.QApplication.UnicodeUTF8))

