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

#ifndef EDITOR_H
#define EDITOR_H

#include <QObject>
#include <QAction>
#include <QMouseEvent>
#include "PCBObject.h"

class Controller;
class FPController;
class PCBController;
class CtrlAction;
class PCBDoc;

class AbstractEditor : public QObject
{
    Q_OBJECT
public:
	explicit AbstractEditor(Controller *ctrl);
	virtual void init() {}
	virtual void drawOverlay(QPainter* painter) = 0;
	virtual QList<const CtrlAction*> actions() const { return QList<const CtrlAction*>(); }

signals:
	void actionsChanged();
	void overlayChanged();
	void editorFinished();

public slots:

protected:
	virtual bool eventFilter(QObject *watched, QEvent *event);
	virtual void mouseMoveEvent(QMouseEvent* event) { event->ignore(); }
	virtual void mousePressEvent(QMouseEvent* event) { event->ignore(); }
	virtual void mouseReleaseEvent(QMouseEvent* event) { event->ignore(); }
	virtual void keyPressEvent(QKeyEvent *event) { event->ignore(); }

	Controller* ctrl() { return mCtrl; }
private:
	Controller *mCtrl;
};

class AbstractEditorFactory
{
public:
	virtual QSharedPointer<AbstractEditor> makeEditor(Controller* ctrl, QSharedPointer<PCBObject> obj) = 0;
};

class EditorFactory
{
public:
	enum ObjType {ObjArea, ObjLine, ObjNet, ObjPartPin, ObjPin, ObjPart, ObjFootprint, ObjText, ObjVertex, ObjSegment, ObjPadstack};
	static EditorFactory& instance();

	QSharedPointer<AbstractEditor> newEditor(QSharedPointer<PCBObject> obj, Controller *ctrl);
	QSharedPointer<AbstractEditor> newTextEditor(Controller *ctrl);
	QSharedPointer<AbstractEditor> newPinEditor(FPController* ctrl);
	QSharedPointer<AbstractEditor> newPartEditor(PCBController* ctrl);
	QSharedPointer<AbstractEditor> newLineEditor(FPController* ctrl);

	static void registerFactory(ObjType type, QSharedPointer<AbstractEditorFactory> factory) { instance().mFactories[type] = factory; }

private:
	EditorFactory();
	EditorFactory(EditorFactory &other);

	static EditorFactory* mInst;
	QSharedPointer<AbstractEditor> mEditor;
	// parameters for visit function
	Controller *mCtrl;
	QSharedPointer<PCBObject> mObj;

	QHash<ObjType, QSharedPointer<AbstractEditorFactory> > mFactories;
};

#endif // EDITOR_H
