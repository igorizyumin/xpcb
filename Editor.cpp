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

QSharedPointer<AbstractEditor> EditorFactory::newEditor(QSharedPointer<PCBObject> obj, Controller *ctrl)
{
	if (QSharedPointer<Area> a = obj.dynamicCast<Area>())
	{
		if (mFactories.contains(ObjArea))
			return mFactories.value(ObjArea)->makeEditor(ctrl, a);
	}
	else if (QSharedPointer<Line> l = obj.dynamicCast<Line>())
	{
		return QSharedPointer<AbstractEditor>(new LineEditor(dynamic_cast<FPController*>(ctrl), l));
	}
	else if (QSharedPointer<Net> n = obj.dynamicCast<Net>())
	{
		if (mFactories.contains(ObjNet))
			return mFactories.value(ObjNet)->makeEditor(ctrl, n);
	}
	else if (QSharedPointer<PartPin> pp = obj.dynamicCast<PartPin>())
	{
		if (mFactories.contains(ObjPartPin))
			return mFactories.value(ObjPartPin)->makeEditor(ctrl, pp);
	}
	else if (QSharedPointer<Pin> p = obj.dynamicCast<Pin>())
	{
		return QSharedPointer<AbstractEditor>(new PinEditor(dynamic_cast<FPController*>(ctrl), p));
	}
	else if (QSharedPointer<Part> p = obj.dynamicCast<Part>())
	{
		return QSharedPointer<AbstractEditor>(new PartEditor(dynamic_cast<PCBController*>(ctrl), p));
	}
	else if (QSharedPointer<Text> t = obj.dynamicCast<Text>())
	{
		return QSharedPointer<AbstractEditor>(new TextEditor(ctrl, t));
	}
	else if (QSharedPointer<Vertex> v = obj.dynamicCast<Vertex>())
	{
		if (mFactories.contains(ObjVertex))
			return mFactories.value(ObjVertex)->makeEditor(ctrl, v);
	}
	else if (QSharedPointer<Segment> s = obj.dynamicCast<Segment>())
	{
		return QSharedPointer<AbstractEditor>(new SegmentEditor(dynamic_cast<PCBController*>(ctrl), s));
	}

	return QSharedPointer<AbstractEditor>();
}

QSharedPointer<AbstractEditor> EditorFactory::newTextEditor(Controller *ctrl)
{
	return QSharedPointer<AbstractEditor>(new TextEditor(ctrl));
}

QSharedPointer<AbstractEditor> EditorFactory::newPinEditor(FPController *ctrl)
{
	return QSharedPointer<AbstractEditor>(new PinEditor(ctrl));
}

QSharedPointer<AbstractEditor> EditorFactory::newPartEditor(PCBController *ctrl)
{
	return QSharedPointer<AbstractEditor>(new PartEditor(ctrl));
}

QSharedPointer<AbstractEditor> EditorFactory::newLineEditor(FPController *ctrl)
{
	return QSharedPointer<AbstractEditor>(new LineEditor(ctrl));
}

QSharedPointer<AbstractEditor> EditorFactory::newTraceEditor(PCBController *ctrl)
{
	return QSharedPointer<AbstractEditor>(new NewTraceEditor(ctrl));
}
