import sys
sys.path.append("sip")
from PyQt4 import QtGui, QtCore, Qt
from PyQt4.QtCore import QObject, pyqtSignal, QPoint
from PyQt4.QtGui import QPainter, QDialog, QUndoCommand, QColor
import xpcb
import sip

class PadsNetlistLoader(xpcb.NetlistLoader):
	class State:
		Start, Header, PartSection, NetSection, InSignal, Done, Invalid = range(7)

	def __init__(self):
		xpcb.NetlistLoader.__init__(self)
		self.lineCount = 0
		self.state = self.State.Start
		self.currNetName = None
		self.currNetPins = []

	
	def loadFromFile(self, fp):
		self.netlist = xpcb.Netlist()
		fp.reset()
		self.lineCount = 0
		self.state = self.State.Start
		self.currNetName = None
		self.currNetPins = []
		while not fp.atEnd():
			line = fp.readLine()
			self.processLine(line)
			if self.state is self.State.Invalid:
				print 'Invalid file format'
				return None
		if self.state is not self.State.Done:
			print "Unexpected end of file"
			return None
		return self.netlist

	def processLine(self, line):
		line = str(line.trimmed())
		if not line or line.startswith('//'):
			return
		self.lineCount += 1
		if self.state is self.State.Start:
			if line.startswith('*PADS-PCB*') or line.startswith('*PADS2000*'):
				self.state = self.State.Header
			elif self.lineCount >= 3:
				print 'Did not find PADS header'
				self.state = self.State.Invalid
		elif self.state is self.State.Header:
			if line.startswith('*PART*'):
				self.state = self.State.PartSection
			else:
				print 'Did not find part section'
				self.state = self.State.Invalid
		elif self.state is self.State.PartSection:
			s = line.split()
			if not line.startswith('*') and len(s) == 2:
				self.netlist.addPart(s[0], s[1])
			elif line.startswith('*NET*'):
				self.state = self.State.NetSection 
			else:
				print 'Did not find net section'
				self.state = self.State.Invalid
		elif self.state in (self.State.NetSection, self.State.InSignal):
			s = line.split()
			if line.startswith('*'):
				if (line.startswith('*SIGNAL*') or line.startswith('*END*')) and self.currNetPins:
					# finish adding net
					self.netlist.addNet(self.currNetName, self.currNetPins)
				if line.startswith('*SIGNAL*') and len(s) == 2:
					self.currNetName = s[1]
					self.currNetPins = []
					self.state = self.State.InSignal
				elif line.startswith('*END*'):
					self.state = self.State.Done
				else:
					print 'Invalid command in net section'
					self.state = self.State.Invalid
			elif self.state is self.State.InSignal:
				self.currNetPins += [tuple(i.split('.')) for i in line.split()]
			else:
				print 'Unexpected content in net section'
				self.state = self.State.Invalid

class PluginMain(xpcb.Plugin):
	def __init__(self):
		xpcb.Plugin.__init__(self)
	
	def installWidgets(self, win):
		pass
		
	def initPlugin(self, win):
		self.loader = PadsNetlistLoader()
		xpcb.NetlistLoader.registerLoader(self.loader)
		
