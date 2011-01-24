#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "ui_mainwindow.h"

class GridToolbarWidget;
class ActionBar;
class PCBView;
class PCBDoc;
class Controller;

/// The MainWindow class represents a single open PCB editor window
class MainWindow : public QMainWindow, private Ui::MainWindowClass
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_actionAbout_triggered();
	void onViewCoords(QPoint pt);
	void documentWasModified();
	void on_actionNew_triggered();
	void on_actionOpen_triggered();
	bool on_actionSave_triggered();
	bool on_actionSave_as_triggered();
	bool on_actionClose_triggered();
	void onUndoAvailableChanged(bool enabled);
	void onRedoAvailableChanged(bool enabled);

protected:
	void closeEvent(QCloseEvent *event);

private:
	bool maybeSave();
	void newDoc();
	void closeDoc();
	bool loadFile(const QString &fileName);
	bool saveFile(const QString &fileName);
	void setCurrentFile(const QString &fileName);
	QString strippedName(const QString &fullFileName);

	QLabel* m_statusbar_xc;
	QLabel* m_statusbar_yc;
	GridToolbarWidget* m_gridwidget;
	ActionBar* m_actionbar;
	PCBView *m_view;
	PCBDoc* m_doc;
	Controller *m_ctrl;
	QString m_curFile;
};

#endif // MAINWINDOW_H
