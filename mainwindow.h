#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "ui_mainwindow.h"
#include "GridToolbarWidget.h"

class MainWindow : public QMainWindow, private Ui::MainWindowClass
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void on_actionAbout_triggered();

protected:
	QLabel* m_statusbar_xc;
	QLabel* m_statusbar_yc;
	GridToolbarWidget* m_gridwidget;
private:
};

#endif // MAINWINDOW_H
