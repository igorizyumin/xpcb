import sys
sys.path.append("sip")
from PyQt4 import QtGui, QtCore, Qt
from PyQt4.QtCore import QObject, pyqtSignal, QPoint
from PyQt4.QtGui import QPainter, QDialog, QUndoCommand
import xpcb
from xpcb import Layer, XPcb
from ui_EditLineDialog import Ui_EditLineDialog
import sip

class LineEditDialog(QDialog, Ui_EditLineDialog):
	def __init__(self, parent = None):
		QDialog.__init__(self, parent)
		self.setupUi(self)
		self.inMM = False

		
	def toUnits(self, pcbu):
		if self.inMM:
			return XPcb.PCB2MM(pcbu)
		else:
			return XPcb.PCB2MIL(pcbu)
		
	def toPcb(self, unit):
		if self.inMM:
			return XPcb.MM2PCB(unit)
		else:
			return XPcb.MIL2PCB(unit)
			
	def updateUnits(self):
		if (self.inMM):
			self.widthBox.setDecimals(3)
			self.widthBox.setValue(XPcb.PCB2MM(XPcb.MIL2PCB(self.widthBox.value())))
			self.widthBox.setSuffix(" mm")
		else:
			self.widthBox.setValue(XPcb.PCB2MIL(XPcb.MM2PCB(self.widthBox.value())))
			self.widthBox.setDecimals(0)
			self.widthBox.setSuffix(" mil")
	
	def on_unitsBox_currentIndexChanged(self, s):
		inmm = (s == 'mm')
		if (inmm != self.inMM):
			self.inMM = inmm
			self.updateUnits()
			
	def init(self, line):
		self.widthBox.setValue(self.toUnits(line.width()))
	
	def width(self):
		return self.toPcb(self.widthBox.value())

class LineEditor(xpcb.AbstractEditor):
	class State:
		Selected, VtxMove, LineMove, LineNewFirst, LineNewSecond = range(5)
		
	def __init__(self, ctrl, line = None):
		xpcb.AbstractEditor.__init__(self, ctrl)
		self.ctrl = ctrl
		self.line = line
		if line is not None:
			ctrl.hideObj(line)
		self.state = self.State.Selected
		self.pos = QPoint()
		self.mode = xpcb.Line.LINE
		self.width = 0
		# actions
		self.actLine = xpcb.CtrlAction(0, "Straight Line")
		self.actLine.execFired.connect(self.setLine)
		self.actArcCW = xpcb.CtrlAction(1, "Arc (CW)")
		self.actArcCW.execFired.connect(self.setArcCW)
		self.actArcCCW = xpcb.CtrlAction(2, "Arc (CCW)")
		self.actArcCCW.execFired.connect(self.setArcCCW)
		self.actEdit = xpcb.CtrlAction(0, "Edit Segment")
		self.actMove = xpcb.CtrlAction(3, "Move Segment")
		self.actDel = xpcb.CtrlAction(7, "Delete Segment")
		self.actDel.execFired.connect(self.delLine)
	
	def setLine(self):
		self.mode = xpcb.Line.LINE
		self.line.setType(self.mode)
		self.overlayChanged.emit()
	
	def setArcCW(self):
		self.mode = xpcb.Line.ARC_CW
		self.line.setType(self.mode)
		self.overlayChanged.emit()
		
	def setArcCCW(self):
		self.mode = xpcb.Line.ARC_CCW
		self.line.setType(self.mode)
		self.overlayChanged.emit()
		
	def delLine(self):
		c = DeleteLineCmd(None, self.ctrl.doc(), self.line)
		self.ctrl.doc().doCommand(c)
		self.editorFinished.emit()
	
	def drawOverlay(self, painter):
		if self.state in (self.State.LineNewFirst, self.State.LineNewSecond):
			painter.save()
			painter.setRenderHint(QPainter.Antialiasing, False)
			painter.setBrush(Qt.Qt.NoBrush)
			painter.translate(self.pos)
			imax = 2147483647
			painter.drawLine(QPoint(0, -imax), QPoint(0, imax))
			painter.drawLine(QPoint(-imax, 0), QPoint(imax, 0))
			painter.restore()
		if self.line is None:
			return
		if self.state is self.State.LineNewSecond:
			self.line.draw(painter, Layer(Layer.LAY_SELECTION))
		if self.state is self.State.Selected:
			self.line.draw(painter, Layer(Layer.LAY_SELECTION))
#			painter.save()
#			painter.setRenderHint(QPainter.Antialiasing, False)
#			p = painter.pen()
#			p.setWidth(0)
#			painter.setPen(p)
#			painter.drawRect(self.line.bbox())
#			painter.restore()
	
	def newLine(self):
		self.line = xpcb.Line()
		self.line.setLayer(Layer(Layer.LAY_SILK_TOP))
		self.line.setWidth(self.width)
		self.line.setType(self.mode)
		
	def init(self):
		if self.line is None:
			d = LineEditDialog(self.ctrl.view())
			ret = d.exec_()
			if (ret == QDialog.Rejected):
				self.editorFinished.emit()
				return
			self.width = d.width()
			self.newLine()
			self.state = self.State.LineNewFirst
			self.actionsChanged.emit()

			
	def actions(self):
		if self.state is self.State.LineNewSecond:
			return [self.actLine, self.actArcCW, self.actArcCCW]
		elif self.state is self.State.Selected:
			return [self.actEdit, self.actMove, self.actDel]
		return []
	
	def mouseMoveEvent(self, event):
		if self.state in (self.State.VtxMove, self.State.LineMove, self.State.LineNewFirst, self.State.LineNewSecond):
			self.pos = self.ctrl.snapToPlaceGrid(self.ctrl.view().transform().inverted()[0].map(event.pos()))
			if self.state is self.State.LineNewSecond:
				self.line.setEnd(self.pos)
			self.overlayChanged.emit()
		
	def mousePressEvent(self, event):
		if self.state is self.State.Selected:
			event.ignore()
			return
		event.accept()
		
	def mouseReleaseEvent(self, event):
		if self.state is self.State.Selected:
			event.ignore()
			return
		elif self.state is self.State.LineNewFirst:
			self.line.setStart(self.pos)
			self.line.setEnd(self.pos)
			self.state = self.State.LineNewSecond
			self.actionsChanged.emit()
			self.overlayChanged.emit()
		elif self.state is self.State.LineNewSecond:
			self.line.setEnd(self.pos)
			if (self.line.start() != self.line.end()):
				c = NewLineCmd(None, self.ctrl.doc(), self.line)
				self.ctrl.doc().doCommand(c)
			self.newLine()
			self.line.setStart(self.pos)
			self.line.setEnd(self.pos)
			self.overlayChanged.emit()
		event.accept()
		
	def keyPressEvent(self, event):
		if (event.key() == Qt.Qt.Key_Escape and self.state is not self.State.Selected):
			self.editorFinished.emit()

class NewLineCmd(QUndoCommand):
	def __init__(self, parent, doc, line):
		QUndoCommand.__init__(self, parent)
		self.doc = sip.cast(doc, xpcb.FPDoc)
		self.line = line
		
	def undo(self):
		self.doc.removeLine(self.line)
	
	def redo(self):
		self.doc.addLine(self.line)
		
class DeleteLineCmd(QUndoCommand):
	def __init__(self, parent, doc, line):
		QUndoCommand.__init__(self, parent)
		self.doc = sip.cast(doc, xpcb.FPDoc)
		self.line = line
		
	def undo(self):
		self.doc.addLine(self.line)
	
	def redo(self):
		self.doc.removeLine(self.line)

class LineEditorFactory(xpcb.AbstractEditorFactory):
	def makeEditor(self, ctrl, obj):
		return LineEditor(ctrl, sip.cast(obj, xpcb.Line))
		
class PluginMain(xpcb.Plugin):
	def __init__(self):
		xpcb.Plugin.__init__(self)
	
	def installWidgets(self, win):
		pass
		
	def initPlugin(self, win):
		self.factory = LineEditorFactory()
		xpcb.EditorFactory.registerFactory(xpcb.EditorFactory.ObjLine, self.factory)
		if isinstance(win, xpcb.FPEditWindow):
			self.action = xpcb.CtrlAction(6, 'New Line')
			self.action.execFired.connect(self.startEditor)
			self.win = win
			win.ctrl().registerAction(self.action)

	def startEditor(self):
		ed = LineEditor(self.win.ctrl())
		self.win.ctrl().installEditor(ed)
		
