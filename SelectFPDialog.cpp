/*
	This file is part of xpcb.

	xpcb is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	xpcb is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with xpcb.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QMessageBox>
#include <QSettings>
#include "SelectFPDialog.h"
#include "PCBDoc.h"

SelectFPDialog::SelectFPDialog(QWidget *parent) :
	QDialog(parent), mPreviewLayout(new QVBoxLayout())
{
    setupUi(this);
	this->fpTree->setModel(&mModel);
	connect(this->fpTree->selectionModel(),
			SIGNAL(currentChanged(QModelIndex,QModelIndex)),
			this, SLOT(onSelChanged(QModelIndex,QModelIndex)));
	mPreviewLayout->addWidget(&mPreview);
	this->PreviewWidget->setLayout(mPreviewLayout);
	loadGeom();
}

void SelectFPDialog::onSelChanged(const QModelIndex &current,
								  const QModelIndex & /*previous*/)
{
	mPreview.showFootprint(mModel.path(current));
}

void SelectFPDialog::accept()
{
	if (this->fpTree->selectionModel()->hasSelection()
			&& !mModel.isFolder(fpTree->selectionModel()->currentIndex()))
	{
		QDialog::accept();
		mFpIndex = QPersistentModelIndex(
					fpTree->selectionModel()->currentIndex());
	}
	else
	{
		QMessageBox mb(QMessageBox::Warning,
					   "No footprint selected",
					   "You have not selected a footprint.  Please make a selection and try again.",
					   QMessageBox::Ok, this);
		mb.exec();
	}
}

void SelectFPDialog::closeEvent(QCloseEvent *e)
{
	saveGeom();
	QDialog::closeEvent(e);
}

void SelectFPDialog::hideEvent(QHideEvent *e)
{
	saveGeom();
	QDialog::hideEvent(e);
}

void SelectFPDialog::saveGeom()
{
	QSettings s;
	s.setValue("fpselwindow/geometry", QVariant(saveGeometry()));
	s.setValue("fpselwindow/splitterState", QVariant(splitter->saveState()));
	s.setValue("fpselwindow/treeHeaderState", QVariant(fpTree->
													   header()->saveState()));
}

void SelectFPDialog::loadGeom()
{
	QSettings s;
	restoreGeometry(s.value("fpselwindow/geometry").toByteArray());
	splitter->restoreState(s.value("fpselwindow/splitterState").toByteArray());
	fpTree->header()->restoreState(
				s.value("fpselwindow/treeHeaderState").toByteArray());
}

/////////////////////////////////////////////////

FootprintPreview::FootprintPreview(QWidget *parent)
	: QWidget(parent)
{
	setSizePolicy(QSizePolicy::Expanding,
				  QSizePolicy::Expanding);
}

void FootprintPreview::showFootprint(QString path)
{
	mDoc = QSharedPointer<FPDoc>(new FPDoc());
	if (path.isEmpty() || !mDoc->loadFromFile(path))
	{
		mDoc.clear();
		update();
		return;
	}

	mIsDirty = true;
	update();
}

void FootprintPreview::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	// erase backgnd
	painter.setBackground(QBrush(Layer::color(Layer::LAY_BACKGND)));
	painter.setClipping(true);
	painter.eraseRect(rect());
	if (mDoc)
	{
		updateTransform();
		QList<Layer> layers = mDoc->layerList(Document::DrawPriorityOrder);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setTransform(mTransform);
		QList<QSharedPointer<PCBObject> > objs = mDoc->findObjs(mBbox);
		foreach(const Layer& l, layers)
		{
			if (l == Layer::LAY_SELECTION)
				continue;
			QPen p = painter.pen();
			p.setColor(l.color());
			p.setWidth(0);
			p.setCapStyle(Qt::RoundCap);
			p.setJoinStyle(Qt::RoundJoin);
			painter.setPen(p);
			QBrush b = painter.brush();
			b.setColor(l.color());
			b.setStyle(Qt::SolidPattern);
			painter.setBrush(b);
			foreach(QSharedPointer<PCBObject> o, objs)
			{
				o->draw(&painter, l);
			}
		}
	}
	painter.end();
}

void FootprintPreview::updateTransform()
{
	if (!mDoc || !mIsDirty)
		return;
	mBbox = (mDoc->footprint()->bbox()
			| mDoc->footprint()->refText()->bbox()
			| mDoc->footprint()->valueText()->bbox()).normalized();
	QRectF bb(mBbox);
	QRectF r(rect());
	double w, h, hoff, woff;
	QRectF mrect;
	double wscale = bb.width() / r.width();
	double hscale = bb.height() / r.height();
	if (wscale > hscale)
	{
		// width is the limiting factor
		w = r.width();
		h = bb.height() / wscale;
		hoff = (r.height() - h) / 2;
		mrect = QRectF(0, hoff, w, h).normalized();
	}
	else
	{
		// height is the limiting factor
		w = bb.width() / hscale;
		h = r.height();
		woff = (r.width() - w) / 2;
		mrect = QRectF(woff, 0, w, h).normalized();
	}
	mTransform.reset();
	QPolygonF poly(mrect);
	QPolygonF poly_rev;
	poly_rev << poly[3] << poly[2] << poly[1] << poly[0];
	poly = QPolygonF(bb);
	poly.remove(4);
	QTransform::quadToQuad(poly, poly_rev, mTransform);
}

//////////////////////////////////////////////////////

QSharedPointer<TreeItem> TreeItem::makeTree()
{
	QSharedPointer<TreeItem> root(new TreeItem(QSharedPointer<FPDBFolder>()));
	int i = 0;
	foreach(QSharedPointer<FPDBFolder> folder,
			FPDatabase::instance().rootFolders())
	{
		root->mChildren.append(
					QSharedPointer<TreeItem>(
						new TreeItem(folder, i++, root.data())));
	}
	return root;
}

TreeItem::TreeItem(QSharedPointer<FPDBFile> file, int row, TreeItem *parent)
	: mFile(file), mRow(row), mParent(parent)
{
}

TreeItem::TreeItem(QSharedPointer<FPDBFolder> folder, int row, TreeItem *parent)
	: mFolder(folder), mRow(row), mParent(parent)
{
	if (mFolder.isNull())
		return;
	// recursively add stuff to tree
	int i = 0;
	foreach(QSharedPointer<FPDBFolder> f, mFolder->folders())
	{
		mChildren.append(QSharedPointer<TreeItem>(new TreeItem(f, i++, this)));
	}
	foreach(QSharedPointer<FPDBFile> f, mFolder->items())
	{
		mChildren.append(QSharedPointer<TreeItem>(new TreeItem(f, i++, this)));
	}
}

QString TreeItem::data(int col) const
{
	if (!mFolder.isNull())
	{
		if (col == 0)
			return mFolder->name();
		else
			return QString();
	}
	else if (!mFile.isNull())
	{
		switch(col)
		{
		case 0:
			return mFile->name();
		case 1:
			return mFile->author();
		case 2:
			return mFile->source();
		case 3:
			return mFile->desc();
		default:
			return QString();
		}
	}
	else
		return QString();
}

QString TreeItem::path() const
{
	if (isFolder())
		return QString();
	return mFile->path();
}

QUuid TreeItem::uuid() const
{
	if (isFolder())
		return QUuid();
	return mFile->uuid();
}

/////////////////////////////////////////////////////

FPTreeModel::FPTreeModel(QObject *parent)
	: QAbstractItemModel(parent), mRootItem(TreeItem::makeTree())
{
}

QModelIndex FPTreeModel::index(int row, int column,
							   const QModelIndex &parent) const
{
	if(!hasIndex(row, column, parent))
		return QModelIndex();
	TreeItem* child;
	if(!parent.isValid())
		child = mRootItem->child(row);
	else
		child = reinterpret_cast<TreeItem*>(
					parent.internalPointer())->child(row);
	if (child)
		return createIndex(row, column, reinterpret_cast<void*>(child));
	else
		return QModelIndex();
}

QModelIndex FPTreeModel::parent(const QModelIndex &child) const
{
	if (!child.isValid())
		return QModelIndex();
	TreeItem* item = reinterpret_cast<TreeItem*>(
				child.internalPointer());
	if (!item->parent())
		return QModelIndex();
	else
		return createIndex(item->parent()->row(), 0,
						   reinterpret_cast<void*>(item->parent()));
}

int FPTreeModel::rowCount(const QModelIndex &parent) const
{
	if (parent.column() > 0)
		return 0;
	if (!parent.isValid())
		return mRootItem->childCount();
	else
		return reinterpret_cast<TreeItem*>(
					parent.internalPointer())->childCount();
}

int FPTreeModel::columnCount(const QModelIndex & /*parent*/) const
{
	return 4;
}

QVariant FPTreeModel::data(const QModelIndex &index, int role) const
{
	if (index.isValid() && role == Qt::DisplayRole)
		return reinterpret_cast<TreeItem*>(
					index.internalPointer())->data(index.column());
	else
		return QVariant();
}

Qt::ItemFlags FPTreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;
	if (reinterpret_cast<TreeItem*>(
				index.internalPointer())->isFolder())
		return Qt::ItemIsEnabled;
	else
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant FPTreeModel::headerData(int section,
								 Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole
			&& section < 4)
	{
		QList<QString> sections;
		sections << QString("Name") << QString("Author") << QString("Source")
					<< QString("Description");
		return QVariant(sections[section]);
	}
	return QVariant();
}

bool FPTreeModel::isFolder(const QModelIndex& index) const
{
	if (index.isValid())
		return reinterpret_cast<TreeItem*>(
					index.internalPointer())->isFolder();
	return false;
}

QString FPTreeModel::path(const QModelIndex& index) const
{
	if (index.isValid())
		return reinterpret_cast<TreeItem*>(
					index.internalPointer())->path();
	return QString();
}

QUuid FPTreeModel::uuid(const QModelIndex& index) const
{
	if (index.isValid())
		return reinterpret_cast<TreeItem*>(
					index.internalPointer())->uuid();
	return QUuid();
}
