import sys
sys.path.append("sip")
from PyQt4 import QtGui, QtCore, Qt
from PyQt4.Qt import Qt
from PyQt4.QtCore import QObject, pyqtSignal, QAbstractItemModel, QModelIndex, QPersistentModelIndex, QRectF, QSettings
from PyQt4.QtGui import QDialog, QWidget, QPainter, QTransform, QBrush, QPen, QSizePolicy, QPolygonF, QVBoxLayout, QMessageBox
import xpcb
from xpcb import Layer, AbstractSelFPDialogFactory
from Ui_SelectFP import Ui_SelectFPDialog
import sip

class SelectFPDialog(xpcb.AbstractSelFPDialog, Ui_SelectFPDialog):
	def __init__(self, parent = None):
		xpcb.AbstractSelFPDialog.__init__(self, parent)
		self.setupUi(self)
		self.model = FPTreeModel()
		self.fpTree.setModel(self.model)
		self.fpTree.selectionModel().currentChanged.connect(self.onObjSelected)
		self.preview = FPPreview()
		self.pvlayout = QVBoxLayout()
		self.pvlayout.addWidget(self.preview)
		self.PreviewWidget.setLayout(self.pvlayout)
		self._fpidx = None
		self.loadGeom()

	def onObjSelected(self, curr, prev):
		self.preview.showFP(self.model.path(curr))

	def path(self):
		return self.model.path(self._fpidx)

	def makeFootprint(self):
		return self.model.makeFootprint(self._fpidx)

	def fpSelected(self):
		return self._fpidx is not None

	def uuid(self):
		return self.model.uuid(self._fpidx)

	def accept(self):
		if self.fpTree.selectionModel().hasSelection() and self.fpTree.selectionModel().currentIndex().internalPointer().isFolder() is False:
			QDialog.accept(self)
			self._fpidx = QPersistentModelIndex(self.fpTree.selectionModel().currentIndex())
		else:
			mb = QMessageBox(QMessageBox.Warning, "No footprint selected", "You have not selected a footprint.  Please make a selection, then try again.", QMessageBox.Ok, self)
			mb.exec_()


	def closeEvent(self, event):
		self.saveGeom()
		QDialog.closeEvent(self, event)
	
	def hideEvent(self, event):
		self.saveGeom()
		QDialog.hideEvent(self, event)


	def saveGeom(self):
		s = QSettings()
		s.setValue("fpselwindow/geometry", self.saveGeometry())
		s.setValue("fpselwindow/splitterState", self.splitter.saveState())
		s.setValue("fpselwindow/treeHeaderState", self.fpTree.header().saveState())

	def loadGeom(self):
		s = QSettings()
		self.restoreGeometry(s.value("fpselwindow/geometry").toByteArray())
		self.splitter.restoreState(s.value("fpselwindow/splitterState").toByteArray())
		self.fpTree.header().restoreState(s.value("fpselwindow/treeHeaderState").toByteArray())

class FPPreview(QWidget):
	def __init__(self, parent = None):
		QWidget.__init__(self, parent)
		self._doc = None
		self._transform = QTransform()
		self.setSizePolicy(QSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding))
		
	def showFP(self, path):
		print "show fp: " + str(path)
		self._doc = xpcb.FPDoc()
		d = sip.cast(self._doc, xpcb.Document)
		if path is None or d is None or not d.loadFromFile(path):
			self._doc = None
			self.update()
			return
		# update transform
		self._bb = (self._doc.footprint().bbox() | self._doc.footprint().getRefText().bbox() | self._doc.footprint().getValueText().bbox()).normalized();
		bb = QRectF(self._bb)
		rect = QRectF(self.rect())
		wscale = bb.width() / rect.width()
		hscale = bb.height() / rect.height()
		if wscale > hscale:
			# width is the limiting factor
			w = rect.width()
			h = bb.height() / wscale
			hoff = (rect.height() - h) / 2
			mrect = QRectF(0, hoff, w, h).normalized()
		else:
			# height is the limiting factor
			w = bb.width() / hscale
			h = rect.height()
			woff = (rect.width() - w) / 2
			mrect = QRectF(woff, 0, w, h).normalized()
		# need to do this to flip vertical direction; otherwise, it is wrong.
		self._transform = QTransform()
		l = list(QPolygonF(mrect)[0:4])
		l.reverse()
		QTransform.quadToQuad(QPolygonF(bb)[0:4], QPolygonF(l), self._transform)
		self.update()


	def paintEvent(self, event):
		painter = QPainter(self)
		# erase backgnd
		painter.setBackground(QBrush(Layer.color(Layer.LAY_BACKGND)))
		painter.setClipping(True)
		painter.eraseRect(self.rect())
		if self._doc is not None:
			layers = self._doc.layerList(xpcb.Document.DrawPriorityOrder)
			painter.setRenderHint(QPainter.Antialiasing)
			painter.setTransform(self._transform)
			objs = self._doc.findObjs(self._bb)
			for l in layers:
				if l == Layer(Layer.LAY_SELECTION):
					continue
				p = painter.pen()
				p.setColor(l.color())
				p.setWidth(0)
				painter.setPen(p)
				b = painter.brush()
				b.setColor(l.color())
				b.setStyle(Qt.SolidPattern)
				painter.setBrush(b)
				for o in objs:
					o.draw(painter, l)
		painter.end()

class TreeItem(object):
	def __init__(self, fpobj, row = 0, parent = None):
		self._fpobj = fpobj
		self._parent = parent
		self._children = []
		self._row = row
		# recursively wrap all child items
		if type(self._fpobj) is xpcb.FPDBFolder:
			for row, obj in enumerate(self._fpobj.folders() + self._fpobj.items()):
				self.appendChild(TreeItem(obj, row, self))

	def isFolder(self):
		return type(self._fpobj) is xpcb.FPDBFolder

	def row(self):
		return self._row 
	
	def child(self, row):
		return self._children[row]
	
	def childCount(self):
		return len(self._children)

	def columnCount(self):
		return 4

	def data(self, col):
		if type(self._fpobj) is xpcb.FPDBFolder and col == 0:
			return self._fpobj.name()
		if type(self._fpobj) is xpcb.FPDBFile:
			if col == 0:
				return self._fpobj.name()
			elif col == 1:
				return self._fpobj.author()
			elif col == 2:
				return self._fpobj.source()
			elif col == 3:
				return self._fpobj.desc()
		if self._fpobj is None:
			return "Root item"
		return None

	def path(self):
		if self.isFolder():
			return None
		return self._fpobj.path()

	def uuid(self):
		if self.isFolder():
			return None
		return self._fpobj.uuid()

	def makeFootprint():
		if self.isFolder():
			return None
		return self._fpobj.loadFootprint()

	def parent(self):
		return self._parent

	def appendChild(self, child):
		self._children.append(child)

class FPTreeModel(QAbstractItemModel):
	def __init__(self, parent = None):
		QAbstractItemModel.__init__(self, parent)
		self.rootitem = TreeItem(None)
		for row, f in enumerate(xpcb.FPDatabase.instance().rootFolders()):
			self.rootitem.appendChild(TreeItem(f, row, self.rootitem))

	def index(self, row, column, parent):
		if not self.hasIndex(row, column, parent):
			return QModelIndex()

		if not parent.isValid():
			parentItem = self.rootitem
		else:
			parentItem = parent.internalPointer()
		childItem = parentItem.child(row)
		if childItem is not None:
			return self.createIndex(row, column, childItem)
		else:
			return QModelIndex()

	def parent(self, index):
		if not index.isValid():
			return QModelIndex()
		item = index.internalPointer()
		if item.parent() is None:
			return QModelIndex()
		else:
			return self.createIndex(item.parent().row(), 0, item.parent())

	def rowCount(self, parent):
		if parent.column() > 0:
			return 0
		if not parent.isValid():
			parentItem = self.rootitem
		else:
			parentItem = parent.internalPointer()
		return parentItem.childCount()

	def columnCount(self, parent):
		if parent.isValid():
			return parent.internalPointer().columnCount()
		else:
#			return self.rootitem.columnCount()
			return 4

	def data(self, index, role):
		if not index.isValid():
			return None
		item = index.internalPointer()
		if role == Qt.DisplayRole:
			return item.data(index.column())
		return None

	def path(self, index):
		if index is None or not index.isValid():
			return None
		item = QModelIndex(index).internalPointer()
		return item.path()

	def uuid(self, index):
		if index is None or not index.isValid():
			return None
		item = QModelIndex(index).internalPointer()
		return item.uuid()

	def makeFootprint(self, index):
		if not index.isValid():
			return None
		item = QModelIndex(index).internalPointer()
		return item.makeFootprint()


	def flags(self, index):
		if not index.isValid():
			return 0
		folder = index.internalPointer().isFolder()
		if folder:
			return Qt.ItemIsEnabled
		return Qt.ItemIsEnabled | Qt.ItemIsSelectable

	def headerData(self, section, orientation, role):
		header = ("Name", "Author", "Source", "Description")
		if orientation == Qt.Horizontal and role == Qt.DisplayRole: 
			return header[section]
		return None

class Factory(AbstractSelFPDialogFactory):
	def __init__(self):
		AbstractSelFPDialogFactory.__init__(self)
		self.registerInstance(self)

	def makeDialog(self, parent):
		return SelectFPDialog(parent)

class PluginMain(xpcb.Plugin):
	def __init__(self):
		xpcb.Plugin.__init__(self)
	
	def installWidgets(self, win):
		pass
		
	def initPlugin(self, win):
		self.f = Factory()

