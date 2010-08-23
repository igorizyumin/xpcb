#include "PCBObject.h"

int PCBObject::nextObjID = 0;

PCBObject::PCBObject() : isSelected(false), isVisible(true), objID(nextObjID++)
{
}

void PCBObject::setSelected(bool selected)
{
	isSelected  = selected;
}

void  PCBObject::setVisible(bool visible)
{
	isVisible = visible;
}
