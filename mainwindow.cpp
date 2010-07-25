#include "MainWindow.h"
#include <QLabel>
#include <QHBoxLayout>
#include "GridToolbarWidget.h"
#include "AboutDialog.h"

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

	m_gridwidget = new GridToolbarWidget(this->gridToolbar);
	this->gridToolbar->addWidget(m_gridwidget);
}

MainWindow::~MainWindow()
{

}

void MainWindow::on_actionAbout_triggered()
{
	AboutDialog about(this);
	about.exec();
}
