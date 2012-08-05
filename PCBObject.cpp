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

#include "PCBObject.h"

int PCBObject::nextObjID = 1000;

PCBObject::PCBObject(QObject *parent) : QObject(parent), objID(getNextID())
{
}

QTransform PCBObject::transform() const
{
	return QTransform();
}

int PCBObject::getNextID()
{
	return nextObjID++;
}

void PCBObject::childEvent(QChildEvent * e)
{
	if (e->type() == QChildEvent::ChildAdded)
	{
		if (!dynamic_cast<PCBObject*>(e->child())) return;
		connect(this, SIGNAL(transformChanged(QTransform)),
				dynamic_cast<PCBObject*>(e->child()), SLOT(onParentTransformChanged(QTransform)));
	}
	else if (e->type() == QChildEvent::ChildRemoved)
	{
		if (!dynamic_cast<PCBObject*>(e->child())) return;
		disconnect(this, SIGNAL(transformChanged(QTransform)),
				dynamic_cast<PCBObject*>(e->child()), SLOT(onParentTransformChanged(QTransform)));

	}
}
