#ifndef PLINE_H
#define PLINE_H

#include "pbimpl.h"
#include "pbgeom.h"

namespace POLYBOOLEAN
{

struct VNODE2;
class PAREA;

/// The PLINE2 class represents a closed contour.  To represent a set of contours,
/// multiple objects are stored in a doubly linked list.
class PLINE2
{
public:
	/// Flags
	enum {
		RESERVED = 0x00FF,
		ORIENT   = 0x0100,
		DIR      = 0x0100,
		INV      = 0x0000
	};


	PLINE2*	next;	///< next contour
	VNODE2*	head;	///< first vertex
	UINT32  Count;	///< number of vertices
	UINT32  Flags;  ///< object flags

	/// lower left corner of bounding rectangle
	GRID2	gMin;
	VECT2	vMin;
	/// upper right corner of bounding rectangle
	GRID2	gMax;
	VECT2	vMax;

	/// Constructs a PLINE2 with the provided starting point.
	/// \param g the first point of the PLINE
	PLINE2(const GRID2 &g);
	/// Constructs a PLINE2 from a list of points
	/// \param g list of points
	/// \param n number of points in list
	PLINE2(const GRID2 *g, int n);
	~PLINE2();

	/// Deallocates a linked list of PLINE2s.
	static void Del(PLINE2** list);

	/// \returns true if pline is an outer contour (counterclockwise),
	/// false if pline is an inner contour (clockwise).
	bool IsOuter() const { return (Flags & ORIENT) == DIR; }

	/// Create a deep copy of the object.
	/// \param bMakeLinks if true, sets the vn pointers of corresponding
	/// VNODE2s to point at each other.
	/// \returns the new copy.
	PLINE2 * Copy(bool bMakeLinks = false) const;

	/// Adds a new vertex to the pline.
	void AddVertex(const GRID2 & g);

	///	Validates the pline.
	/// This method calculates the orientation and bounding box,
	///	and removes collinear and coincident points.  It must be called
	/// after the pline is created.
	///	\returns true if contour is valid, i.e. Count >= 3 and Area != 0.
	bool    Prepare();

	/// Reverses the orientation of the contour.
	void	Invert();

	/// Adds pline either into area or holes depending on its orientation.
	void Put(PAREA ** area, PLINE2 ** holes);

	/// \returns true if provided point is inside this pline.
	bool GridInside(const GRID2 & g) const;

	/// \returns true if provided pline is completely inside this pline.
	bool PlineInside(const PLINE2 & p) const;

	/// Reverses the line, if necessary, to make it an inner contour.
	void makeInner() { if (IsOuter()) Invert(); }
	/// Reverses the line, if necessary, to make it an outer contour.
	void makeOuter() { if (!IsOuter()) Invert(); }


private:
	/// Adjusts bounding box to include given point.
	void AdjustBox(const GRID2 & g);
	/// Adds a vnode to the pline.
	void InclVnode(VNODE2 &vn);
	/// Check if the given point is inside the bounding box.
	bool GridInBox(const GRID2 & g) const;
};

} // namespace POLYBOOLEAN
#endif // PLINE_H
