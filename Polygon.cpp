#include "Polygon.h"

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

bool Polygon::intersects( const Polygon &other ) const
{
	// test bounding boxes
	if( !bbox().intersects(other.bbox()) )
		return false;

	// check if the polygon intersection is empty
	return !(intersection(other).isEmpty());
}
