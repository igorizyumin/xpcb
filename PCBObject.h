#ifndef PCBOBJECT_H
#define PCBOBJECT_H

#include <QPainter>
#include "global.h"

// Forward declarations for visitor abstract class
class Area;
class Arc;
class Line;
class Net;
class PartPin;
class Part;
class Footprint;
class Text;
class Vertex;
class Segment;
class Padstack;
class PCBObjectVisitor;
class PCBDoc;

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
	virtual void draw(QPainter *painter, XPcb::PCBLAYER layer) const = 0;

	/// Returns the object's bounding box
	virtual QRect bbox() const = 0;

	/// Returns the unique object identifier
	/// \return object ID
	int getid() const {return objID;}

	/// Returns true if the object was hit.
	virtual bool testHit(QPoint pt, XPcb::PCBLAYER l) const { return false; }

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

	static int getNextID();
private:
	int objID;
	static int nextObjID;
};

/// Abstract base class for PCB object visitor
class PCBObjectVisitor
{
public:
	virtual void visit(Area* a) = 0;
	virtual void visit(Arc* a) = 0;
	virtual void visit(Line* a) = 0;
	virtual void visit(Net* a) = 0;
	virtual void visit(PartPin* a) = 0;
	virtual void visit(Part* a) = 0;
	virtual void visit(Footprint* a) = 0;
	virtual void visit(Text* a) = 0;
	virtual void visit(Vertex* a) = 0;
	virtual void visit(Segment* a) = 0;
	virtual void visit(Padstack* a) = 0;
};

#endif // PCBOBJECT_H
