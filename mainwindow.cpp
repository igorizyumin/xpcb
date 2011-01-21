#include "mainwindow.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include "GridToolbarWidget.h"
#include "ActionBar.h"
#include "AboutDialog.h"
#include "PCBView.h"
#include "PCBDoc.h"
#include "global.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setupUi(this);
	m_statusbar_xc = new QLabel("X: 0");
	m_statusbar_xc->setFixedWidth(60);
	m_statusbar_yc = new QLabel("Y: 0");
	m_statusbar_yc->setFixedWidth(60);
	this->statusbar->addPermanentWidget(m_statusbar_xc);
	this->statusbar->addPermanentWidget(m_statusbar_yc);

	m_gridwidget = new GridToolbarWidget();
	m_actionbar = new ActionBar();
	this->actionToolbar->addWidget(m_actionbar);
	this->gridToolbar->addWidget(m_gridwidget);
	this->m_view = new PCBView(NULL);
	this->m_doc = NULL;
	QObject::connect(this->m_view, SIGNAL(mouseMoved(QPoint)),
					 this, SLOT(onViewCoords(QPoint)));
	this->setCentralWidget(this->m_view);
	setCurrentFile("");
}

MainWindow::~MainWindow()
{

}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (maybeSave())
	{
		event->accept();
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
	this->m_view->setDoc(m_doc);
	setCurrentFile("");
}

void MainWindow::closeDoc()
{
	if (m_doc)
	{
		delete m_doc;
		m_doc = NULL;
	}
	this->m_view->setDoc(NULL);
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
	m_statusbar_xc->setText(QString("X: %1").arg(PCB2IN(pt.x()), -6, 'f', 3));
	m_statusbar_yc->setText(QString("Y: %1").arg(PCB2IN(pt.y()), -6, 'f', 3));
}

void MainWindow::documentWasModified()
{
	if (m_doc)
		setWindowModified(m_doc->isModified());
	else
		setWindowModified(false);
}
