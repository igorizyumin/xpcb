//	polybool.h - main PolyBoolean header
//
//	This file is a part of PolyBoolean software library
//	(C) 1998-1999 Michael Leonov
//	Consult your license regarding permissions and restrictions
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

#ifndef _POLYBOOL_H_
#define _POLYBOOL_H_

#include "pbdefs.h"
#include "pbgeom.h"
#include "PLine.h"
#include "PArea.h"

namespace POLYBOOLEAN
{

struct VLINK;

struct VNODE2
{
    VNODE2 *next, *prev;
    UINT32	Flags;

// Flags reserved for internal use
	enum {
		RESERVED = 0x00FFu
	};


	GRID2	g;		// vertex coordinates
	VECT2	p;

	union	// temporary fields
	{
        struct
        {
            VLINK *i, *o;
        } lnk;

        int     i;
        void  * v;
        VNODE2* vn;
	};

	VNODE2(const GRID2 & g_) :
			next(this), prev(this),  Flags(0), g(g_) { lnk.i = NULL; lnk.o = NULL;}
	~VNODE2() { Remove(); }

	void Insert(VNODE2 & after) {
		next = after.next;
		next->prev = this;
		this->prev = &after;
		after.next = this;
	}

	void Remove() {
		prev->next = next;
		next->prev = prev;
		next = prev = this;
	}

}; // struct VNODE2

} // namespace POLYBOOLEAN

#endif // _POLYBOOL_H_

