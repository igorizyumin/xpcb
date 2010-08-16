#ifndef AREA_H
#define AREA_H

#include "PolyLine.h"

class PartPin;
class Vertex;
class Net;
class TraceList;
class PCBDoc;

/// A copper area.

/// Area describes a copper polygon.  Areas can either be unconnected,
/// or they can be assigned to a net.  In the latter case, the area will
/// automatically connect to any vertices, vias, or pins that are within its
/// boundaries.
class Area : public Polygon
{
public:
	/// A list of the possible fill styles for the polygon when drawing.
	enum HATCH_STYLE { NO_HATCH,		///< No hatch lines; only the outline is drawn.
					   DIAGONAL_FULL,	///< The polygon is filled with diagonal hatch lines.
					   DIAGONAL_EDGE	///< Short diagonal hatch lines are drawn on the inside part of the edges.
				   };

	Area();
	~Area();

	/// Sets the polygon layer.
	/// \param layer the new layer.
	void setLayer( PCBLAYER layer );
	/// \returns the current polygon layer.
	PCBLAYER layer() const {return mLayer;}

	/// Gets the hatch style.
	/// \returns the current hatch style
	HATCH_STYLE hatchStyle() const { return mHatchStyle; }
	/// Sets the hatch style.
	/// \param hatch the new hatch style.
	void setHatchStyle( HATCH_STYLE hatch ) { mHatchStyle = hatch; }

	// XXX move this to pad class?
//	Polygon * MakePolylineForPad( int type, int x, int y, int w, int l, int r, int angle );

//	void AddContourForPadClearance( int type, int x, int y, int w,
//						int l, int r, int angle, int fill_clearance,
//						int hole_w, int hole_clearance, bool bThermal=false, int spoke_w=0 );

private:
	/// Parent container
	PCBDoc* mDoc;

	/// Net assigned to this area
	Net* mNet;

	/// Whether to connect SMT pads to this area
	bool mConnectSMT;

	/// List of connected pins
	QList<PartPin*> mConnPins;
	/// List of connected vias and vertices
	QList<Vertex*> mConnVtx;

	/// Layer this polygon is on.
	PCBLAYER mLayer;

	/// Hatch style for drawing this polygon
	HATCH_STYLE mHatchStyle;

};

#endif // AREA_H
