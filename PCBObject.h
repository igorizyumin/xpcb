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

#ifndef PCBOBJECT_H
#define PCBOBJECT_H

#include <QPainter>
#include <QUndoCommand>
#include "global.h"
#include "Log.h"

// Forward declarations for visitor abstract class
class Area;
class Line;
class Net;
class PartPin;
class Pin;
class Part;
class Text;
class Vertex;
class Segment;
class Padstack;
class PCBObjectVisitor;
class PCBDoc;
class Layer;
class PCBObjState;
class QPainter;

/// Abstract base class for PCB graphical objects

/// PCBObject is an abstract base class for all graphical PCB objects.  It provides a common
/// interface for working with PCB objects, such as selection, collision detection, visibility,
/// and drawing.  It also provides a facility for identifying particular objects
class PCBObject
{
public:
	PCBObject();

	/// Draws the object using the provided QPainter.  This function is
	/// called multiple times during a single redraw operation, once for each layer.
	/// \param painter the painter to use
	/// \param layer the PCB layer to draw
	virtual void draw(QPainter *painter, const Layer& layer) const = 0;

	/// Returns the object's bounding box
	virtual QRect bbox() const = 0;

	/// Returns the unique object identifier
	/// \return object ID
	int getid() const {return objID;}

	/// Returns true if the object was hit.
	virtual bool testHit(QPoint /* pt */, const Layer& /*l*/) const { return false; }

	/// Returns the object's transform (from the object's coordinate system to
	/// PCB coordinates).
	/// This interface is used to map child widgets into the parent's coordinate system.
	/// This is used by part and footprint description text, pins, padstacks,
	/// and other PCBObjects that are contained in another PCBObject.
	/// The default implementation returns a null transform.
	virtual QTransform transform() const { return QTransform(); }

	/// Notifies the object that its parent has changed.  The default implementation
	/// does nothing.
	virtual void parentChanged() {}

	/// Returns a snapshot of this object's state (the memento pattern).
	virtual PCBObjState getState() const = 0;
	/// Restores this object's state from the supplied object. Returns true
	/// if successful, false otherwise.
	virtual bool loadState(PCBObjState &state) = 0;

	static int getNextID();
private:
	int objID;
	static int nextObjID;
};

/// Abstract base class for PCBObject mementos
class PCBObjStateInternal
{
public:
	// needed to make object polymorphic for RTTI
	virtual ~PCBObjStateInternal() { }
};

/// Wrapper class that takes care of dealing with smartpointers.
class PCBObjState
{
public:
	PCBObjState() {}
	PCBObjState(PCBObjStateInternal* ptr) : mPtr(ptr) {  }
	const QSharedPointer<PCBObjStateInternal> ptr() const { return mPtr; }


protected:
	QSharedPointer<PCBObjStateInternal> mPtr;
};

/// Universal undo command
class PCBObjEditCmd : public QUndoCommand
{
public:
	PCBObjEditCmd(QUndoCommand* parent, QSharedPointer<PCBObject> obj,
				  PCBObjState prevState)
		: QUndoCommand(parent), mObj(obj), mPrevState(prevState),
		  mNewState(obj->getState()) {}

	virtual void undo() { mObj->loadState(mPrevState); }
	virtual void redo() { mObj->loadState(mNewState); }
private:
	QSharedPointer<PCBObject> mObj;
	PCBObjState mPrevState;
	PCBObjState mNewState;
};

#endif // PCBOBJECT_H
