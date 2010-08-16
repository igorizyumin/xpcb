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
			SEG_TYPE st = LINE;
			if (vn == pline->head)
				st = START;
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

void PolyContour::rebuildPb()
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

//// Polygon ////
Polygon::Polygon()
	: mArea(NULL), mPbDirty(true)
{
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

static PolygonList Polygon::pbToPolygons(PAREA *a)
{
	if (!a)
		return PolygonList();

	PAREA *curr = a;
	PolygonList plist;

	// iterate over polygons
	do
	{
		// iterate over contours
		Q_ASSERT(curr->cntr != NULL);
		if (curr->cntr)
		{
			// make new polygon
			Polygon p;
			p.mOutline = PolyContour(curr->cntr);
			// add holes
			PLINE2 *pl = curr->cntr->next;
			while(pl != NULL)
			{
				p.mHoles.append(PolyContour(pl));
				pl = pl->next;
			}
			// add to polylist
			plist.append(p);
		}

		curr = curr->f;
	} while(curr != a);

	return plist;
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
	PolygonList pl = ::pbToPolygons(r);
	PAREA::Del(&r);

	return pl;
}

PolygonList Polygon::united(const Polygon &other) const
{
	// test bounding boxes
	if( !bbox().intersects(other.bbox()) )
	{
		PolygonList p(*this);
		p.append(other);
		return p;
	}

	rebuildPb();
	other.rebuildPb();

	PAREA *r = NULL;
	PAREA::Boolean(this->mArea, other.mArea, &r, PAREA::OR);
	PolygonList pl = ::pbToPolygons(r);
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
	PolygonList pl = ::pbToPolygons(r);
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
	foreach(PolyContour& pc, mHoles)
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
