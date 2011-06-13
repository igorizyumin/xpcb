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

#include "Editor.h"
#include "Text.h"
#include "PartEditor.h"
#include "PinEditor.h"
#include "Controller.h"
#include "Area.h"
#include "Shape.h"

AbstractEditor::AbstractEditor(Controller *ctrl) :
	QObject(ctrl), mCtrl(ctrl)
{

}

bool AbstractEditor::eventFilter(QObject * /*watched*/, QEvent *event)
{
	event->accept();
	if (event->type() == QEvent::MouseMove)
	{
		mouseMoveEvent(static_cast<QMouseEvent*>(event));
	}
	else if (event->type() == QEvent::MouseButtonPress)
	{
		mousePressEvent(static_cast<QMouseEvent*>(event));
	}
	else if (event->type() == QEvent::MouseButtonRelease)
	{
		mouseReleaseEvent(static_cast<QMouseEvent*>(event));
	}
	else if (event->type() == QEvent::KeyPress)
	{
		keyPressEvent(static_cast<QKeyEvent*>(event));
	}
	else
	{
		event->ignore();
		return false;
	}
	return event->isAccepted();
}




EditorFactory* EditorFactory::mInst = NULL;

EditorFactory::EditorFactory()
	: mEditor(NULL)
{

}

EditorFactory& EditorFactory::instance()
{
	if (!EditorFactory::mInst)
	{
		EditorFactory::mInst = new EditorFactory();
	}
	return *EditorFactory::mInst;
}

AbstractEditor* EditorFactory::newEditor(PCBObject *obj, Controller *ctrl)
{
	mEditor = NULL;
	mObj = obj;
	mCtrl = ctrl;
	obj->accept(this);
	return mEditor;
}

AbstractEditor* EditorFactory::newTextEditor(Controller *ctrl)
{
	return new TextEditor(ctrl, NULL);
}

AbstractEditor* EditorFactory::newPinEditor(FPController *ctrl)
{
	return new PinEditor(ctrl, NULL);
}

void EditorFactory::visit(Area* a)
{
	if (mFactories.contains(ObjArea))
		mEditor = mFactories.value(ObjArea)->makeEditor(mCtrl, a);
}

void EditorFactory::visit(Line* l)
{
	if (mFactories.contains(ObjLine))
		mEditor = mFactories.value(ObjLine)->makeEditor(mCtrl, l);
}

void EditorFactory::visit(Net* n)
{
	if (mFactories.contains(ObjNet))
		mEditor = mFactories.value(ObjNet)->makeEditor(mCtrl, n);
}

void EditorFactory::visit(PartPin* p)
{
	if (mFactories.contains(ObjPartPin))
		mEditor = mFactories.value(ObjPartPin)->makeEditor(mCtrl, p);
}

void EditorFactory::visit(Pin* pin)
{
	mEditor = new PinEditor(dynamic_cast<FPController*>(mCtrl), pin);
}

void EditorFactory::visit(Part* p)
{
	mEditor = new PartEditor(mCtrl, p);
}

void EditorFactory::visit(Text *t)
{
	mEditor = new TextEditor(mCtrl, t);
}

void EditorFactory::visit(Vertex* v)
{
	if (mFactories.contains(ObjVertex))
		mEditor = mFactories.value(ObjVertex)->makeEditor(mCtrl, v);
}

void EditorFactory::visit(Segment* s)
{
	if (mFactories.contains(ObjSegment))
		mEditor = mFactories.value(ObjSegment)->makeEditor(mCtrl, s);
}

