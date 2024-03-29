/*
	Copyright (C) 2010-2011 Igor Izyumin	
	
	This file is part of xpcb.

	xpcb is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	xpcb is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with xpcb.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef POLYGON_H
#define POLYGON_H

#include <QPoint>
#include <QRect>
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "PolygonList.h"
#include "polybool.h"

class QPainter;

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
		enum SegType {START,		///< the beginning of the contour.
					  LINE,		///< a straight line to the endpoint.
					  ARC_CW,		///< a clockwise arc.
					  ARC_CCW		///< a counterclockwise arc.
					 };

		Segment(SegType t, const QPoint& endPt) :
			type(t), end(endPt) {}

		/// The type of this segment.
		SegType type;

		/// Endpoint coordinates of this segment.  The starting point is the endpoint
		/// of the previous segment, unless this is a START segment.
		QPoint end;
	};



	PolyContour(POLYBOOLEAN::PLINE2 * pline = NULL);

	void appendSegment(const Segment& seg) { mSegs.append(seg); mPbDirty = true;}
	void insertSegment(int pos, const Segment& seg) { mSegs.insert(pos, seg); mPbDirty = true;}
	Segment& segment(int pos) { return mSegs[pos]; }
	const Segment& segment(int pos) const { return mSegs[pos]; }
	int numSegs() const { return mSegs.size(); }
	bool testPointInside(const QPoint& pt) const;

	QRect bbox() const;
	void draw(QPainter *painter) const;

	void translate(const QPoint& vec);

	void toPline(POLYBOOLEAN::PLINE2 ** pline) const;

	static PolyContour newFromXML(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;

private:
	void rebuildPb() const;
	QList<Segment> mSegs;

	mutable bool mPbDirty;
	mutable POLYBOOLEAN::PLINE2 * mPline;
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
	Polygon(const POLYBOOLEAN::PAREA *area);
	Polygon(const Polygon&);
	~Polygon();

	// functions for modifying polygon
	PolyContour* outline() {return &mOutline;}
	PolyContour const* outline() const {return &mOutline;}
	PolyContour* hole(int n) {return &(mHoles[n]); }
	int numHoles() const {return mHoles.size();}
	void removeHole(int n);

	/// Returns the bounding box of the polygon.
	QRect bbox() const;

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
	bool isVoid() const;

	/// Check if a point is within the polygon's
	/// filled area (i.e. inside the outer boundary and outside of a hole/cutout).
	/// \param pt point to test.
	/// \returns true if point is inside the polygon.
	bool testPointInside( const QPoint& pt ) const;

	/// Notifies object that the contours have been modified.
	void markChanged() { mPbDirty = true; }

	/// Returns a copy of this object's PAREA.  Caller is responsible for
	/// deleting the PAREA using PAREA::Del.
	POLYBOOLEAN::PAREA* getParea() const;

	static Polygon newFromXML(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;

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
	mutable POLYBOOLEAN::PAREA *mArea;
};

#endif // POLYGON_H
