//	pbimpl.h - common definitions for all PolyBoolean source files
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
//	------

#ifndef _PBIMPL_H_
#define _PBIMPL_H_

#include <cassert>
#include <cmath>

#include "pbdefs.h"

#define local		static inline

inline void error(PBERRCODE nCode) {
	throw nCode;
}

#endif // _PBIMPL_H_

