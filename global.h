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

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QColor>
#include <QPoint>
#include <cmath>

namespace XPcb
{
	// units for length
	enum UNIT
	{
		NM,		// nanometers
		MM,		// millimeters
		MIL,	// mils (1/1000 inch)
		MM_MIL,	// both mm and mils (for text output)
		NATIVE	// native units (for text output )
	};

	const int PCBU_PER_MIL = 254;
	const int PCBU_PER_MM = 10000;

	const int PCB_BOUND	= 32000*PCBU_PER_MIL;	// boundary

	// unit conversions
	inline int inchToPcb(double x) { return x * 1000 * PCBU_PER_MIL; }
	inline int mmToPcb(double x) {return x * PCBU_PER_MM; }
	inline int milToPcb(double x) { return x * PCBU_PER_MIL; }
	inline double pcbToInch(int x) { return double(x) / (1000 * PCBU_PER_MIL); }
	inline double pcbToMm(int x) {return double(x) / PCBU_PER_MM; }
	inline double pcbToMil(int x) { return double(x) / PCBU_PER_MIL; }

	// useful math ops
	// signum function
	inline short sign(double x)
	{
		return x > 0 ? 1 : (x < 0 ? -1 : 0);
	}

	// computes perpendicular vector
	inline QPoint perp(const QPoint& v)
	{
		return QPoint(-v.y(), v.x());
	}

	// dot (scalar) product
	inline double dotProd(const QPoint &pt1, const QPoint &pt2)
	{
		return double(pt1.x()) * double(pt2.x())
				+ double(pt1.y()) * double(pt2.y());
	}

	// returns true if vectors are parallel
	inline bool isParallel(const QPoint &dir1, const QPoint &dir2)
	{
		return int(dotProd(dir1, perp(dir2))) == 0;
	}

	// finds the line intersection of the two lines given by (pt1, dir1)
	// and (pt2, dir2)
	// returns scale factor for dir1 vector (intersect. pt = pt1 + retVal * dir1
	// check for parallel-ness before using this
	inline double lineIntersect(const QPoint &pt1, const QPoint &dir1,
						 const QPoint &pt2, const QPoint &dir2)
	{
		QPoint w = pt1 - pt2;
		return (double(dir2.y())*w.x() - double(dir2.x())*w.y()) /
				(double(dir2.x())*dir1.y() - double(dir2.y())*dir1.x());
	}

	// returns line intersection point
	// check for lines being parallel first!
	inline QPoint lineIntersectPt(const QPoint &pt1, const QPoint &dir1,
								  const QPoint &pt2, const QPoint &dir2)
	{
		return pt1 + dir1 * lineIntersect(pt1, dir1, pt2, dir2);
	}

	// computes length of a vector
	inline double norm(const QPoint& pt)
	{
		return std::sqrt(dotProd(pt, pt));
	}

	// computes distance between two points
	inline double distance(const QPoint &pt1, const QPoint& pt2)
	{
		return norm(pt1 - pt2);
	}

	// computes the distance between a point and a line segment.
	inline double distPtToSegment(const QPoint &p,
							  const QPoint &start,
							  const QPoint &end)
	{
		QPoint v = end - start;
		QPoint w = p - start;
		double c1 = dotProd(w, v);
		if (c1 <= 0)
			return distance(p, start);

		double c2 = dotProd(v, v);
		if (c2 <= c1)
			return distance(p, end);

		double b = double(c1) / c2;
		QPoint pb = start + b*v;
		return distance(p, pb);
	}

	// computes the distance between a point and a line
	inline int distPtToLine(const QPoint &p,
							  const QPoint &start,
							  const QPoint &end)
	{
		QPoint v = end - start;
		QPoint w = p - start;
		int c1 = dotProd(w, v);
		int c2 = dotProd(v, v);
		double b = double(c1) / c2;
		QPoint pb = start + b*v;
		return distance(p, pb);
	}
};

class Dimension
{
public:
	enum Unit { mm, mils, pcbu };

	Dimension() : mUnit(pcbu), mValue(0) {}
	Dimension(int value) : mUnit(pcbu), mValue(value) {}
	Dimension(double value, Unit unit = mils);

	int toPcb() const { return mValue; }
	double toMm() const { return XPcb::pcbToMm(mValue); }
	double toMils() const { return XPcb::pcbToMil(mValue); }

	Unit units() const { return mUnit; }
	void setUnits(Unit unit) { mUnit = unit; }

	bool operator==(const Dimension& other) const { return other.mValue == mValue; }
	bool operator!=(const Dimension& other) const { return other.mValue != mValue; }
	bool operator<(const Dimension& rhs) const { return mValue < rhs.mValue; }
	bool operator>(const Dimension& rhs) const { return mValue > rhs.mValue; }
	bool operator<=(const Dimension& rhs) const { return mValue <= rhs.mValue; }
	bool operator>=(const Dimension& rhs) const { return mValue >= rhs.mValue; }

private:
	Unit mUnit;
	int mValue;
};

class Layer
{
public:
	enum Type
	{
		LAY_BACKGND,
		LAY_SELECTION,
		LAY_VISIBLE_GRID,
		LAY_DRC,
		LAY_BOARD_OUTLINE,
		LAY_RAT_LINE,
		LAY_SILK_TOP,
		LAY_SILK_BOTTOM,
		LAY_SMCUT_TOP,
		LAY_SMCUT_BOTTOM,
		LAY_HOLE,
		LAY_TOP_COPPER,
		LAY_BOTTOM_COPPER,
		LAY_INNER1,
		LAY_INNER2,
		LAY_INNER3,
		LAY_INNER4,
		LAY_INNER5,
		LAY_INNER6,
		LAY_INNER7,
		LAY_INNER8,
		LAY_INNER9,
		LAY_INNER10,
		LAY_INNER11,
		LAY_INNER12,
		LAY_INNER13,
		LAY_INNER14,
		LAY_CENTROID,
		LAY_GLUE,
		LAY_PASTE_TOP,
		LAY_PASTE_BOTTOM,
		LAY_START,
		LAY_INNER,
		LAY_END,
		LAY_UNKNOWN
	};

	Layer(Type c = LAY_UNKNOWN)
		: mType(c) {}
	Layer(int indx)
		: mType(static_cast<Type>(indx)) {}

	Type type() const { return mType; }
	bool isPhysical() const;
	bool isCopper() const;
	QString name() const { return Layer::name(mType); }
	QColor color() const { return Layer::color(mType); }
	int toInt() const { return static_cast<int>(mType); }
	static QString name(Type c);
	static QColor color(Type c);
	bool operator==(const Layer& other) const { return mType == other.mType; }
	bool operator==(const Type& other) const { return mType == other; }
	bool operator!=(const Layer& other) const { return mType != other.mType; }
private:
	Type mType;
};

#endif
