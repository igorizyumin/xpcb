import sys
sys.path.append("sip")
from PyQt4 import QtGui, QtCore, Qt
from PyQt4.QtCore import QObject, pyqtSignal, QPoint
from PyQt4.QtGui import QPainter, QDialog, QUndoCommand, QColor
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
		if line.layer() == Layer(Layer.LAY_SILK_TOP):
			self.layerBox.setCurrentIndex(0)
		else:
			self.layerBox.setCurrentIndex(1)
	
	def width(self):
		return self.toPcb(self.widthBox.value())

	def layer(self):
		if self.layerBox.currentIndex() == 0:
			return Layer(Layer.LAY_SILK_TOP)
		else:
			return Layer(Layer.LAY_SILK_BOTTOM)

class LineEditor(xpcb.AbstractEditor):
	class State:
		Selected, VtxSelStart, VtxSelEnd, VtxMoveStart, VtxMoveEnd, PickRef, LineMove, LineNewFirst, LineNewSecond = range(9)
		
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
		self.layer = Layer(Layer.LAY_SILK_TOP)
		# actions
		self.actLine = xpcb.CtrlAction(4, "Straight Line")
		self.actLine.execFired.connect(self.setLine)
		self.actArcCW = xpcb.CtrlAction(5, "Arc (CW)")
		self.actArcCW.execFired.connect(self.setArcCW)
		self.actArcCCW = xpcb.CtrlAction(6, "Arc (CCW)")
		self.actArcCCW.execFired.connect(self.setArcCCW)
		self.actEdit = xpcb.CtrlAction(0, "Edit Segment")
		self.actEdit.execFired.connect(self.editSegment)
		self.actMove = xpcb.CtrlAction(3, "Move Segment")
		self.actMove.execFired.connect(self.startMoveLine)
		self.actDel = xpcb.CtrlAction(7, "Delete Segment")
		self.actDel.execFired.connect(self.delLine)
		self.actMoveVtx = xpcb.CtrlAction(3, "Move Vertex")
		self.actMoveVtx.execFired.connect(self.startMoveVtx)

	def setLineType(self, ltype):
		if self.state is self.State.Selected:
			prevState = self.line.getState()
		else:
			self.mode = ltype
		self.line.setType(ltype)
		self.overlayChanged.emit()
		if self.state is self.State.Selected:
			c = EditLineCmd(None, self.line, prevState)
			self.ctrl.doc().doCommand(c)

	def editSegment(self):
		prevState = self.line.getState()
		d = LineEditDialog(self.ctrl.view())
		d.init(self.line)
		ret = d.exec_()
		if (ret == QDialog.Rejected):
			self.editorFinished.emit()
			return
		self.line.setWidth(d.width())
		self.line.setLayer(d.layer())
		c = EditLineCmd(None, self.line, prevState)
		self.ctrl.doc().doCommand(c)
		self.overlayChanged.emit()

	def setLine(self):
		self.setLineType(xpcb.Line.LINE)
	
	def setArcCW(self):
		self.setLineType(xpcb.Line.ARC_CW)
		
	def setArcCCW(self):
		self.setLineType(xpcb.Line.ARC_CCW)
		
	def delLine(self):
		c = DeleteLineCmd(None, self.ctrl.doc(), self.line)
		self.ctrl.doc().doCommand(c)
		self.editorFinished.emit()

	def startMoveVtx(self):
		if self.state is self.State.VtxSelStart:
			self.state = self.State.VtxMoveStart
			self.pos = self.line.start()
		else:
			self.state = self.State.VtxMoveEnd
			self.pos = self.line.end()
		self.prevState = self.line.getState()

	def startMoveLine(self):
		self.prevState = self.line.getState()
		self.state = self.State.PickRef
		self.delta = QPoint()
	
	def drawOverlay(self, painter):
		if self.state in (self.State.LineNewFirst, self.State.LineNewSecond, self.State.VtxMoveStart, self.State.VtxMoveEnd, self.State.PickRef, self.State.LineMove):
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
		if self.state in (self.State.LineNewSecond, self.State.VtxMoveStart, self.State.VtxMoveEnd, self.State.PickRef):
			self.line.draw(painter, Layer(Layer.LAY_SELECTION))
		elif self.state is self.State.LineMove:
			painter.save()
			painter.translate(self.delta)
			self.line.draw(painter, Layer(Layer.LAY_SELECTION))
			painter.restore()
		elif self.state in (self.State.Selected, self.State.VtxSelStart, self.State.VtxSelEnd):
			self.line.draw(painter, Layer(Layer.LAY_SELECTION))
			if self.state in (self.State.VtxSelStart, self.State.VtxSelEnd):
				# calculate the size of a handle (10px to PCB coords)
				hs = 10.0/self.ctrl.view().transform().m11()
				if self.state is self.State.VtxSelStart:
					pos = self.line.start()
				else:
					pos = self.line.end()
				# draw the handle
				painter.save()
				painter.setRenderHint(QPainter.Antialiasing, False)
				painter.setBrush(QColor(255,0,0))
				p = painter.pen()
				p.setWidth(0)
				p.setColor(QColor(255,0,0))
				painter.setPen(p)
				painter.drawRect(pos.x()-hs/2.0, pos.y()-hs/2.0, hs, hs)
				painter.restore()

	
	def newLine(self):
		self.line = xpcb.Line()
		self.line.setLayer(Layer(Layer.LAY_SILK_TOP))
		self.line.setWidth(self.width)
		self.line.setLayer(self.layer)
		self.line.setType(self.mode)
		
	def init(self):
		if self.line is None:
			d = LineEditDialog(self.ctrl.view())
			ret = d.exec_()
			if (ret == QDialog.Rejected):
				self.editorFinished.emit()
				return
			self.width = d.width()
			self.layer = d.layer()
			self.newLine()
			self.state = self.State.LineNewFirst
			self.actionsChanged.emit()

			
	def actions(self):
		if self.state is self.State.LineNewSecond:
			return [self.actLine, self.actArcCW, self.actArcCCW]
		elif self.state is self.State.Selected:
			return [self.actEdit, self.actMove, self.actDel, self.actLine, self.actArcCW, self.actArcCCW]
		elif self.state in (self.State.VtxSelStart, self.State.VtxSelEnd):
			return [self.actMoveVtx]
		return []
	
	def mouseMoveEvent(self, event):
		if self.state in (self.State.PickRef, self.State.LineMove, self.State.LineNewFirst, self.State.LineNewSecond, self.State.VtxMoveStart, self.State.VtxMoveEnd):
			self.pos = self.ctrl.snapToPlaceGrid(self.ctrl.view().transform().inverted()[0].map(event.pos()))
			if self.state is self.State.VtxMoveStart:
				self.line.setStart(self.pos)
			elif self.state in (self.State.LineNewSecond, self.State.VtxMoveEnd):
				self.line.setEnd(self.pos)
			elif self.state is self.State.LineMove:
				self.delta = self.pos - self.refPt
			self.overlayChanged.emit()
		
	def mousePressEvent(self, event):
		if self.state in (self.State.Selected, self.State.VtxSelStart, self.State.VtxSelEnd):
			# check if a vertex was hit
			start = self.ctrl.view().transform().map(self.line.start())
			end = self.ctrl.view().transform().map(self.line.end())
			if (start - event.pos()).manhattanLength() <= 20:
				event.accept()
			elif (end - event.pos()).manhattanLength() <= 20:
				event.accept()
			elif (self.state is not self.State.Selected):
				pos = self.ctrl.view().transform().inverted()[0].map(event.pos())
				if self.line.testHit(pos, self.line.layer()):
					event.accept()
				else:
					event.ignore()
			else:
				event.ignore()
		else:
			event.accept()
		
	def mouseReleaseEvent(self, event):
		if self.state in (self.State.Selected, self.State.VtxSelStart, self.State.VtxSelEnd):
			# check if a vertex was hit
			start = self.ctrl.view().transform().map(self.line.start())
			end = self.ctrl.view().transform().map(self.line.end())
			if (start - event.pos()).manhattanLength() <= 20:
				self.state = self.State.VtxSelStart
				event.accept()
				self.actionsChanged.emit()
				self.overlayChanged.emit()
			elif (end - event.pos()).manhattanLength() <= 20:
				self.state = self.State.VtxSelEnd
				event.accept()
				self.actionsChanged.emit()
				self.overlayChanged.emit()
			elif (self.state is not self.State.Selected):
				pos = self.ctrl.view().transform().inverted()[0].map(event.pos())
				if self.line.testHit(pos, self.line.layer()):
					self.state = self.State.Selected
					event.accept()
					self.actionsChanged.emit()
					self.overlayChanged.emit()
				else:
					event.ignore()
			else:
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
		elif self.state in (self.State.VtxMoveStart, self.State.VtxMoveEnd):
			c = EditLineCmd(None, self.line, self.prevState)
			self.ctrl.doc().doCommand(c)
			if self.state is self.State.VtxMoveStart:
				self.state = self.State.VtxSelStart
			else:
				self.state = self.State.VtxSelEnd
			self.overlayChanged.emit()
			self.actionsChanged.emit()
		elif self.state is self.State.PickRef:
			self.refPt = self.pos
			self.state = self.State.LineMove
			self.overlayChanged.emit()
			self.actionsChanged.emit()
		elif self.state is self.State.LineMove:
			self.line.setStart(self.line.start() + self.delta)
			self.line.setEnd(self.line.end() + self.delta)
			c = EditLineCmd(None, self.line, self.prevState)
			self.ctrl.doc().doCommand(c)
			self.state = self.State.Selected
			self.overlayChanged.emit()
			self.actionsChanged.emit()
		event.accept()
		
	def keyPressEvent(self, event):
		if (event.key() == Qt.Qt.Key_Escape and self.state is not self.State.Selected):
			if self.state in (self.State.VtxMoveStart, self.State.VtxMoveEnd):
				self.line.loadState(self.prevState)
			self.editorFinished.emit()

class NewLineCmd(QUndoCommand):
	def __init__(self, parent, doc, line):
		QUndoCommand.__init__(self, parent)
		self.doc = doc
		self.line = line
		
	def undo(self):
		self.doc.removeLine(self.line)
	
	def redo(self):
		self.doc.addLine(self.line)
		
class DeleteLineCmd(QUndoCommand):
	def __init__(self, parent, doc, line):
		QUndoCommand.__init__(self, parent)
		self.doc = doc
		self.line = line
		
	def undo(self):
		self.doc.addLine(self.line)
	
	def redo(self):
		self.doc.removeLine(self.line)

class EditLineCmd(QUndoCommand):
	def __init__(self, parent, line, prevState):
		QUndoCommand.__init__(self, parent)
		self.line = line
		self.prevState = prevState
		self.currState = line.getState()
		
	def undo(self):
		self.line.loadState(self.prevState)
	
	def redo(self):
		self.line.loadState(self.currState)

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
			self.action = xpcb.CtrlAction(2, 'Add Line')
			self.action.execFired.connect(self.startEditor)
			self.win = win
			win.ctrl().registerAction(self.action)

	def startEditor(self):
		ed = LineEditor(self.win.ctrl())
		self.win.ctrl().installEditor(ed)
		
