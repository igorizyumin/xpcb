import sys
import math
sys.path.append("sip")
from PyQt4 import QtGui, QtCore, Qt
from PyQt4.QtCore import QObject, pyqtSignal, QPoint
from PyQt4.QtGui import QPainter, QDialog, QUndoCommand, QColor
import xpcb
from xpcb import Layer, XPcb
import sip

def sign(x):
	if x > 0:
		return 1
	return -1

def perp(vec):
	return QPoint(-vec.y(), vec.x())

def dotProd(pt1, pt2):
	return pt1.x() * pt2.x() + pt1.y() * pt2.y()

# finds the intersection of the two lines given by (pt1, dir1) and (pt2, dir2)
# ptN = point on line, dirN = direction vector
# returns scale factor for dir1 vector (intersection pt = pt1 + return value * dir1)
def lineIntersect(pt1, dir1, pt2, dir2):
	# check for collinearity
	if (dotProd(dir1, perp(dir2)) == 0):
		return None
	w = pt1 - pt2
	return (1.0*dir2.y()*w.x() - dir2.x()*w.y())/(1.0*dir2.x()*dir1.y()-dir2.y()*dir1.x())

# finds the intersection of the two lines given by (pt1, dir1) and (pt2, dir2)
# ptN = point on line, dirN = direction vector
# returns intersection point
def lineIntersectPt(pt1, dir1, pt2, dir2):
	return pt1 + dir1 * lineIntersect(pt1, dir1, pt2, dir2)

def vecLen(vec):
	return math.sqrt(pow(1.0*vec.x(),2) + pow(1.0*vec.y(),2))

class NewTraceEditor(xpcb.AbstractEditor):
	class State:
		PickStart, PickEnd = range(2)
	class Mode:
		Mode45Straight, ModeStraight45 = range(2)
		
	def __init__(self, ctrl):
		xpcb.AbstractEditor.__init__(self, ctrl)
		self.ctrl = ctrl
		self.state = self.State.PickStart
		self.mode = self.Mode.ModeStraight45
		self.pos = None
		self.startPt = None
		self.vtxStart = None
		self.vtxMid = None
		self.vtxEnd = None
		self.seg1 = None
		self.seg2 = None
		self.width = XPcb.MIL2PCB(10)
		self.layer = Layer(Layer.LAY_TOP_COPPER)
	
	def drawOverlay(self, painter):
		# draw 45 degree crosshair
		if self.pos is not None:
			painter.save()
			painter.setRenderHint(QPainter.Antialiasing, False)
			painter.setBrush(Qt.Qt.NoBrush)
			p = painter.pen()
			p.setStyle(Qt.Qt.DashLine)
			p.setColor(QColor(128, 128, 128))
			painter.setPen(p)
			painter.translate(self.pos)
			imax = 2147483647
			painter.drawLine(QPoint(0, -imax), QPoint(0, imax))
			painter.drawLine(QPoint(-imax, -imax), QPoint(imax, imax))
			painter.drawLine(QPoint(-imax, 0), QPoint(imax, 0))
			painter.drawLine(QPoint(-imax, imax), QPoint(imax, -imax))
			painter.restore()

		if self.state is self.State.PickEnd:
			# draw rubberband segments
			self.seg1.draw(painter, self.layer)
			self.seg2.draw(painter, self.layer)
			self.vtxStart.draw(painter, self.layer)
			self.vtxMid.draw(painter, self.layer)
			self.vtxEnd.draw(painter, self.layer)

	def init(self):
		self.actionsChanged.emit()

			
	def actions(self):
		return []
	
	def updateDogleg(self):
		start = self.vtxStart.pos()
		end = self.vtxEnd.pos()
		d = end - start
		if self.mode is self.Mode.ModeStraight45:
			if abs(d.x()) > abs(d.y()):
				self.vtxMid.setPos(QPoint(end.x() - sign(d.x())*abs(d.y()), start.y()))
			else:
				self.vtxMid.setPos(QPoint(start.x(), end.y() - sign(d.y())*abs(d.x())))
		elif self.mode is self.Mode.Mode45Straight:
			if abs(d.x()) > abs(d.y()):
				self.vtxMid.setPos(QPoint(start.x() + sign(d.x())*abs(d.y()), end.y()))
			else:
				self.vtxMid.setPos(QPoint(end.x(), start.y() + sign(d.y())*abs(d.x())))
	
	def toggleMode(self):
		if self.mode is self.Mode.Mode45Straight:
			self.mode = self.Mode.ModeStraight45
		elif self.mode is self.Mode.ModeStraight45:
			self.mode = self.Mode.Mode45Straight

	def mouseMoveEvent(self, event):
		self.pos = self.ctrl.snapToRouteGrid(self.ctrl.view().transform().inverted()[0].map(event.pos()))
		# TODO snap to pins and vertices
		if self.state is self.State.PickEnd:
			# update rubberband segments
			self.vtxEnd.setPos(self.pos)
			self.updateDogleg()
		self.overlayChanged.emit()
		
	def mousePressEvent(self, event):
		event.accept()
		
	def mouseReleaseEvent(self, event):
		if self.state is self.State.PickStart:
			# TODO check if a segment or vertex is already present at this location
			# set start vertex, create rubberband segs/vtxs
			self.vtxStart = xpcb.Vertex(self.pos)
			self.vtxMid = xpcb.Vertex(self.pos)
			self.vtxEnd = xpcb.Vertex(self.pos)
			self.seg1 = xpcb.Segment(self.vtxStart, self.vtxMid, self.layer, self.width)
			self.seg2 = xpcb.Segment(self.vtxMid, self.vtxEnd, self.layer, self.width)
			self.state = self.State.PickEnd
		elif self.state is self.State.PickEnd:
			# TODO add first segment, update start vtx,, move second->first, create new seg/vtx
			cmd = NewSegmentCmd(None, self.ctrl.doc(), self.seg1)
			self.seg1 = self.seg2
			self.vtxStart = self.vtxMid
			self.vtxMid = self.vtxEnd
			self.vtxEnd = xpcb.Vertex(self.pos)
			self.seg2 = xpcb.Segment(self.vtxMid, self.vtxEnd, self.layer, self.width)
			self.ctrl.doc().doCommand(cmd)
			self.toggleMode()
		event.accept()

		
	def keyPressEvent(self, event):
		if (event.key() == Qt.Qt.Key_Escape):
			self.editorFinished.emit()

	

class NewSegmentCmd(QUndoCommand):
	def __init__(self, parent, doc, seg):
		QUndoCommand.__init__(self, parent)
		self.doc = sip.cast(doc, xpcb.PCBDoc)
		self.segment = seg

	def undo(self):
		self.doc.traceList().removeSegment(self.segment)

	def redo(self):
		self.doc.traceList().addSegment(self.segment)

	 
class SegmentEditor(xpcb.AbstractEditor):
	class State:
		Selected, Move, AddVtx = range(3)
		
	def __init__(self, ctrl, seg):
		xpcb.AbstractEditor.__init__(self, ctrl)
		self.ctrl = ctrl
		self.segment = seg
		self.ctrl.hideObj(self.segment)
		self.state = self.State.Selected
		self.pos = None
		self.actMove = xpcb.CtrlAction(3, "Move Segment")
		self.actMove.execFired.connect(self.startMoveSegment)
	
	# returns false if segment cannot be moved
	def startMoveSegment(self):
		# get the neighboring segments
		# get the segment vertices
		v1 = self.segment.v1()
		v2 = self.segment.v2()
		# find the other segments
		v1s = v1.segments()
		v1s.remove(self.segment)
		v2s = v2.segments()
		v2s.remove(self.segment)
		if len(v1s) > 1 or len(v2s) > 1:
			# more than 1 neighbor, cannot be moved
			return False
		
		self.pt1 = v1.pos()
		self.pt2 = v2.pos()
		# secant
		self.vec_sec = self.pt2 - self.pt1
		
		# get the neighboring segments, the outer (fixed) vertices, and the direction vectors
		if len(v1s) == 0:
			# no neighboring segment
			self.seg1 = None
			self.pt1_fixed = None
			self.vector1 = None
		else:
			self.seg1 = v1s[0]
			self.ctrl.hideObj(self.seg1)
			self.pt1_fixed = self.seg1.otherVertex(v1).pos()
			self.vector1 = self.pt1 - self.pt1_fixed
		if len(v2s) == 0:
			self.seg2 = None
			self.pt2_fixed = None 
			self.vector2 = None
		else:
			self.seg2 = v2s[0]
			self.ctrl.hideObj(self.seg2)
			self.pt2_fixed = self.seg2.otherVertex(v2).pos()
			self.vector2 = self.pt2 - self.pt2_fixed
		# save the signs
		self.signx = sign((self.pt2 - self.pt1).x())
		self.signy = sign((self.pt2 - self.pt1).y())
		self.state = self.State.Move
		self.actionsChanged.emit()

	def updateMove(self):
		# mouse is at self.pos
		ok = True
		if self.vector1 is not None:
			# find the intersection of vec1 and the secant
			sc = lineIntersect(self.pt1_fixed, self.vector1, self.pos, self.vec_sec)
			newpt1 = self.pt1_fixed + self.vector1 * sc
			if (sc < 0):
				ok = False
		else:
			# otherwise just translate the point in a direction perpendicular to the line
			newpt1 = lineIntersectPt(self.pt1, perp(self.vec_sec), self.pos, self.vec_sec)
		if self.vector2 is not None:
			sc = lineIntersect(self.pt2_fixed, self.vector2, self.pos, self.vec_sec)
			newpt2 = self.pt2_fixed + self.vector2 * sc
			if (sc < 0):
				ok = False
		else:
			newpt2 = lineIntersectPt(self.pt2, perp(self.vec_sec), self.pos, self.vec_sec)
		if (self.signx != sign((newpt2 - newpt1).x()) or self.signy != sign((newpt2 - newpt1).y())):
			ok = False
		if ok:
			self.pt1 = newpt1
			self.pt2 = newpt2

	def drawOverlay(self, painter):
		# draw 45 degree crosshair when in add vertex mode
		if self.state in [self.State.AddVtx, self.State.Move] and self.pos is not None:
			painter.save()
			painter.setRenderHint(QPainter.Antialiasing, False)
			painter.setBrush(Qt.Qt.NoBrush)
			p = painter.pen()
			p.setStyle(Qt.Qt.DashLine)
			p.setColor(QColor(128, 128, 128))
			painter.setPen(p)
			painter.translate(self.pos)
			imax = 2147483647
			painter.drawLine(QPoint(0, -imax), QPoint(0, imax))
			painter.drawLine(QPoint(-imax, -imax), QPoint(imax, imax))
			painter.drawLine(QPoint(-imax, 0), QPoint(imax, 0))
			painter.drawLine(QPoint(-imax, imax), QPoint(imax, -imax))
			painter.restore()
		if self.state is self.State.Selected:
			self.segment.draw(painter, Layer(Layer.LAY_SELECTION))
		elif self.state is self.State.Move:
			# draw line segments
			if self.pt1_fixed is not None:
				painter.drawLine(self.pt1_fixed, self.pt1)
			painter.drawLine(self.pt1, self.pt2)
			if self.pt2_fixed is not None:
				painter.drawLine(self.pt2_fixed, self.pt2)

	def init(self):
		self.actionsChanged.emit()
			
	def actions(self):
		if self.state is self.State.Selected:
			return [self.actMove]
		return []
	
	def mouseMoveEvent(self, event):
		self.pos = self.ctrl.snapToRouteGrid(self.ctrl.view().transform().inverted()[0].map(event.pos()))
		if self.state is self.State.Move:
			# update rubberband segments
			self.updateMove()
			self.overlayChanged.emit()
		
	def mousePressEvent(self, event):
		event.accept()
		
	def mouseReleaseEvent(self, event):
		event.accept()

		
	def keyPressEvent(self, event):
		if (event.key() == Qt.Qt.Key_Escape):
			self.editorFinished.emit()

class SegmentEditorFactory(xpcb.AbstractEditorFactory):
	def makeEditor(self, ctrl, obj):
		return SegmentEditor(ctrl, sip.cast(obj, xpcb.Segment))

class PluginMain(xpcb.Plugin):
	def __init__(self):
		xpcb.Plugin.__init__(self)
	
	def installWidgets(self, win):
		pass
		
	def initPlugin(self, win):
		self.factory = SegmentEditorFactory()
		xpcb.EditorFactory.registerFactory(xpcb.EditorFactory.ObjSegment, self.factory)
		if isinstance(win, xpcb.PCBEditWindow):
			self.action = xpcb.CtrlAction(1, 'Add Trace')
			self.action.execFired.connect(self.startEditor)
			self.win = win
			win.ctrl().registerAction(self.action)

	def startEditor(self):
		ed = NewTraceEditor(self.win.ctrl())
		self.win.ctrl().installEditor(ed)
		
