#!/usr/bin/python


import sys
sys.path.append("sip")
from PyQt4 import QtGui, QtCore, Qt
from PyQt4.QtCore import QObject, pyqtSignal
from PyQt4.QtGui import QWidget, QDockWidget
import xpcb
import cmdLinePlugin
import sip

app = QtGui.QApplication(sys.argv)
app.setApplicationName("xpcb")
app.setOrganizationDomain("xpcb.org")
app.setOrganizationName("xpcb.org")

pl = cmdLinePlugin.PluginMain()
w = xpcb.FPEditWindow(None, [pl])
pl.initPlugin(w)
w.show()
sys.exit(app.exec_())

