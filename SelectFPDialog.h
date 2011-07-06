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

#ifndef SELECTFPDIALOG_H
#define SELECTFPDIALOG_H

#include <QUuid>
#include "Footprint.h"
#include "Document.h"
#include "ui_SelectFPDialog.h"

class FootprintPreview : public QWidget
{
public:
	explicit FootprintPreview(QWidget* parent = 0);

	void showFootprint(QString path);

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void resizeEvent(QResizeEvent *) { mIsDirty = true; }

private:
	void updateTransform();

	QSharedPointer<FPDoc> mDoc;
	QRect mBbox;
	QTransform mTransform;
	bool mIsDirty;

};

class TreeItem
{
public:
	static QSharedPointer<TreeItem> makeTree();

	bool isFolder() const { return !mFolder.isNull(); }
	int row() const { return mRow; }
	TreeItem* child(int row) const { return mChildren[row].data(); }
	TreeItem* parent() const { return mParent; }
	int childCount() const { return mChildren.count(); }
	QString data(int col) const;
	QString path() const;
	QUuid uuid() const;
private:
	TreeItem(QSharedPointer<FPDBFolder> folder, int row = 0,
			 TreeItem* parent = NULL);
	TreeItem(QSharedPointer<FPDBFile> file, int row = 0,
			 TreeItem* parent = NULL);

	QSharedPointer<FPDBFolder> mFolder;
	QSharedPointer<FPDBFile> mFile;
	int mRow;
	TreeItem* mParent;
	QList<QSharedPointer<TreeItem> > mChildren;
};

class FPTreeModel : public QAbstractItemModel
{
public:
	FPTreeModel(QObject *parent = NULL);

	// reimplemented virtual methods
	virtual QModelIndex index(int row, int column,
							  const QModelIndex &parent) const;
	virtual QModelIndex parent(const QModelIndex &child) const;
	virtual int rowCount(const QModelIndex &parent) const;
	virtual int columnCount(const QModelIndex &parent) const;
	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
	virtual QVariant headerData(int section,
								Qt::Orientation orientation, int role) const;

	/// Returns the footprint path
	QString path(const QModelIndex& index) const;
	/// Returns the footprint uuid
	QUuid uuid(const QModelIndex& index) const;
	/// Returns true if this is a folder
	bool isFolder(const QModelIndex& index) const;

private:
	QSharedPointer<TreeItem> mRootItem;

};

class SelectFPDialog : public QDialog, private Ui::SelectFPDialog
{
	Q_OBJECT

public:
	explicit SelectFPDialog(QWidget *parent = 0);

	bool isFpSelected() const { return mFpIndex.isValid(); }
	QUuid uuid() const { return mModel.uuid(mFpIndex); }
	QString path() const { return mModel.path(mFpIndex); }

private slots:
	void onSelChanged(const QModelIndex & current,
					  const QModelIndex & previous);

protected:
	virtual void accept();
	virtual void closeEvent(QCloseEvent *);
	virtual void hideEvent(QHideEvent *);

private:
	void saveGeom();
	void loadGeom();

	FPTreeModel mModel;
	FootprintPreview mPreview;
	QVBoxLayout* mPreviewLayout;
	QPersistentModelIndex mFpIndex;

};

#endif // SELECTFPDIALOG_H
