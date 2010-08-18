//	pbdefs.h - common definitions for PolyBoolean
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

#ifndef _PBDEFS_H_
#define _PBDEFS_H_

#include <qglobal.h>

namespace POLYBOOLEAN
{

////////////// Begin of the platform specific section //////////////////

// insert platform specific sized integer types here

typedef qint32				INT32;
typedef qint64				INT64;
typedef quint32				UINT32;

////////////// End of the platform specific section //////////////////

// if you would like to use your own VECT2, simply put it here
// instead of the default struct VECT2 definition
struct VECT2
{
		double  x, y;
};

} // namespace POLYBOOLEAN

// ranges for the integer coordinates
#define INT20_MAX			+524287
#define INT20_MIN			-524288

// error codes thrown by the library 
enum PBERRCODE {
	err_ok = 0,		// never thrown
	err_no_memory,	// not enough memory
	err_io,			// file I/O error
	err_bad_parm	// bad input data
};

#endif // _PBDEFS_H_

