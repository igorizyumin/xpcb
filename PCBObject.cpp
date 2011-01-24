#include "PCBObject.h"
#include "PCBDoc.h"

int PCBObject::nextObjID = 1000;

PCBObject::PCBObject() : objID(nextObjID++)
{
}
