import sys
sys.path.append("sip")
from PyQt4 import QtGui, QtCore, Qt
from PyQt4.QtCore import QObject, pyqtSignal
from PyQt4.QtGui import QWidget, QDockWidget
from ui_commandline import Ui_CommandLine
import code
import xpcb
import sip

class CommandLineInterp(QObject, code.InteractiveConsole):
	writeSig = pyqtSignal('QString')
	
	def __init__(self, locals = None):
		QObject.__init__(self)
		code.InteractiveConsole.__init__(self, locals)
	
	def push(self, s):
		self.oldstdout = sys.stdout
		sys.stdout = self
		ret = code.InteractiveConsole.push(self, s)
		sys.stdout = self.oldstdout
		return ret
		
	def write(self, s):
		self.writeSig.emit(s)
	
class CommandLineWidget(QDockWidget, Ui_CommandLine):
	def __init__(self, parent = None):
		QDockWidget.__init__(self, parent)
		self.setupUi(self)
		
	
	def startInterp(self, win):
		self.runButton.clicked.connect(self.runPushed)
		self.interp = CommandLineInterp({'xpcb': xpcb, 'win': win})
		self.interp.writeSig.connect(self.write)
		self.write(">>> ")
		
	def runPushed(self):
		self.write(self.cmdEntry.text() + '\n')
		ret = self.interp.push(self.cmdEntry.text().__str__())
		self.cmdEntry.clear()
		if ret is True:
			self.write("... ")
		else:
			self.write(">>> ")
		
	def write(self, data):
		self.cmdHist.moveCursor(QtGui.QTextCursor.End)
		self.cmdHist.insertPlainText(data)

class PluginMain(xpcb.Plugin):
	def __init__(self):
		xpcb.Plugin.__init__(self)
	
	def installWidgets(self, win):
		self.clw = CommandLineWidget()
		win.addDockWidget(QtCore.Qt.BottomDockWidgetArea, self.clw)
		
	def initPlugin(self, win):
		self.clw.startInterp(win)

