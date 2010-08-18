//	PArea.cpp -- PAREA class implementation
//
//	This file is a part of PolyBoolean software library
//	(C) 1998-1999 Michael Leonov
//	Consult your license regarding permissions and restrictions
//
//	Modifications (C) 2010 Igor Izyumin
//
//	From readme.txt:
//	------
//	The library can be legally used by:
//	1) Open source software projects. This means that PolyBoolean source code should
//	be distributed along with your software and you give the users of your software
//	ability to modify PolyBoolean code and recompile your software using modified
//	PolyBoolean code. Also you should place the following notice in copyright and
//	readme sections of your software:
//	"This software uses the PolyBoolean library
//	(C) 1998-1999 Michael Leonov (mvl@rocketmail.com)"
//	------

#include "PArea.h"
#include "PLine.h"
#include "polybool.h"

using namespace POLYBOOLEAN;

PAREA::~PAREA()
{
	// remove from linked list
	Remove();

	// delete PLINE2s
	PLINE2 *p;
	while ((p = this->cntr) != NULL)
	{
		this->cntr = p->next;
		delete p;
	}
	delete tria;
}

void PAREA::Del(PAREA ** p)
{
	if (*p == NULL)
		return;
	PAREA *cur;
	while ((cur = (*p)->f) != *p)
	{
		delete cur;
	}
	delete cur;
	*p = NULL;
}

void PAREA::AddPline(PLINE2 * c)
{
	assert(c->next == NULL);

	if (c->IsOuter())
	{
		// outline
		assert(cntr == NULL);
		cntr = c;
	}
	else
	{
		// hole
		assert(cntr != NULL);
		// add after first element
		c->next = cntr->next;
		cntr->next = c;
	}
}

void PAREA::AddToList(PAREA **list)
{
	if (*list == NULL)
		*list = this;
	else
	{
		// insert at end
		this->b = (*list)->b;
		this->b->f = this;
		this->f = *list;
		this->f->b = this;
	}
}

void PAREA::AddPlineToList(PAREA ** list, PLINE2 * pline)
{
	assert(pline != NULL and pline->next == NULL);
	PAREA * t;
	if (pline->IsOuter())
	{
		t = new PAREA();
		t->AddToList(list);
	}
	else
	{
		assert(*list != NULL);
		// find the smallest container for the hole
		t = NULL;
		PAREA *	pa = *list;
		do {
			if (pa->cntr->PlineInside(*pline) and
			   (t == NULL or t->cntr->PlineInside(*(pa->cntr))))
				t = pa;
		} while ((pa = pa->f) != *list);

		if (t == NULL)	// couldn't find a container for the hole
			error(err_bad_parm);
	}
	t->AddPline(pline);
} // PAREA::InclPline


void PAREA::AddPlinesToList(PAREA ** area, PLINE2 ** holes)
{
	if (*holes == NULL)
		return;
	if (*area == NULL)
		error(err_bad_parm);

	while (*holes != NULL)
	{
		PLINE2 * next = (*holes)->next;
		(*holes)->next = NULL;
		AddPlineToList(area, *holes);
		*holes = next;
	}
} // PAREA::InsertHoles

local
PAREA * PareaCopy0(const PAREA * area)
{
	PAREA * dst = NULL;

	try
	{
		for (const PLINE2 * pline = area->cntr; pline != NULL; pline = pline->next)
			PAREA::AddPlineToList(&dst, pline->Copy(true));

		if (area->tria == NULL)
			return dst;

		dst->tria = new PTRIA2[area->tnum];
	}
	catch (...)
	{
		PAREA::Del(&dst);
		throw;
	}
	dst->tnum = area->tnum;
	for (UINT32 i = 0; i < area->tnum; i++)
	{
		PTRIA2 * td =  dst->tria + i;
		PTRIA2 * ts = area->tria + i;

		td->v0 = ts->v0->vn;
		td->v1 = ts->v1->vn;
		td->v2 = ts->v2->vn;
	}
	return dst;
} // PareaCopy0

PAREA * PAREA::Copy() const
{
	PAREA * dst = NULL;
	const PAREA *src = this;
	try
	{
		do {
			PAREA *di = PareaCopy0(src);
			di->AddToList(&dst);
		} while ((src = src->f) != this);
	}
	catch (...)
	{
		Del(&dst);
		throw;
	}
	return dst;
} // PAREA::Copy


bool PAREA::GridInside(const GRID2 & g) const
{
	const PAREA * pa = this;

	// check each area in linked list
	do {
		// check outer contour
		const PLINE2 *l = pa->cntr;
		if (l->GridInside(g))
		{
			// check if it is inside a hole
			while ((l = l->next) != NULL)
				if (l->GridInside(g))
					goto Proceed; // inside a hole, done with this poly
			// not inside a hole
			return true;
		}
Proceed: ;
	} while ((pa = pa->f) != this);
	return false;
} // GridInParea

bool PAREA::PlineInside(const PLINE2& p) const
{
	const PAREA * pa = this;

	// check each area in linked list
	do {
		// check outer contour
		const PLINE2 *l = pa->cntr;
		if (l->PlineInside(p))
		{
			// check if it is inside a hole
			while ((l = l->next) != NULL)
				if (l->PlineInside(p))
					goto Proceed; // inside a hole, done with this poly
			// not inside a hole
			return true;
		}
Proceed: ;
	} while ((pa = pa->f) != this);
	return false;
}

#ifndef NDEBUG

local
bool Chk(INT32 x)
{
	return INT20_MIN > x or x > INT20_MAX;
}

bool PAREA::CheckDomain()
{
	PAREA * pa = this;
	do {
		for (PLINE2 * pline = pa->cntr; pline != NULL; pline = pline->next)
		{
			if (Chk(pline->gMin.x) or
				Chk(pline->gMin.y) or
				Chk(pline->gMax.x) or
				Chk(pline->gMax.y))
				return false;
		}
	} while ((pa = pa->f) != this);
	return true;
} // PAREA::CheckDomain

#endif // NDEBUG

void PAREA::JoinLists(PAREA ** list1, PAREA ** list2)
{
	if (!list2)
		return;
	if (!list1)
	{
		*list1 = *list2;
		*list2 = NULL;
		return;
	}
	// connect list2 to the end of list1
	PAREA* last1 = (*list1)->b;
	last1->f = (*list2);
	(*list2)->b->f = (*list1);
	(*list1)->b = (*list2)->b;
	(*list2)->b = last1;

	*list2 = NULL;
}
