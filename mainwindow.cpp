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
#include "PCBDoc.h"
#include "Controller.h"
#include "global.h"
#include "EditTextDialog.h"
#include "SelFilterWidget.h"
#include "LayerWidget.h"

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
	this->m_ctrl = new Controller(this);
	this->m_ctrl->registerView(m_view);
	this->m_ctrl->registerActionBar(m_actionbar);
	this->m_ctrl->registerLayerWidget(m_layers);
	this->m_doc = NULL;
	connect(this->m_view, SIGNAL(mouseMoved(QPoint)),
					 this, SLOT(onViewCoords(QPoint)));
	connect(this->m_gridwidget, SIGNAL(viewGridChanged(int)),
			m_view, SLOT(visGridChanged(int)));
	connect(this->m_gridwidget, SIGNAL(placeGridChanged(int)),
			m_ctrl, SLOT(onPlaceGridChanged(int)));
	connect(this->m_gridwidget, SIGNAL(routeGridChanged(int)),
			m_ctrl, SLOT(onRouteGridChanged(int)));
	this->setCentralWidget(this->m_view);
	setCurrentFile("");

	// restore geometry
	QSettings settings;
	restoreGeometry(settings.value("mainWindow/geometry").toByteArray());
	restoreState(settings.value("mainWindow/windowState").toByteArray());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (maybeSave())
	{
		event->accept();
		// save geometry
		QSettings settings;
		settings.setValue("mainWindow/geometry", saveGeometry());
		settings.setValue("mainWindow/windowState", saveState());
		QMainWindow::closeEvent(event);
	}
	else
	{
		event->ignore();
	}
}

void MainWindow::newDoc()
{
	m_doc = new PCBDoc();
	connect(this->m_doc, SIGNAL(changed()), this, SLOT(documentWasModified()));
	connect(this->m_doc, SIGNAL(canUndoChanged(bool)), this, SLOT(onUndoAvailableChanged(bool)));
	connect(this->m_doc, SIGNAL(canRedoChanged(bool)), this, SLOT(onRedoAvailableChanged(bool)));
	connect(this->action_Undo, SIGNAL(triggered()), m_doc, SLOT(undo()));
	connect(this->action_Redo, SIGNAL(triggered()), m_doc, SLOT(redo()));
	this->m_ctrl->registerDoc(m_doc);
	setCurrentFile("");
}

void MainWindow::closeDoc()
{
	if (m_doc)
	{
		m_ctrl->registerDoc(NULL);
		delete m_doc;
		m_doc = NULL;
	}
	setCurrentFile("");
}

void MainWindow::on_actionNew_triggered()
{
	if (maybeSave())
	{
		closeDoc();
		newDoc();
	}
}

void MainWindow::on_actionOpen_triggered()
{
	if (maybeSave())
	{
		QString fileName = QFileDialog::getOpenFileName(this);
		if (!fileName.isEmpty())
		{
			if (!m_doc)
			{
				newDoc();
				if (!loadFile(fileName))
					closeDoc();
			}
			else
				loadFile(fileName);
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
	if (this->m_doc && this->m_doc->isModified())
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
	if (m_doc && m_doc->loadFromFile(fileName))
	{
		setCurrentFile(fileName);
		statusBar()->showMessage(tr("File loaded"), 2000);
		return true;
	}
	return false;
}

bool MainWindow::saveFile(const QString &fileName)
{
	if (m_doc && m_doc->saveToFile(fileName))
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
	documentWasModified();


	QString shownName = m_curFile;
	if (m_doc && m_curFile.isEmpty())
		shownName = "untitled.xbrd";
	else if (!m_doc)
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
	m_statusbar_xc->setText(QString("X: %1").arg(XPcb::PCB2IN(pt.x())*1000.0, -6, 'f', 1));
	m_statusbar_yc->setText(QString("Y: %1").arg(XPcb::PCB2IN(pt.y())*1000.0, -6, 'f', 1));
}

void MainWindow::documentWasModified()
{
	if (m_doc)
		setWindowModified(m_doc->isModified());
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

