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
#include "PCBObject.h"
#include "Controller.h"

class PCBDoc;

class AbstractEditor : public QObject
{
    Q_OBJECT
public:
	explicit AbstractEditor(Controller *ctrl);
	virtual void init() {}
	virtual void drawOverlay(QPainter* painter) = 0;
	virtual QList<CtrlAction> actions() const { return QList<CtrlAction>(); }
	virtual void action(int key) {}

signals:
	void actionsChanged();
	void overlayChanged();
	void editorFinished();

public slots:

protected:
	Controller *mCtrl;
};

class EditorFactory : public PCBObjectVisitor
{
public:
	static EditorFactory& instance();

	AbstractEditor* newEditor(PCBObject* obj, Controller *ctrl);
	AbstractEditor* newTextEditor(Controller *ctrl);


	virtual void visit(Area* a);
	virtual void visit(Arc* a);
	virtual void visit(Line* a);
	virtual void visit(Net* a);
	virtual void visit(PartPin* a);
	virtual void visit(Pin* a);
	virtual void visit(Part* a);
	virtual void visit(Footprint* a);
	virtual void visit(Text* a);
	virtual void visit(Vertex* a);
	virtual void visit(Segment* a);
	virtual void visit(Padstack* a);

private:
	EditorFactory();
	EditorFactory(EditorFactory &other);

	static EditorFactory* mInst;
	AbstractEditor* mEditor;
	// parameters for visit function
	Controller *mCtrl;
	PCBObject *mObj;
};



#endif // EDITOR_H
