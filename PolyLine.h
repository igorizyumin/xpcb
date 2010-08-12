// PolyLine.h ... definition of CPolyLine class
//
// A polyline contains one or more contours, where each contour
// is defined by a list of corners and side-styles
// There may be multiple contours in a polyline.
// The last contour may be open or closed, any others must be closed.
// All of the corners and side-styles are concatenated into 2 arrays,
// separated by setting the end_contour flag of the last corner of 
// each contour.
//
// When used for copper areas, the first contour is the outer edge 
// of the area, subsequent ones are "holes" in the copper.
//
// If a CDisplayList pointer is provided, the polyline can draw itself 

#pragma once

#include "global.h"
#include "PCBObject.h"
#include <QList>
#include <QPainter>
#include "gpc.h"


class CArc {
public: 
	enum{ MAX_STEP = 50*25400 };	// max step is 20 mils
	enum{ MIN_STEPS = 18 };		// min step is 5 degrees
	int style;
	int xi, yi, xf, yf;
	int n_steps;	// number of straight-line segments in gpc_poly 
	bool bFound;
};

class CPolyPt
{
public:
	CPolyPt( int qx=0, int qy=0, bool qf=false ):
			x(qx), y(qy), end_contour(qf), utility(0) {}
	int x;
	int y;
	bool end_contour;
	int utility;
};

class PolyLine : public PCBObject
{
public:
	enum { STRAIGHT, ARC_CW, ARC_CCW };	// side styles
	enum { NO_HATCH, DIAGONAL_FULL, DIAGONAL_EDGE }; // hatch styles
	enum { DEF_SIZE = 50, DEF_ADD = 50 };	// number of array elements to add at a time

	// constructors/destructor
	PolyLine();
	~PolyLine();

	// functions for modifying polyline
	void Start( int layer, int w, int x, int y,
		int hatch);
	void AppendCorner( int x, int y, int style = STRAIGHT );
	void InsertCorner( int ic, int x, int y );
	void DeleteCorner( int ic, bool bDraw=true );
	void MoveCorner( int ic, int x, int y );
	void Close( int style = STRAIGHT, bool bDraw=true );
	void RemoveContour( int icont );

	// clipping/combination
	int Clip(bool bRetainArcs=true );
	int TestIntersection( const PolyLine &other );
	int Combine( PolyLine *other );

	// drawing functions
	// XXX move this to editor delegate
//	void HighlightSide( int is );
//	void HighlightCorner( int ic );
//	void StartDraggingToInsertCorner( QPainter *painter, int ic, int x, int y, int crosshair = 1 );
//	void StartDraggingToMoveCorner( QPainter *painter, int ic, int x, int y, int crosshair = 1 );
//	void CancelDraggingToInsertCorner( int ic );
//	void CancelDraggingToMoveCorner( int ic );
	virtual void draw( QPainter *painter, PCBLAYER layer);

	void MoveOrigin( int x_off, int y_off );
	void SetSideVisible( int is, int visible );

	// misc. functions
	QRect GetBounds();
	QRect GetCornerBounds();
	QRect GetCornerBounds( int icont );
	void Copy( PolyLine * src );
	bool TestPointInside( int x, int y );
	bool TestPointInsideContour( int icont, int x, int y );
	void AppendArc( int xi, int yi, int xf, int yf, int xc, int yc, int num );


	// access functions
	int GetNumCorners();
	int GetNumSides();
	bool GetClosed();
	int GetNumContours();
	int GetContour( int ic );
	int GetContourStart( int icont );
	int GetContourEnd( int icont );
	int GetContourSize( int icont );
	const CPolyPt & GetPt(int ic) {return mCorners.at(ic);}
	PCBLAYER GetLayer();
	int GetW();
	int GetSideStyle( int is );
	int GetHatch(){ return mHatch; }
	void SetX( int ic, int x );
	void SetY( int ic, int y );
	void SetEndContour( int ic, bool end_contour );
	void SetLayer( int layer );
	void SetW( int w );
	void SetSideStyle( int is, int style );
	void SetHatch( int hatch ){ mHatch = hatch; }

	// XXX TODO this needs to be moved to the CArea class or something
	int OnModified();
	// XXX move this to pad class?
	PolyLine * MakePolylineForPad( int type, int x, int y, int w, int l, int r, int angle );

	void AddContourForPadClearance( int type, int x, int y, int w, 
						int l, int r, int angle, int fill_clearance,
						int hole_w, int hole_clearance, bool bThermal=false, int spoke_w=0 );

private:
	int TestSelfIntersection();

	// GPC functions
	int MakeGpcPoly( int icontour=0, QList<CArc> * arc_array=NULL );
	int FreeGpcPoly();
	gpc_polygon * GetGpcPoly(){ return mGpcPoly; }
	int NormalizeWithGpc( bool bRetainArcs=false );
	int RestoreArcs( QList<CArc> * arc_array, QList<PolyLine*> * pa=NULL );
	void ClipGpcPolygon( gpc_op op, PolyLine * poly );

	/// Draws the hatch lines.
	void Hatch();

	int mLayer;	// layer to draw on
	int mWidth;		// line width
	QList <CPolyPt> mCorners;	// array of points for corners
	QList <int> mSides;	// array of styles for sides
	int mHatch;	// hatch style, see enum above
	int mNumHatch;	// number of hatch lines
	gpc_polygon * mGpcPoly;	// polygon in gpc format
};
