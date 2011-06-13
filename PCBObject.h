#ifndef PCBOBJECT_H
#define PCBOBJECT_H

#include <QPainter>
#include "global.h"

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

/// Abstract base class for PCB graphical objects

/// PCBObject is an abstract base class for all graphical PCB objects.  It provides a common
/// interface for working with PCB objects, such as selection, collision detection, visibility,
/// and drawing.  It also provides a facility for identifying particular objects
class PCBObject
{
public:
	PCBObject();

	virtual void accept(PCBObjectVisitor *v) = 0;

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

/// Abstract base class for PCB object visitor
class PCBObjectVisitor
{
public:
	virtual void visit(Area*) = 0;
	virtual void visit(Line*) = 0;
	virtual void visit(Net*) = 0;
	virtual void visit(PartPin*) = 0;
	virtual void visit(Pin*) = 0;
	virtual void visit(Part*) = 0;
	virtual void visit(Text*) = 0;
	virtual void visit(Vertex*) = 0;
	virtual void visit(Segment*) = 0;
};

/// Abstract base class for PCBObject mementos
class PCBObjStateInternal
{
public:
	// needed to make object polymorphic for RTTI
	virtual ~PCBObjStateInternal() {}
};

/// Wrapper class that takes care of dealing with smartpointers.
class PCBObjState
{
public:
	PCBObjState() {}
	PCBObjState(PCBObjStateInternal* ptr) : mPtr(ptr) {};
	const QSharedPointer<PCBObjStateInternal>& ptr() const { return mPtr; }

protected:
	QSharedPointer<PCBObjStateInternal> mPtr;
};

#endif // PCBOBJECT_H
