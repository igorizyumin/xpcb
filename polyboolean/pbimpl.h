//	pbimpl.h - common definitions for all PolyBoolean source files
//
//	This file is a part of PolyBoolean software library
//	(C) 1998-1999 Michael Leonov
//	Consult your license regarding permissions and restrictions
//

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

