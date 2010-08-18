//	pbio.h - I/O routines
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

#ifndef _PBIO_H_
#define _PBIO_H_

#include "polybool.h"

namespace POLYBOOLEAN
{

void LoadPline(const char * szFname, PLINE2 ** pline);
void LoadParea(const char * szFname, PAREA ** area);
void SavePline(const char * szFname, const PLINE2 * pline);
void SaveParea(const char * szFname, const PAREA * area);

// calculates area bounding box
void CalcPareaBox(PAREA * area, VECT2 * vMin, VECT2 * vMax);

// scale area to grid, may delete contours with null area
void PareaToGrid(PAREA ** area, const VECT2 & vMin, const VECT2 & vMax);

// scale area from grid
void PareaFromGrid(PAREA * area, const VECT2 & vMin, const VECT2 & vMax);

} // namespace POLYBOOLEAN

#endif // _PBIO_H_

