#include "PCBObject.h"

PCBObject::PCBObject() : isSelected(false), isVisible(true), objID(nextObjID++)
{
}

virtual void PCBObject::setSelected(bool selected)
{
	isSelected  = selected;
}

virtual void  PCBObject::setVisible(bool visible)
{
	isVisible = visible;
}
