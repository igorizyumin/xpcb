// pbunit.cpp : Defines the entry point for the console application.
//
#include <stdlib.h>
#include "polybool.h" 
using namespace POLYBOOLEAN; 
int main() 
{ 
	static const GRID2 a1[4] = 
	{ {-7, 8}, {-7, -3}, {2, -3}, {2, 8} }; 
	static const GRID2 a2[4] = 
	{ {-5, 6}, {0, 6}, {0, 0}, {-5, 0} }; 
	static const GRID2 b[11] = 
	{ {-5, -6}, {7,-6}, {7, 4}, {-5, 4}, {0, 0}, {0, 2}, 
	{5, 2}, {5, -4}, {0, -4}, {0, 0}, {-5, 0} }; 
	PAREA * A = NULL; 
	PAREA * B = NULL; 

	// construct 1st polygon
	PLINE2 * pline = new PLINE2(a1, 4);
	pline->Prepare(); 
	if (not pline->IsOuter()) // make sure the contour is outer 
		pline->Invert();

	PAREA::AddPlineToList(&A, pline);

	pline = new PLINE2(a2, 4);
	pline->Prepare(); 
	if (pline->IsOuter()) // make sure the contour is a hole 
		pline->Invert();

	PAREA::AddPlineToList(&A, pline);

	// construct 2nd polygon
	pline = new PLINE2(b, 11);
	pline->Prepare(); 
	if (not pline->IsOuter()) // make sure the contour is outer 
		pline->Invert();

	PAREA::AddPlineToList(&B, pline);

	// do Boolean operation XOR
	PAREA * R = NULL; 
	int err = PAREA::Boolean(A, B, &R, PAREA::XOR);

	// triangulate R 
	err = PAREA::Triangulate(R);

	// delete all polygons 
	PAREA::Del(&A); 
	PAREA::Del(&B); 
	PAREA::Del(&R);

	return err;
} 
