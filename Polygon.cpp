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

#include "Polygon.h"
#include "polybool.h"

using namespace POLYBOOLEAN;

static inline int toPbGrid(int x) { return x/4096; }
static inline int fromPbGrid(int x) { return x*4096; }
static inline QPoint ptFromGrid(const GRID2 &g)
{
	return QPoint(fromPbGrid(g.x), fromPbGrid(g.y));
}
static inline GRID2 gridFromPt(const QPoint &pt)
{
	return GRID2(toPbGrid(pt.x()), toPbGrid(pt.y()));
}

//// Polycontour ////
PolyContour::PolyContour(PLINE2 *pline)
	: mPbDirty(false), mPline(NULL)
{
	if (pline)
	{
		// create polycontour from pline
		Q_ASSERT(pline->Count >= 3);
		// iterate over vertices
		VNODE2* vn = pline->head;
		do
		{
			Segment::SEG_TYPE st = Segment::LINE;
			if (vn == pline->head)
				st = Segment::START;
			this->mSegs.append(Segment(st, ptFromGrid(vn->g)));

			vn = vn->next;
		}
		while (vn != pline->head);
	}
}

void PolyContour::toPline(PLINE2 **pline) const
{
	Q_ASSERT((*pline) == NULL);

	rebuildPb();

	Q_ASSERT(mPline != NULL);
	if (mPline)
		(*pline) = mPline->Copy();
}

bool PolyContour::testPointInside(const QPoint &pt) const
{
	rebuildPb();
	if (mPline)
		return mPline->GridInside(gridFromPt(pt));
	else
		return false;
}

void PolyContour::rebuildPb() const
{
	if (!mPbDirty)
		return;

	delete mPline;
	mPline = NULL;

	int n = numSegs();
	Q_ASSERT(n>=3);

	// we ignore arcs for now
	PLINE2 *plnew = new PLINE2(gridFromPt(this->mSegs[0].end));
	for(int i = 1; i < n; i++)
	{
		plnew->AddVertex(gridFromPt(this->mSegs[i].end));
	}
	bool res = plnew->Prepare();
	Q_ASSERT(res);
	if (res)
		mPline = plnew;
	mPbDirty = false;
}

QRect PolyContour::bbox() const
{
	if (numSegs() == 0)
		return QRect();
	int xmin, xmax, ymin, ymax;
	xmin = xmax = mSegs[0].end.x();
	ymin = ymax = mSegs[0].end.y();
	for(int i = 1; i < mSegs.size(); i++)
	{
		const QPoint &pt = mSegs[i].end;
		xmin = (xmin > pt.x()) ? pt.x() : xmin;
		xmax = (xmax < pt.x()) ? pt.x() : xmax;
		ymin = (ymin > pt.y()) ? pt.y() : ymin;
		ymax = (ymax < pt.y()) ? pt.y() : ymax;
	}
	return QRect(xmin, ymin, xmax-xmin, ymax-ymin);
}

PolyContour PolyContour::newFromXML(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "start");

	PolyContour pc;

	while(1)
	{
		QXmlStreamAttributes attr = reader.attributes();
		QPoint endPt = QPoint(
				attr.value("x").toString().toInt(),
				attr.value("y").toString().toInt());

		QStringRef t = reader.name();
		if (t == "start")
		{
			pc.mSegs.append(Segment(Segment::START, endPt));
		}
		else if (t == "lineTo")
		{
			pc.mSegs.append(Segment(Segment::LINE, endPt));
		}
		else if (t == "arcTo")
		{
			Segment::SEG_TYPE t = (attr.value("dir") == "cw")
						 ? Segment::ARC_CW : Segment::ARC_CCW;
			pc.mSegs.append(Segment(t, endPt));
		}
		do
				reader.readNext();
		while(!reader.isEndElement());
		// at end of current element; find next one or end of this contour
		do
				reader.readNext();
		while(!reader.isEndElement() && !reader.isStartElement());
		if (reader.isEndElement())
			break;
	}

	// parser will be at the end of this contour

	return pc;
}

void PolyContour::toXML(QXmlStreamWriter &writer) const
{
	foreach(const Segment& seg, mSegs)
	{
		switch(seg.type)
		{
		case Segment::START:
			writer.writeStartElement("start");
			writer.writeAttribute("x", QString::number(seg.end.x()));
			writer.writeAttribute("y", QString::number(seg.end.y()));
			writer.writeEndElement();
			break;
		case Segment::LINE:
			writer.writeStartElement("lineTo");
			writer.writeAttribute("x", QString::number(seg.end.x()));
			writer.writeAttribute("y", QString::number(seg.end.y()));
			writer.writeEndElement();
			break;
		case Segment::ARC_CW:
		case Segment::ARC_CCW:
			writer.writeStartElement("arcTo");
			writer.writeAttribute("dir", seg.type == Segment::ARC_CW ? "cw" : "ccw");
			writer.writeAttribute("x", QString::number(seg.end.x()));
			writer.writeAttribute("y", QString::number(seg.end.y()));
			writer.writeEndElement();
			break;
		}
	}
}

void PolyContour::translate(const QPoint &vec)
{
	for(int i = 0; i < mSegs.size(); i++)
	{
		mSegs[i].end += vec;
	}
	mPbDirty = true;
}

//// Polygon ////
Polygon::Polygon()
	: mPbDirty(true), mArea(NULL)
{
}

Polygon::Polygon(const PAREA *area)
	:  mPbDirty(true), mArea(NULL)
{
	if (area == NULL || area->cntr == NULL)
		return;

	mOutline = PolyContour(area->cntr);
	// add holes
	PLINE2 *pl = area->cntr->next;
	while(pl != NULL)
	{
		mHoles.append(PolyContour(pl));
		pl = pl->next;
	}
}

Polygon::Polygon(const Polygon &other)
	: mOutline(other.mOutline), mHoles(other.mHoles), mPbDirty(true), mArea(NULL)
{
}

Polygon::~Polygon()
{
	if (mArea)
		PAREA::Del(&mArea);
}

void Polygon::translate( const QPoint& vec )
{
	mOutline.translate(vec);
	foreach(PolyContour p, mHoles)
	{
		p.translate(vec);
	}
	mPbDirty = true;
}

bool Polygon::intersects( const Polygon &other ) const
{
	// test bounding boxes
	if( !bbox().intersects(other.bbox()) )
		return false;

	// check intersection using PB
	rebuildPb();
	other.rebuildPb();
	PAREA *r = NULL;
	PAREA::Boolean(this->mArea, other.mArea, &r, PAREA::AND);
	bool isects = (r != NULL);
	PAREA::Del(&r);
	return isects;
}

PolygonList Polygon::intersected(const Polygon &other) const
{
	// test bounding boxes
	if( !bbox().intersects(other.bbox()) )
		return PolygonList();

	rebuildPb();
	other.rebuildPb();

	PAREA *r = NULL;
	PAREA::Boolean(this->mArea, other.mArea, &r, PAREA::AND);
	PolygonList pl(r);
	PAREA::Del(&r);

	return pl;
}

PolygonList Polygon::united(const Polygon &other) const
{
	// test bounding boxes
	if( !bbox().intersects(other.bbox()) )
	{
		PolygonList p(*this);
		p.insert(new Polygon(other));
		return p;
	}

	rebuildPb();
	other.rebuildPb();

	PAREA *r = NULL;
	PAREA::Boolean(this->mArea, other.mArea, &r, PAREA::OR);
	PolygonList pl(r);
	PAREA::Del(&r);

	return pl;
}

PolygonList Polygon::subtracted(const Polygon &other) const
{
	// test bounding boxes
	if( !bbox().intersects(other.bbox()) )
		return PolygonList(*this);

	rebuildPb();
	other.rebuildPb();

	PAREA *r = NULL;
	PAREA::Boolean(this->mArea, other.mArea, &r, PAREA::SUB);
	PolygonList pl(r);
	PAREA::Del(&r);

	return pl;
}

bool Polygon::testPointInside(const QPoint &pt) const
{
	if (!bbox().contains(pt))
		return false;

	rebuildPb();
	return mArea->GridInside(gridFromPt(pt));
}

void Polygon::rebuildPb() const
{
	if (!mPbDirty)
		return;

	if (mArea)
		PAREA::Del(&mArea);
	PLINE2 *pline;

	// add outline to area
	this->mOutline.toPline(&pline);
	Q_ASSERT(pline);
	if (pline)
	{
		pline->makeOuter();
		PAREA::AddPlineToList(&mArea, pline);
	}

	// add holes to area
	foreach(const PolyContour& pc, mHoles)
	{
		pline = NULL;
		pc.toPline(&pline);
		Q_ASSERT(pline != NULL);
		if (pline)
		{
			pline->makeInner();
			PAREA::AddPlineToList(&mArea, pline);
		}
	}

	mPbDirty = false;
}

PAREA* Polygon::getParea() const
{
	rebuildPb();
	return mArea->Copy();
}

Polygon Polygon::newFromXML(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "polygon");

	Polygon poly;

	reader.readNextStartElement();
	Q_ASSERT(reader.name() == "outline");
	reader.readNextStartElement();
	poly.mOutline = PolyContour::newFromXML(reader);
	while(reader.readNextStartElement())
	{
		Q_ASSERT(reader.name() == "hole");
		QStringRef t = reader.name();

		reader.readNextStartElement();
		poly.mHoles.append(PolyContour::newFromXML(reader));
	}

	return poly;
}

void Polygon::toXML(QXmlStreamWriter &writer) const
{
	if (isVoid()) return;
	writer.writeStartElement("polygon");
	writer.writeStartElement("outline");
	mOutline.toXML(writer);
	writer.writeEndElement();
	foreach(const PolyContour &hole, mHoles)
	{
		writer.writeStartElement("hole");
		hole.toXML(writer);
		writer.writeEndElement();
	}
	writer.writeEndElement();
}

QRect Polygon::bbox() const
{
	return mOutline.bbox();
}

bool Polygon::isVoid() const
{
	return bbox().isEmpty();
}
