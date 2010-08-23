#include "PolygonList.h"
#include "Polygon.h"
#include "polybool.h"

using namespace POLYBOOLEAN;

PolygonList::PolygonList()
{
}

PolygonList::PolygonList(const PAREA *from)
{
	rebuildFromParea(from);
}

PolygonList::PolygonList(const Polygon &from)
{
	this->insert(new Polygon(from));
}

PolygonList::PolygonList(const PolygonList &from)
	: QSet<Polygon*>::QSet()
{
	foreach(const Polygon* p, from)
	{
		this->insert(new Polygon(*p));
	}
}

PolygonList::~PolygonList()
{
	removeAll();
}

const PolygonList& PolygonList::operator=(const PolygonList& rhs)
{
	// check for self-assignment
	if (&rhs == this)
		return *this;
	removeAll();
	foreach(const Polygon* p, rhs)
	{
		this->insert(new Polygon(*p));
	}
	return *this;
}

void PolygonList::removeAll()
{
	foreach(Polygon* p, *this)
	{
		delete p;
	}
	this->clear();
}

void PolygonList::removeElement(Polygon *p)
{
	delete p;
	this->remove(p);
}

PAREA* PolygonList::toPareaList() const
{
	PAREA* ret = NULL;
	foreach(Polygon* p, *this)
	{
		PAREA* pa = p->getParea();
		PAREA::JoinLists(&ret, &pa);
	}
	return ret;
}


void PolygonList::rebuildFromParea(const PAREA* a)
{
	removeAll();
	// iterate over result polygons
	if (a == NULL)
		return;
	const PAREA *curr = a;
	do
	{
		if (curr->cntr)
		{
			// add to polylist
			this->insert(new Polygon(curr));
		}
		else
			Q_ASSERT(false);

		curr = curr->f;
	} while(curr != a);
}

PolygonList& PolygonList::operator|=(const Polygon& rhs)
{
	// check if rhs intersects anything in the list
	foreach(Polygon* p, *this)
	{
		if (p->intersects(rhs))
		{
			// found intersection, combine with another poly
			this->unite(p->united(rhs));
			this->removeElement(p);
			return *this;
		}
	}

	// does not intersect any of the polygons in list, just add it
	this->insert(new Polygon(rhs));
	return *this;
}

PolygonList& PolygonList::operator&=(const Polygon& rhs)
{
	return (*this &= PolygonList(rhs));
}

PolygonList& PolygonList::operator-=(const Polygon& rhs)
{
	return (*this -= PolygonList(rhs));
}

void PolygonList::doBoolean(const PolygonList &rhs, PAREA::PBOPCODE op)
{
	PAREA *pathis = toPareaList();
	PAREA *parhs = rhs.toPareaList();
	PAREA *result;
	PBERRCODE ret = PAREA::Boolean0(pathis, parhs, &result, op);
	if (ret != err_ok)
		Q_ASSERT(false);
	else
		rebuildFromParea(result);
	PAREA::Del(&pathis);
	PAREA::Del(&parhs);
	PAREA::Del(&result);
}

PolygonList& PolygonList::operator|=(const PolygonList& rhs)
{
	doBoolean(rhs, PAREA::OR);
	return *this;
}

PolygonList& PolygonList::operator&=(const PolygonList& rhs)
{
	doBoolean(rhs, PAREA::AND);
	return *this;
}

PolygonList& PolygonList::operator-=(const PolygonList& rhs)
{
	doBoolean(rhs, PAREA::SUB);
	return *this;
}

PolygonList& PolygonList::operator|(const PolygonList& rhs) const
{
	PolygonList p(*this);
	return p |= rhs;
}

PolygonList& PolygonList::operator&(const PolygonList& rhs) const
{
	PolygonList p(*this);
	return p &= rhs;
}

PolygonList& PolygonList::operator-(const PolygonList& rhs) const
{
	PolygonList p(*this);
	return p -= rhs;
}

PolygonList& PolygonList::operator|(const Polygon& rhs) const
{
	PolygonList p(*this);
	return p |= rhs;
}

PolygonList& PolygonList::operator&(const Polygon& rhs) const
{
	PolygonList p(*this);
	return p &= rhs;
}

PolygonList& PolygonList::operator-(const Polygon& rhs) const
{
	PolygonList p(*this);
	return p -= rhs;
}
