#ifndef AREA_H
#define AREA_H

#include <QSet>
#include "global.h"
#include "PCBObject.h"

class PartPin;
class Vertex;
class Net;
class TraceList;
class PCBDoc;
class Polygon;
class QXmlStreamReader;

/// A copper area.

/// Area describes a copper area.  Areas can either be unconnected,
/// or they can be assigned to a net.  In the latter case, the area will
/// automatically connect to any vertices, vias, or pins that are within its
/// boundaries.  Areas are internally represented as a Polygon object (the area
/// outline, with any cutouts, drawn by the user).  For drawing and DRC, a PolygonList
/// is computed by subtracting all pad/trace clearances from the master polygon.
class Area : public PCBObject
{
public:
	/// A list of the possible fill styles for the polygon when drawing.
	enum HATCH_STYLE { NO_HATCH,		///< No hatch lines; only the outline is drawn.
					   DIAGONAL_FULL,	///< The polygon is filled with diagonal hatch lines.
					   DIAGONAL_EDGE	///< Short diagonal hatch lines are drawn on the inside part of the edges.
				   };

	Area(const PCBDoc *doc);
	~Area();

	virtual void draw(QPainter *painter, PCBLAYER layer);
	virtual QRect bbox() const;

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

	/// Check if a point is within the area boundaries.
	/// \returns true if p is inside area.
	bool pointInside(const QPoint &p) const;

	static Area* newFromXML(QXmlStreamReader &reader, const PCBDoc &doc);

private:
	/// Compute list of connected pins and vertices
	void findConnections();

	/// Parent container
	const PCBDoc* mDoc;

	/// Net assigned to this area
	Net* mNet;

	/// Whether to connect SMT pads to this area
	bool mConnectSMT;

	Polygon *mPoly;

	/// List of connected pins
	QSet<PartPin*> mConnPins;
	/// List of connected vias and vertices
	QSet<Vertex*> mConnVtx;

	/// Layer this polygon is on.
	PCBLAYER mLayer;

	/// Hatch style for drawing this polygon
	HATCH_STYLE mHatchStyle;

};

#endif // AREA_H
