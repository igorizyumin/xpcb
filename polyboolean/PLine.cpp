#include "PLine.h"
#include "polybool.h"

using namespace POLYBOOLEAN;

#define SETBITS(n,mask,s) (((n)->Flags & ~(mask)) | ((s) & mask))

static inline
bool PointOnLine(const GRID2 & a, const GRID2 & b, const GRID2 & c);


PLINE2::PLINE2(const GRID2 &g) :
		next(NULL), head(new VNODE2(g)), Count(1), Flags(0), gMin(g), gMax(g)
{
}

PLINE2::PLINE2(const GRID2 *g, int n) :
		next(NULL), head(new VNODE2(*g)), Count(1), Flags(0), gMin(*g), gMax(*g)
{
	for(int i = 1; i < n; i++)
		AddVertex(g[i]);
}

PLINE2::~PLINE2()
{
	while (head->next != head)
		delete head->next;
	delete head;
}

void PLINE2::Del(PLINE2** list)
{
	if (*list == NULL)
		return;

	PLINE2 *cur;
	while ((cur = (*list)->next) != *list)
	{
		delete cur;
	}
	delete cur;
	*list = NULL;
}

void PLINE2::AddVertex(const GRID2 & g)
{
	VNODE2* vn = new VNODE2(g);
	InclVnode(*vn);
}

PLINE2 * PLINE2::Copy(bool bMakeLinks) const
{
	PLINE2 * dst = new PLINE2(head->g);
	if (bMakeLinks)
	{
		head->vn = dst->head;
		dst->head->vn = head;
	}
	try
	{
		VNODE2*	vn = head;
		while ((vn = vn->next) != head)
		{
			dst->AddVertex(vn->g);
			if (bMakeLinks)
			{
				vn->vn = dst->head->prev;
				dst->head->prev->vn = vn;
			}
		}
	}
	catch (...)
	{
		delete dst;
		throw;
	}
	dst->Flags = Flags;
	return dst;
}


bool PLINE2::Prepare()
{
	gMin.x = gMax.x = head->g.x;
	gMin.y = gMax.y = head->g.y;

	if (Count < 3)
		return false;

	// remove coincident vertices and those lying on the same line
	VNODE2 * p = head;
	VNODE2 * c = p->next;
	VNODE2 * n = c->next;

	do
	{
		// invariant: consecutive vertices from head to p are not extraneous
		if (PointOnLine(p->g, c->g, n->g))
		{
			// remove c
			delete c;
			Count--;
		}
		else
			p = c;

		c = n;
		n = n->next;
	} while (p != head->prev && Count >= 3);
	// postcondition: count == 2 OR p == head->prev, still need to check (head-1, head, head+1)
	if (PointOnLine(head->prev->g, head->g, head->next->g))
	{
		// head is extraneous
		p = head;
		head = head->next;
		delete p;
		Count--;
	}
	if (Count < 3)
		return false;

	c = head;
	p = c->prev;
	INT64  nArea = 0;

	do {
		nArea += (INT64)(p->g.x - c->g.x) * (p->g.y + c->g.y);
		AdjustBox(c->g);
	} while ((c = (p = c)->next) != head);

	if (nArea == 0)
		return false;

	Flags = SETBITS(this, ORIENT, (nArea < 0) ? INV : DIR);
	return true;
} // PLINE2::Prepare

void PLINE2::Invert()
{
	VNODE2 *vn = head;
	VNODE2 *n;
	// reverse linked list
	do {
		// swap vn->next and vn->prev
		n = vn->next;
		vn->next = vn->prev;
		vn->prev = n;
		// advance to next vnode
		vn = n;
	} while (vn != head);

	Flags ^= ORIENT;
} // PLINE2::Invert


void PLINE2::Put(PAREA ** area, PLINE2 ** holes)
{
	assert(this->next == NULL);
	if (this->IsOuter())
		PAREA::AddPlineToList(area, this);
	else
		this->next = *holes, *holes = this;
} // PLINE2::Put


void PLINE2::AdjustBox(const GRID2 & g)
{
	if (gMin.x > g.x)
		gMin.x = g.x;
	if (gMin.y > g.y)
		gMin.y = g.y;
	if (gMax.x < g.x)
		gMax.x = g.x;
	if (gMax.y < g.y)
		gMax.y = g.y;
}

bool PLINE2::GridInBox(const GRID2 & g) const
{
	return (g.x > this->gMin.x and g.y > this->gMin.y and
			g.x < this->gMax.x and g.y < this->gMax.y);
} // GridInBox

static inline
bool BoxInBox(const PLINE2 & c1, const PLINE2 & c2)
{
	return (c1.gMin.x >= c2.gMin.x and
			c1.gMin.y >= c2.gMin.y and
			c1.gMax.x <= c2.gMax.x and
			c1.gMax.y <= c2.gMax.y);
} // BoxInBox


void PLINE2::InclVnode(VNODE2 &vn)
{
	vn.Insert(*(this->head->prev));
	this->Count++;
	AdjustBox(vn.g);
}

// returns if b lies on (a,c)
static inline
bool PointOnLine(const GRID2 & a, const GRID2 & b, const GRID2 & c)
{
	return	(INT64)(b.x - a.x) * (c.y - b.y) ==
			(INT64)(b.y - a.y) * (c.x - b.x);
} // PointOnLine

bool PLINE2::PlineInside(const PLINE2 & p) const
{
	return BoxInBox(p, *this) and
		   GridInside(p.head->g);
}

bool PLINE2::GridInside(const GRID2 & g) const
{
	if (!GridInBox(g))
		return false;

	const VNODE2 *vn = this->head;
	bool bInside = false;
	do {
		const GRID2 & vc = vn->g;
		const GRID2 & vp = vn->prev->g;

		if (vc.y <= g.y and g.y < vp.y and
			(INT64)(vp.y - vc.y) * (g.x - vc.x) <
			(INT64)(g.y - vc.y) * (vp.x - vc.x))
		{
			bInside = not bInside;
		}
		else if (vp.y <= g.y and g.y < vc.y and
			(INT64)(vp.y - vc.y) * (g.x - vc.x) >
			(INT64)(g.y - vc.y) * (vp.x - vc.x))
		{
			bInside = not bInside;
		}
	} while ((vn = vn->next) != this->head);
	return bInside;
} // GridInside
