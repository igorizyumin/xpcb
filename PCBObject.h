#ifndef PCBOBJECT_H
#define PCBOBJECT_H

#include <QPainter>
#include "global.h"

/// Abstract base class for PCB graphical objects

/// PCBObject is an abstract base class for all graphical PCB objects.  It provides a common
/// interface for working with PCB objects, such as selection, collision detection, visibility,
/// and drawing.  It also provides a facility for identifying particular objects
class PCBObject
{
public:
	PCBObject();

	virtual bool selected() { return isSelected; }
	virtual void setSelected(bool selected);

	virtual bool visible() { return isVisible; }
	virtual void setVisible(bool visible);

	/// Draws the object using the provided QPainter.  This function is
	/// called multiple times during a single redraw operation, once for each layer.
	/// \param painter the painter to use
	/// \param layer the PCB layer to draw
	virtual void draw(QPainter *painter, PCBLAYER layer) = 0;

	/// Returns the object's bounding box
	virtual const QRect& bbox() = 0;

	/// Returns the unique object identifier
	/// \return object ID
	int getid() {return objID;}

private:
	bool isSelected;
	bool isVisible;
	int objID;
	static int nextObjID;
};

#endif // PCBOBJECT_H
