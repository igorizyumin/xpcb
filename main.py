#!/usr/bin/python

import sys
sys.path.append("sip")
from PyQt4 import QtGui, QtCore, Qt
from PyQt4.QtCore import QObject, pyqtSignal, QSettings
from PyQt4.QtGui import QWidget, QDockWidget, QColor
import xpcb
import cmdLinePlugin
import sip

def setDefaultValue(s, key, value):
	if (not s.contains(key)):
		s.setValue(key, value)

def initSettings():
	s = QSettings()
	setDefaultValue(s, "colors/selection", QColor(255, 255, 255));
	setDefaultValue(s, "colors/background", QColor(0, 0, 0));
	setDefaultValue(s, "colors/visible grid", QColor(255, 255, 255));
	setDefaultValue(s, "colors/drc error", QColor(255, 128, 64));
	setDefaultValue(s, "colors/board outline", QColor(0, 0, 255));
	setDefaultValue(s, "colors/rat line", QColor(255, 0, 255));
	setDefaultValue(s, "colors/top silk", QColor(255, 255, 0));
	setDefaultValue(s, "colors/bottom silk", QColor(255, 192, 192));
	setDefaultValue(s, "colors/top sm cutout", QColor(160,160,160));
	setDefaultValue(s, "colors/bot sm cutout", QColor(95, 95, 95));
	setDefaultValue(s, "colors/drilled hole", QColor(0, 0, 255));
	setDefaultValue(s, "colors/top copper", QColor(0, 210, 0));
	setDefaultValue(s, "colors/bottom copper", QColor(210, 0, 0));
	setDefaultValue(s, "colors/inner 1", QColor(0, 190, 0));
	setDefaultValue(s, "colors/inner 2", QColor(190, 0, 0));
	setDefaultValue(s, "colors/inner 3", QColor(0, 170, 0));
	setDefaultValue(s, "colors/inner 4", QColor(170, 0, 0));
	setDefaultValue(s, "colors/inner 5", QColor(0, 150, 0));
	setDefaultValue(s, "colors/inner 6", QColor(150, 0, 0));
	setDefaultValue(s, "colors/inner 7", QColor(0, 130, 0));
	setDefaultValue(s, "colors/inner 8", QColor(130, 0, 0));
	setDefaultValue(s, "colors/inner 9", QColor(0, 110, 0));
	setDefaultValue(s, "colors/inner 10", QColor(110, 0, 0));
	setDefaultValue(s, "colors/inner 11", QColor(119, 0, 220));
	setDefaultValue(s, "colors/inner 12", QColor(5, 0, 220));
	setDefaultValue(s, "colors/inner 13", QColor(0, 150, 220));
	setDefaultValue(s, "colors/inner 14", QColor(220, 145, 0));
#	setDefaultValue(s, "colors/inner 15", QColor(220, 83, 0));
#	setDefaultValue(s, "colors/inner 16", QColor(220, 0, 140));
	setDefaultValue(s, "colors/top paste", QColor(0, 150, 0));
	setDefaultValue(s, "colors/bottom paste", QColor(150, 0, 0));
	setDefaultValue(s, "colors/start pad", QColor(0, 210, 0));
	setDefaultValue(s, "colors/inner pad", QColor(220, 145, 0));
	setDefaultValue(s, "colors/end pad", QColor(210, 0, 0));
	setDefaultValue(s, "colors/centroid", QColor(255, 255, 255));
	setDefaultValue(s, "colors/adhesive", QColor(231, 231, 231));
	setDefaultValue(s, "colors/unknown layer", QColor(255, 0, 0));



app = QtGui.QApplication(sys.argv)
app.setApplicationName("xpcb")
app.setOrganizationDomain("xpcb.org")
app.setOrganizationName("xpcb.org")
initSettings()

pl = cmdLinePlugin.PluginMain()
w = xpcb.FPEditWindow(None, [pl])
pl.initPlugin(w)
w.show()
sys.exit(app.exec_())

