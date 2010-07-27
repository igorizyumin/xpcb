#ifndef NET_H
#define NET_H

#include <QString>
#include <QList>
#include <QSet>
#include "global.h"
#include "Part.h"

// Basic types




// cconnect: describes a connection between two pins or a stub trace with no end pin
class Connection
{
public:
	Connection()
	{	// constructor
		locked = 0;
		seg.SetSize( 0 );
		vtx.SetSize( 0 );
		utility = 0;
	}
	Connection(PartPin* start, PartPin* end);

	bool hasPin(const PartPin* pin) {return start_pin == pin || end_pin == pin;}
	PartPin* startPin() {return start_pin;}
	PartPin* endPin() {return end_pin;}
	bool isVoid() {return seg.size() == 0 || vtx.size() == 0;}

private:
	PartPin* start_pin;
	PartPin* end_pin;
	int locked;					// 1 if locked (will not be optimized away)
	QList<Segment> seg;			// array of segments
	QList<Vertex> vtx;		// array of vertices, size = nsegs + 1
#if 0
	int utility;				// used for various temporary ops
	// these params used only by DRC
	int min_x, max_x;			// bounding rect
	int min_y, max_y;
	bool vias_present;
	int seg_layers;
#endif
};


// Vertex: describes a vertex between segments
class Vertex
{
public:
	Vertex()
	{
		// constructor
		coords = QPoint(0,0);
		pad_layer = 0;	// only for first or last
		force_via_flag = false;		// only used for end of stub trace
		via_w = 0;
		via_hole_w = 0;
		tee_ID = false;
		utility = 0;
		utility2 = 0;
	}
	Vertex(QPoint pos, PCBLAYER padLayer)
	{
		// constructor
		coords = pos;
		pad_layer = padLayer;
		force_via_flag = false;
		via_w = 0;
		via_hole_w = 0;
		tee_ID = false;
		utility = 0;
		utility2 = 0;
	}

	QPoint pos() {return coords;}

private:
	QPoint coords;					// coords
	PCBLAYER pad_layer;				// layer of pad if this is first or last vertex, otherwise 0
	bool force_via;				// force a via even if no layer change
	int via_w, via_hole_w;		// via width and hole width (via_w==0 means no via)
	bool isTee;					// used to flag a t-connection point
	int utility, utility2;		// used for various functions
};

// Segment: describes a segment of a connection
class Segment
{
public:
	Segment(PCBLAYER l = LAY_RAT_LINE, int w = 0) :
			layer(l), width(w), selected(false), utility(0) {}
private:
	PCBLAYER layer;
	int width;
	bool selected;			// true if selected for editing
	int utility;
};

// carea: describes a copper area
class Area
{
public:
	Area();
	~Area();						// destructor
	CPolyLine * poly;	// outline
	int npins;			// number of thru-hole pins within area on same net
	QList<int> pin;	// array of thru-hole pins
	int nvias;			// number of via connections to area
	QList<int> vcon;	// connections
	QList<int> vtx;	// vertices
	int utility, utility2;
};

// Net: describes a net
class Net
{
public:
	Net( QString name, int def_width, int def_via_w, int def_via_hole_w );
	~Net();

	QString getName() {return name;}
	void moveOrigin(QPoint delta);
	void AddPin( PartPin * pin, bool set_areas=true );
	PartPin* TestHitOnPin( QPoint pt, PCBLAYER layer);

private:
	id id;				// net id
	QString name;		// net name
	QList<Connection> connect; // array of connections (size = max_pins-1)
	QSet<PartPin*> pin;	// pointers to part pins that are in this net
	QList<Area> area;	// array of copper areas
	int def_w;			// default trace width
	int def_via_w;		// default via width
	int def_via_hole_w;	// default via hole width
	bool visible;		// false to hide ratlines and make unselectable
	int utility;		// used to keep track of which nets have been optimized
	int utility2;		// used to keep track of which nets have been optimized
};

#endif // NET_H
