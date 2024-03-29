/*
	Copyright (C) 2010-2011 Igor Izyumin

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

#include "mainwindow.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QSettings>
#include "GridToolbarWidget.h"
#include "ActionBar.h"
#include "AboutDialog.h"
#include "PCBView.h"
#include "Document.h"
#include "Controller.h"
#include "global.h"
#include "EditTextDialog.h"
#include "SelFilterWidget.h"
#include "LayerWidget.h"
#include "Plugin.h"
#include "NetlistDialog.h"
#include "PartPlacer.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setupUi(this);
	m_statusbar_xc = new QLabel("X: 0");
	m_statusbar_xc->setFixedWidth(80);
	m_statusbar_yc = new QLabel("Y: 0");
	m_statusbar_yc->setFixedWidth(80);
	this->statusbar->addPermanentWidget(m_statusbar_xc);
	this->statusbar->addPermanentWidget(m_statusbar_yc);

	m_gridwidget = new GridToolbarWidget();
	m_selmask = new SelFilterWidget();
	m_layers = new LayerWidget();
	this->selMaskScrollArea->setWidget(m_selmask);
	this->layerScrollArea->setWidget(m_layers);
	m_actionbar = new ActionBar();
	this->actionToolbar->addWidget(m_actionbar);
	this->gridToolbar->addWidget(m_gridwidget);
	this->m_view = new PCBView(this);

	this->mCtrl = new Controller(this);
	this->mCtrl->registerView(m_view);
	this->mCtrl->registerActionBar(m_actionbar);
	this->mCtrl->registerLayerWidget(m_layers);
	connect(this->m_gridwidget, SIGNAL(placeGridChanged(int)),
			mCtrl, SLOT(onPlaceGridChanged(int)));
	connect(this->m_gridwidget, SIGNAL(routeGridChanged(int)),
			mCtrl, SLOT(onRouteGridChanged(int)));

	connect(this->m_view, SIGNAL(mouseMoved(QPoint)),
					 this, SLOT(onViewCoords(QPoint)));
	connect(this->m_gridwidget, SIGNAL(viewGridChanged(int)),
			m_view, SLOT(visGridChanged(int)));


	this->setCentralWidget(this->m_view);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (maybeSave())
	{
		event->accept();
		saveGeom();
		QMainWindow::closeEvent(event);
	}
	else
	{
		event->ignore();
	}
}

void MainWindow::on_actionNew_triggered()
{
	if (maybeSave())
	{
		closeDoc();
		newDoc();
		emit documentHasChanged();
	}
}

void MainWindow::on_actionOpen_triggered()
{
	if (maybeSave())
	{
		QString fileName = QFileDialog::getOpenFileName(this);
		if (!fileName.isEmpty())
		{
			closeDoc();
			newDoc();
			if (!loadFile(fileName))
				closeDoc();
			emit documentHasChanged();
		}
	}
}

bool MainWindow::on_actionSave_triggered()
{
	if (m_curFile.isEmpty()) {
		return on_actionSave_as_triggered();
	} else {
		return saveFile(m_curFile);
	}
}

bool MainWindow::on_actionClose_triggered()
{
	if (maybeSave())
	{
		closeDoc();
		emit documentHasChanged();
		return true;
	}
	return false;
}

bool MainWindow::on_actionSave_as_triggered()
{
	QString fileName = QFileDialog::getSaveFileName(this);
	if (fileName.isEmpty())
		return false;

	return saveFile(fileName);
}

bool MainWindow::maybeSave()
{
	if (doc() && doc()->isModified())
	{
		QMessageBox::StandardButton ret;
		ret = QMessageBox::warning(this, tr("xpcb"),
					 tr("The document has been modified.\n"
						"Do you want to save your changes?"),
					 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		if (ret == QMessageBox::Save)
			return on_actionSave_triggered();
		else if (ret == QMessageBox::Cancel)
			return false;
	}
	return true;
}

bool MainWindow::loadFile(const QString &fileName)
{
	if (doc() && doc()->loadFromFile(fileName))
	{
		setCurrentFile(fileName);
		statusBar()->showMessage(tr("File loaded"), 2000);
		emit documentHasChanged();
		return true;
	}
	return false;
}

bool MainWindow::saveFile(const QString &fileName)
{
	if (doc() && doc()->saveToFile(fileName))
	{
		setCurrentFile(fileName);
		statusBar()->showMessage(tr("File saved"), 2000);
		return true;
	}
	return false;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
	m_curFile = fileName;
	updateModifiedFlag();

	QString shownName = m_curFile;
	if (doc() && m_curFile.isEmpty())
		shownName = "untitled.xbrd";
	else if (!doc())
		shownName = "No document open";
	setWindowFilePath(shownName);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}

void MainWindow::on_actionAbout_triggered()
{
	AboutDialog about(this);
	about.exec();
}

void MainWindow::onViewCoords(QPoint pt)
{
	m_statusbar_xc->setText(QString("X: %1").arg(XPcb::pcbToInch(pt.x())*1000.0, -6, 'f', 1));
	m_statusbar_yc->setText(QString("Y: %1").arg(XPcb::pcbToInch(pt.y())*1000.0, -6, 'f', 1));
}

void MainWindow::updateModifiedFlag()
{
	if (doc())
		setWindowModified(doc()->isModified());
	else
		setWindowModified(false);
}

void MainWindow::onUndoAvailableChanged(bool enabled)
{
	this->action_Undo->setEnabled(enabled);
}

void MainWindow::onRedoAvailableChanged(bool enabled)
{
	this->action_Redo->setEnabled(enabled);
}

void MainWindow::on_action_Undo_triggered()
{
	if (doc()) doc()->undo();
}

void MainWindow::on_action_Redo_triggered()
{
	if (doc()) doc()->redo();
}

///////////////////////////// PCBEDITWINDOW ///////////////////////////////////

PCBEditWindow::PCBEditWindow(QWidget *parent, QList<Plugin*> plugins)
	: MainWindow(parent), mDoc(NULL)
{
	setCurrentFile("");

	mPartPlacer = new PartPlacer(0, ctrl());
	this->addDockWidget(Qt::RightDockWidgetArea, mPartPlacer);
	connect(this, SIGNAL(documentHasChanged()),
			mPartPlacer, SLOT(updateList()));

	foreach(Plugin* p, plugins)
	{
		p->installWidgets(*this);
	}

	loadGeom();
}

void PCBEditWindow::newDoc()
{
	mDoc = new PCBDoc();
	connect(this->mDoc, SIGNAL(changed()),
			this, SLOT(updateModifiedFlag()));
	connect(this->mDoc, SIGNAL(canUndoChanged(bool)),
			this, SLOT(onUndoAvailableChanged(bool)));
	connect(this->mDoc, SIGNAL(canRedoChanged(bool)),
			this, SLOT(onRedoAvailableChanged(bool)));
	connect(this->mDoc, SIGNAL(partsChanged()),
			this->mPartPlacer, SLOT(updateList()));
	ctrl()->registerDoc(mDoc);
	setCurrentFile("");
}

void PCBEditWindow::closeDoc()
{
	if (mDoc)
	{
		ctrl()->registerDoc(NULL);
		delete mDoc;
		mDoc = NULL;
	}
	setCurrentFile("");
}

void PCBEditWindow::on_actionImport_Netlist_triggered()
{
	if (!mDoc) return;
	QString fileName = QFileDialog::getOpenFileName(this);
	if (!fileName.isEmpty())
	{
		mDoc->netlist()->loadFromFile(fileName);
	}
}

void PCBEditWindow::on_actionNets_triggered()
{
	if (!mDoc) return;
	NetlistDialog d(this, mDoc->netlist(), mDoc);
	d.exec();
}

Document* PCBEditWindow::doc()
{
	return mDoc;
}

void PCBEditWindow::loadGeom()
{
	// restore geometry
	QSettings settings;
	restoreGeometry(settings.value("mainWindow/geometry").toByteArray());
	restoreState(settings.value("mainWindow/windowState").toByteArray());
}

void PCBEditWindow::saveGeom()
{
	// save geometry
	QSettings settings;
	settings.setValue("mainWindow/geometry", saveGeometry());
	settings.setValue("mainWindow/windowState", saveState());
}

///////////////////////////// FPEDITWINDOW ///////////////////////////////////

FPEditWindow::FPEditWindow(QWidget *parent, QList<Plugin*> plugins)
	: MainWindow(parent), mDoc(NULL)
{
	setCurrentFile("");

	foreach(Plugin* p, plugins)
	{
		p->installWidgets(*this);
	}

	loadGeom();
}

void FPEditWindow::newDoc()
{
	mDoc = new FPDoc();
	connect(this->mDoc, SIGNAL(changed()), this, SLOT(updateModifiedFlag()));
	connect(this->mDoc, SIGNAL(canUndoChanged(bool)), this, SLOT(onUndoAvailableChanged(bool)));
	connect(this->mDoc, SIGNAL(canRedoChanged(bool)), this, SLOT(onRedoAvailableChanged(bool)));
	this->ctrl()->registerDoc(mDoc);
	setCurrentFile("");
}

void FPEditWindow::closeDoc()
{
	if (mDoc)
	{
		ctrl()->registerDoc(NULL);
		delete mDoc;
		mDoc = NULL;
	}
	setCurrentFile("");
}

Document* FPEditWindow::doc()
{
	return mDoc;
}

void FPEditWindow::loadGeom()
{
	// restore geometry
	QSettings settings;
	restoreGeometry(settings.value("fpWindow/geometry").toByteArray());
	restoreState(settings.value("fpWindow/windowState").toByteArray());
}

void FPEditWindow::saveGeom()
{
	// save geometry
	QSettings settings;
	settings.setValue("fpWindow/geometry", saveGeometry());
	settings.setValue("fpWindow/windowState", saveState());
}


