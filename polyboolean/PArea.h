#ifndef PAREA_H
#define PAREA_H

#include "pbimpl.h"

namespace POLYBOOLEAN
{

struct GRID2;
struct VNODE2;
class PLINE2;

/// Triangle structure
struct PTRIA2
{
	/// Triangle vertices
	VNODE2   *v0, *v1, *v2;
};

/// A PAREA object represents a single polygon (which may also contain holes).  To represent
/// multiple polygons, PAREA objects are connected in a doubly linked list.
class PAREA
{
public:
	/// The boolean operations performed by PolyBoolean.
	enum PBOPCODE
	{
		OR,	///< union
		AND,	///< intersection
		SUB,	///< difference
		XOR	///< symmetrical difference
	};

	/// Forward and backward linked list pointers.
	PAREA *	f, * b;
	/// Pointer to a linked list of contours.  The first contour is always the outline.
	PLINE2 *	cntr;
	/// List of triangles.
	PTRIA2 *	tria;
	/// Number of triangles.
	UINT32	tnum;

	PAREA() : f(this), b(this), cntr(NULL), tria(NULL), tnum(0) {}
	~PAREA();

	/// Deallocates a linked list of areas
	static void Del(PAREA ** p);
	/// Creates a copy of this area and the linked list.
	PAREA * Copy() const;

	/// This routine performs a Boolean operation nOpCode with
	/// two sets of polygons a and b. After the operation is performed,
	/// r will point to the resulting set of polygons.  The input parameters are
	/// not modified.
	/// \param a the first operand.
	/// \param b the second operand.
	/// \param nOpCode the boolean operation to be performed.
	/// \param r pointer to result area pointer
	static PBERRCODE Boolean(const PAREA * a, const PAREA * b, PAREA ** r, PBOPCODE nOpCode);

	/// PolyBoolean0 operates destructively on a and b
	static PBERRCODE Boolean0(PAREA * a, PAREA * b, PAREA ** r, PBOPCODE nOpCode);

	/// This routine triangulates area and assigns its tria and tnum fields.
	/// tria is the array of triangles each consisting of 3 pointers to
	/// corresponding vertices in area.
	static PBERRCODE Triangulate(PAREA * area);

	/// Insert a single pline into an area linked list.
	/// \param area pointer to the pline pointer.  If NULL, a
	/// new area will be created.
	/// \param pline the pline to insert into the area.  May not be part of a linked list.
	/// The PAREA object takes ownership of the pline.
	static void AddPlineToList(PAREA ** area, PLINE2 * pline);
	/// Insert multiple plines into an area linked list.
	/// \param area pointer to the pline pointer.  If NULL, a
	/// new area will be created.
	/// \param pline pointer to the pline list pointer to insert into the area.
	/// The PAREA object takes ownership of the plines.
	static void AddPlinesToList(PAREA ** area, PLINE2 ** holes);

	/// Merges list2 with list1.  list2 will be appended to list1,
	/// then set to NULL.
	static void JoinLists(PAREA ** list1, PAREA ** list2);

#ifndef NDEBUG
	/// Check if coordinates are within 20 bit grid.
	/// \returns true if coordinates are within grid; false otherwise.
	bool CheckDomain();
#endif // NDEBUG

	/// Check if a point is inside any of the PAREAs in linked list.
	/// \returns true if point is inside any PAREA and not inside a hole.
	bool GridInside(const GRID2 & g) const;
	/// Check if a PLINE2 is inside any of the PAREAs in linked list.
	/// \returns true if PLINE2 is inside any PAREA and not inside a hole.
	bool PlineInside(const PLINE2 &p) const;

private:
	/// Adds this area to list
	void AddToList(PAREA ** list);
	/// Adds pline to this area's contour
	void AddPline(PLINE2 * c);
	/// Removes this area from the linked list
	void Remove() { (b->f = f)->b = b; }


}; // struct PAREA


} // namespace POLYBOOLEAN

#endif // PAREA_H
