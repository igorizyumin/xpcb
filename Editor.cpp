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
#include "Controller.h"

AbstractEditor::AbstractEditor(Controller *ctrl) :
	QObject(ctrl), mCtrl(ctrl)
{

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

void EditorFactory::visit(Area* a)
{
}

void EditorFactory::visit(Arc* a)
{
}

void EditorFactory::visit(Line* a)
{
}

void EditorFactory::visit(Net* a)
{
}

void EditorFactory::visit(PartPin* a)
{
}

void EditorFactory::visit(Part* p)
{
	mEditor = new PartEditor(mCtrl, p);
}

void EditorFactory::visit(Footprint* a)
{
}

void EditorFactory::visit(Text *t)
{
	mEditor = new TextEditor(mCtrl, t);
}

void EditorFactory::visit(Vertex* a)
{
}

void EditorFactory::visit(Segment* a)
{
}

void EditorFactory::visit(Padstack* a)
{
}
