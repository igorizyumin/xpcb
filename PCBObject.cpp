#include "PCBObject.h"

int PCBObject::objID = 0;

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
