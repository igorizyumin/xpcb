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

#include "Editor.h"
#include "Text.h"
#include "PartEditor.h"
#include "PinEditor.h"
#include "LineEditor.h"
#include "Controller.h"
#include "Area.h"
#include "Footprint.h"
#include "Part.h"
#include "TraceEditor.h"
#include "AreaEditor.h"
#include "FPPropDialog.h"
#include "Document.h"

AbstractEditor::AbstractEditor(Controller *ctrl) :
	QObject(ctrl), mCtrl(ctrl)
{

}

QList<const CtrlAction*> AbstractEditor::actions() const
{
	return QList<const CtrlAction*>();
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

QSharedPointer<AbstractEditor> EditorFactory::newEditor(QSharedPointer<PCBObject> obj, Controller *ctrl)
{
	if (QSharedPointer<Area> a = obj.dynamicCast<Area>())
	{
		if (mFactories.contains(ObjArea))
			return mFactories.value(ObjArea)->makeEditor(ctrl, a);
	}
	else if (QSharedPointer<Line> l = obj.dynamicCast<Line>())
	{
		return QSharedPointer<AbstractEditor>(new LineEditor(ctrl, l));
	}
	else if (QSharedPointer<PartPin> pp = obj.dynamicCast<PartPin>())
	{
		if (mFactories.contains(ObjPartPin))
			return mFactories.value(ObjPartPin)->makeEditor(ctrl, pp);
	}
	else if (QSharedPointer<Pin> p = obj.dynamicCast<Pin>())
	{
		return QSharedPointer<AbstractEditor>(new PinEditor(ctrl, p));
	}
	else if (QSharedPointer<Part> p = obj.dynamicCast<Part>())
	{
		return QSharedPointer<AbstractEditor>(new PartEditor(ctrl, p));
	}
	else if (QSharedPointer<Text> t = obj.dynamicCast<Text>())
	{
		return QSharedPointer<AbstractEditor>(new TextEditor(ctrl, t));
	}
	else if (QSharedPointer<Vertex> v = obj.dynamicCast<Vertex>())
	{
		return QSharedPointer<AbstractEditor>(new VertexEditor(ctrl, v));
	}
	else if (QSharedPointer<Segment> s = obj.dynamicCast<Segment>())
	{
		return QSharedPointer<AbstractEditor>(new SegmentEditor(ctrl, s));
	}

	return QSharedPointer<AbstractEditor>();
}

QSharedPointer<AbstractEditor> EditorFactory::newEditor(Document* doc, Controller *ctrl)
{
	if (dynamic_cast<PCBDoc*>(doc))
	{
		return QSharedPointer<AbstractEditor>(new DefaultPCBEditor(ctrl));
	}
	else if (dynamic_cast<FPDoc*>(doc))
	{
		return QSharedPointer<AbstractEditor>(new DefaultFPEditor(ctrl));
	}
	return QSharedPointer<AbstractEditor>();
}

QSharedPointer<AbstractEditor> EditorFactory::newTextEditor(Controller *ctrl)
{
	return QSharedPointer<AbstractEditor>(new TextEditor(ctrl));
}

QSharedPointer<AbstractEditor> EditorFactory::newPinEditor(Controller *ctrl)
{
	return QSharedPointer<AbstractEditor>(new PinEditor(ctrl));
}

QSharedPointer<AbstractEditor> EditorFactory::newPartEditor(Controller *ctrl)
{
	return QSharedPointer<AbstractEditor>(new PartEditor(ctrl));
}

QSharedPointer<AbstractEditor> EditorFactory::newLineEditor(Controller *ctrl)
{
	return QSharedPointer<AbstractEditor>(new LineEditor(ctrl));
}

QSharedPointer<AbstractEditor> EditorFactory::newTraceEditor(Controller *ctrl)
{
	return QSharedPointer<AbstractEditor>(new NewTraceEditor(ctrl));
}

QSharedPointer<AbstractEditor> EditorFactory::newAreaEditor(Controller *ctrl)
{
	return QSharedPointer<AbstractEditor>(new AreaEditor(ctrl));
}

QSharedPointer<AbstractEditor> EditorFactory::placePartEditor(Controller *ctrl, QList<NLPart> parts)
{
	return QSharedPointer<AbstractEditor>(new PartEditor(ctrl, parts));
}

///////////////////////// DEFAULTPCBEDITOR ////////////////////

DefaultPCBEditor::DefaultPCBEditor(Controller *ctrl)
		: AbstractEditor(ctrl),
		  mAddTraceAction(1, "Add Trace"),
		  mAddTextAction(2, "Add Text"),
		  mAddPartAction(3, "Add Part"),
		  mAddAreaAction(4, "Add Area")
{
	connect(&mAddTraceAction, SIGNAL(execFired()), this, SLOT(actionAddTrace()));
	connect(&mAddTextAction, SIGNAL(execFired()), this, SLOT(actionAddText()));
	connect(&mAddPartAction, SIGNAL(execFired()), this, SLOT(actionAddPart()));
	connect(&mAddAreaAction, SIGNAL(execFired()), this, SLOT(actionAddArea()));
}

void DefaultPCBEditor::init()
{
	emit actionsChanged();
}

QList<const CtrlAction*> DefaultPCBEditor::actions() const
{
	QList<const CtrlAction*> out;
	out << &mAddTraceAction << &mAddTextAction << &mAddPartAction << &mAddAreaAction;
	return out;
}

void DefaultPCBEditor::actionAddText()
{
	ctrl()->installEditor(EditorFactory::instance().newTextEditor(ctrl()));
}

void DefaultPCBEditor::actionAddPart()
{
	ctrl()->installEditor(EditorFactory::instance().newPartEditor(ctrl()));
}

void DefaultPCBEditor::actionAddTrace()
{
	ctrl()->installEditor(EditorFactory::instance().newTraceEditor(ctrl()));
}

void DefaultPCBEditor::actionAddArea()
{
	ctrl()->installEditor(EditorFactory::instance().newAreaEditor(ctrl()));
}

///////////////////////// DEFAULTFPEDITOR ////////////////////

DefaultFPEditor::DefaultFPEditor(Controller *ctrl)
		: AbstractEditor(ctrl),
		  mAddLineAction(2, "Add Line"),
		  mAddPinAction(3, "Add Pin"),
		  mAddTextAction(1, "Add Text"),
		  mEditPropsAction(0, "Edit Properties")
{
	connect(&mAddLineAction, SIGNAL(execFired()), SLOT(actionAddLine()));
	connect(&mAddPinAction, SIGNAL(execFired()), SLOT(actionAddPin()));
	connect(&mAddTextAction, SIGNAL(execFired()), SLOT(actionAddText()));
	connect(&mEditPropsAction, SIGNAL(execFired()), this, SLOT(actionEditProps()));
}

void DefaultFPEditor::init()
{
	emit actionsChanged();
}

QList<const CtrlAction*> DefaultFPEditor::actions() const
{
	QList<const CtrlAction*> out;
	out << &mAddLineAction << &mAddTextAction << &mAddPinAction << &mEditPropsAction;
	return out;
}


void DefaultFPEditor::actionAddPin()
{
	ctrl()->installEditor(EditorFactory::instance().newPinEditor(ctrl()));
}

void DefaultFPEditor::actionAddText()
{
	ctrl()->installEditor(EditorFactory::instance().newTextEditor(ctrl()));
}

void DefaultFPEditor::actionAddLine()
{
	ctrl()->installEditor(EditorFactory::instance().newLineEditor(ctrl()));
}

void DefaultFPEditor::actionEditProps()
{
	FPPropDialog d(ctrl()->view());
	FPDoc* doc = dynamic_cast<FPDoc*>(ctrl()->doc());
	d.init(doc->footprint().data());
	int result = d.exec();
	if (result == QDialog::Accepted)
	{
		d.updateFp(doc->footprint().data());
	}
}
