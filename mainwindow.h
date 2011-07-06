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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "ui_mainwindow.h"

class GridToolbarWidget;
class ActionBar;
class PCBView;
class Document;
class PCBDoc;
class FPDoc;
class Controller;
class PCBController;
class FPController;
class SelFilterWidget;
class LayerWidget;
class Plugin;

/// The MainWindow class represents a single open PCB editor window
class MainWindow : public QMainWindow, private Ui::MainWindowClass
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);

	/// Returns the current document, or NULL if no document is open.
	virtual Document* doc() = 0;
	virtual Controller* ctrl() = 0;

protected slots:
	void on_actionAbout_triggered();
	virtual void onViewCoords(QPoint pt);
	void documentWasModified();
	virtual void on_actionNew_triggered();
	virtual void on_actionOpen_triggered();
	virtual bool on_actionSave_triggered();
	virtual bool on_actionSave_as_triggered();
	virtual bool on_actionClose_triggered();
	void on_action_Undo_triggered();
	void on_action_Redo_triggered();
	void onUndoAvailableChanged(bool enabled);
	void onRedoAvailableChanged(bool enabled);

protected:
	virtual void closeEvent(QCloseEvent *event);
	virtual bool maybeSave();
	virtual void newDoc() = 0;
	virtual void closeDoc() = 0;
	virtual bool loadFile(const QString &fileName);
	virtual bool saveFile(const QString &fileName);
	virtual void setCurrentFile(const QString &fileName);
	QString strippedName(const QString &fullFileName);

	/// Restores the window geometry
	virtual void loadGeom() = 0;
	virtual void saveGeom() = 0;

	QLabel* m_statusbar_xc;
	QLabel* m_statusbar_yc;
	GridToolbarWidget* m_gridwidget;
	ActionBar* m_actionbar;
	PCBView *m_view;
	QString m_curFile;
	SelFilterWidget *m_selmask;
	LayerWidget *m_layers;
};

class PCBEditWindow : public MainWindow
{
	Q_OBJECT

public:
	PCBEditWindow(QWidget *parent = 0, QList<Plugin*> plugins = QList<Plugin*>());

protected:
	virtual void newDoc();
	virtual void closeDoc();
	virtual Document* doc();
	virtual Controller* ctrl();
	virtual void loadGeom();
	virtual void saveGeom();


private:
	PCBDoc* mDoc;
	PCBController* mCtrl;

};

class FPEditWindow : public MainWindow
{
	Q_OBJECT

public:
	FPEditWindow(QWidget *parent = 0, QList<Plugin*> plugins = QList<Plugin*>());

protected:
	virtual void newDoc();
	virtual void closeDoc();
	virtual Document* doc();
	virtual Controller* ctrl();
	virtual void loadGeom();
	virtual void saveGeom();

private:
	FPDoc* mDoc;
	FPController* mCtrl;
};

#endif // MAINWINDOW_H
