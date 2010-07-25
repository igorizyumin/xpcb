#ifndef NETLIST_H
#define NETLIST_H

#include <QString>
#include <QList>
#include "PolyLine.h"

// Forward declarations
class Net;
class Pin;
class Connection;
class Segment;
class Vertex;
class Part;
class Area;

// Basic types


// Net: describes a net
class Net
{
public:
	id id;				// net id
	QString name;		// net name
	int nconnects;		// number of connections
	QList<Connection> connect; // array of connections (size = max_pins-1)
	int npins;			// number of pins
	QList<Pin> pin;	// array of pins
	int nareas;			// number of copper areas
	QList<Area,Area> area;	// array of copper areas
	int def_w;			// default trace width
	int def_via_w;		// default via width
	int def_via_hole_w;	// default via hole width
	bool visible;		// FALSE to hide ratlines and make unselectable
	int utility;		// used to keep track of which nets have been optimized
	int utility2;		// used to keep track of which nets have been optimized
};

// cconnect: describes a connection between two pins or a stub trace with no end pin
class Connection
{
public:
	enum {
		NO_END = -1		// used for end_pin if stub trace
	};
	Connection()
	{	// constructor
		locked = 0;
		nsegs = 0;
		seg.SetSize( 0 );
		vtx.SetSize( 0 );
		utility = 0;
	};
	int start_pin, end_pin;		// indexes into net.pin array
	int nsegs;					// # elements in seg array
	int locked;					// 1 if locked (will not be optimized away)
	QList<Segment> seg;			// array of segments
	QList<Vertex> vtx;		// array of vertices, size = nsegs + 1
	int utility;				// used for various temporary ops
	// these params used only by DRC
	int min_x, max_x;			// bounding rect
	int min_y, max_y;
	bool vias_present;
	int seg_layers;
};


// Vertex: describes a vertex between segments
class Vertex
{
public:
	Vertex()
	{
		// constructor
		m_dlist = 0;	// this must set with Initialize()
		x = 0; y = 0;
		pad_layer = 0;	// only for first or last
		force_via_flag = 0;		// only used for end of stub trace
		via_w = 0;
		via_hole_w = 0;
		tee_ID = 0;
		utility = 0;
		utility2 = 0;
	}
	int x, y;					// coords
	int pad_layer;				// layer of pad if this is first or last vertex, otherwise 0
	int force_via_flag;			// force a via even if no layer change
	int via_w, via_hole_w;		// via width and hole width (via_w==0 means no via)
	int tee_ID;					// used to flag a t-connection point
	int utility, utility2;		// used for various functions
};

// Segment: describes a segment of a connection
class Segment
{
public:
	Segment()
	{
		// constructor
		layer = 0;
		width = 0;
		selected = 0;
		utility = 0;
	}
	int layer;				// copper layer
	int width;				// width
	int selected;			// 1 if selected for editing
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

// cpin: describes a pin in a net
class Pin
{
public:
	Pin(){ part = NULL; }
	QString ref_des;	// reference designator such as 'U1'
	QString pin_name;	// pin name such as "1" or "A23"
	Part * part;		// pointer to part containing the pin
	int utility;
};



#endif // NETLIST_H
