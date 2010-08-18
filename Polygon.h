#ifndef POLYGON_H
#define POLYGON_H

#include <QPoint>
#include <QRect>
#include <QList>
#include <QXmlStreamReader>
#include "PolygonList.h"

// forward declarations
class PAREA;
class PLINE2;

/// A polygon contour.
/// Describes a single polygon contour.  A contour can represent either the
/// outside boundary of a polygon, or a polygon cutout.  Contours consist of
/// segments, which may be either straight lines and arcs.  Contours are
/// always closed, either implicitly (with a straight line), or explicitly.
/// Contours may not be self-intersecting (segments may not intersect).
class PolyContour
{
public:
	struct Segment
	{

		/// Segment types.
		enum SEG_TYPE {START,		///< the beginning of the contour.
					   LINE,		///< a straight line to the endpoint.
					   ARC_CW,		///< a clockwise arc.
					   ARC_CCW		///< a counterclockwise arc.
				   };

		Segment(SEG_TYPE t, const QPoint& endPt, const QPoint& arcCtr = QPoint() ) :
				type(t), end(endPt), arcCenter(arcCtr), mPbDirty(false), mArea(NULL) {}

		/// The type of this segment.
		SEG_TYPE type;
		/// Endpoint coordinates of this segment.  The starting point is the endpoint
		/// of the previous segment, unless this is a START segment.
		QPoint end;
		/// Coordinates of arc center (if this is an arc).
		QPoint arcCenter;
	};



	PolyContour(PLINE2 * pline = NULL);

	void appendSegment(const Segment& seg) { mSegs.append(seg); mPbDirty = true;}
	void insertSegment(int pos, const Segment& seg) { mSegs.insert(pos, seg); mPbDirty = true;}
	Segment& segment(int pos) { return mSegs[pos]; }
	const Segment& segment(int pos) const { return mSegs[pos]; }
	int numSegs() const { return mSegs.size(); }
	bool testPointInside(const QPoint& pt) const;

	QRect bbox() const;

	void translate(const QPoint& vec);

	void toPline(PLINE2 ** pline) const;

	static PolyContour newFromXML(QXmlStreamReader &reader);

private:
	void rebuildPb();
	QList<Segment> mSegs;

	mutable bool mPbDirty;
	mutable PLINE2 * mPline;
};

/// A polygon object represents a basic polygon type.  A polygon consists of multiple contours,
/// which describe the outer boundary and the inner cutouts (holes).
/// Contour edges may be either arcs or straight lines.  Polygons may not be
/// self-intersecting.  Polygons support union, intersection,
/// and subtraction operations.
class Polygon
{
public:
	Polygon();
	Polygon(PAREA *area);
	~Polygon();

	// functions for modifying polygon
	PolyContour* outline() const {return &mOutline;}
	PolyContour* hole(int n) const;
	int numHoles() const {return mHoles.size();}
	void removeHole(int n);

	/// Returns the bounding box of the polygon.
	QRect bbox();

	/// Returns true if this polygon intersects the one given.
	bool intersects( const Polygon &other ) const;

	/// Computes the intersection (boolean AND) of two polygons.
	/// \param other another polygon to intersect with.
	/// \returns list of resulting polygons.
	PolygonList intersected(const Polygon &other) const;

	/// Computes the union (boolean OR) of two polygons.
	/// \param other another polygon to combine.
	/// \returns list of resulting polygons.
	PolygonList united(const Polygon &other) const;

	/// Subtracts another polygon from this one.
	/// \param rhs polygon to be subtracted from this one.
	/// \returns list of resulting polygons.
	PolygonList subtracted(const Polygon &rhs) const;

	/// Translates this polygon along a vector.
	/// \param vec translation vector.
	void translate( const QPoint& vec );

	/// Checks if the polygon is void (has zero area).
	/// \returns true if polygon is void
	bool isVoid();

	/// Check if a point is within the polygon's
	/// filled area (i.e. inside the outer boundary and outside of a hole/cutout).
	/// \param pt point to test.
	/// \returns true if point is inside the polygon.
	bool testPointInside( const QPoint& pt ) const;

	/// Notifies object that the contours have been modified.
	void markChanged() { mPbDirty = true; }

	/// Returns a copy of this object's PAREA.  Caller is responsible for
	/// deleting the PAREA using PAREA::Del.
	PAREA* getParea();

	static Polygon* newFromXML(QXmlStreamReader &reader);

private:
	/// Rebuilds the PolyBoolean area, if needed.
	void rebuildPb() const;

	/// Polygon outer border.
	PolyContour mOutline;
	/// Polygon cutouts.
	QList<PolyContour> mHoles;

	// PolyBoolean structures
	/// When true, indicates that the PolyBoolean structures
	/// need to be rebuilt.
	mutable bool mPbDirty;
	/// PolyBoolean area corresponding to this polygon
	mutable PAREA *mArea;
};

#endif // POLYGON_H
