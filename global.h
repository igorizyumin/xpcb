// Global definitions
// Layers, units, ids, etc.
#pragma once


// units for length
enum UNIT
{
	NM,		// nanometers
	MM,		// millimeters
	MIL,	// mils (1/1000 inch)
	MM_MIL,	// both mm and mils (for text output)
	NATIVE	// native units (for text output )
};

// conversion factors
#define NM_PER_MIL 25400
#define NM_PER_MM 1000000

// there are four coordinate systems:
//	WU = window coords
//	screen coords (pixels)
//	PCBU = PCB coords (nanometers)
//	DU = display coords (mils)
//
// conversion factors
#define PCBU_PER_MIL	NM_PER_MIL
#define PCBU_PER_MM		NM_PER_MM

#define PCB_BOUND	32000*PCBU_PER_MIL	// boundary

typedef enum
{
	SIDE_TOP = 0,
	SIDE_BOTTOM
} PCBSIDE;

// define standard drawing layers
//

#define MAX_LAYERS 32

typedef enum
{
	// layout layers
	LAY_SELECTION = 0,
	LAY_BACKGND,
	LAY_VISIBLE_GRID,
	LAY_HILITE,
	LAY_DRC_ERROR,
	LAY_BOARD_OUTLINE,
	LAY_RAT_LINE,
	LAY_SILK_TOP,
	LAY_SILK_BOTTOM,
	LAY_SM_TOP,
	LAY_SM_BOTTOM,
	LAY_PAD_THRU,
	LAY_TOP_COPPER,
	LAY_BOTTOM_COPPER,
	// invisible layers
	LAY_MASK_TOP = -100,	
	LAY_MASK_BOTTOM = -101,
	LAY_PASTE_TOP = -102,
	LAY_PASTE_BOTTOM = -103,
	LAY_UNKNOWN = -999
} PCBLAYER;

typedef enum
{
	// footprint layers
	LAY_FP_SELECTION = 0,
	LAY_FP_BACKGND,
	LAY_FP_VISIBLE_GRID,
	LAY_FP_HILITE,
	LAY_FP_SILK_TOP,
	LAY_FP_CENTROID,
	LAY_FP_DOT,
	LAY_FP_PAD_THRU,
	LAY_FP_TOP_MASK,
	LAY_FP_TOP_PASTE,
	LAY_FP_BOTTOM_MASK,
	LAY_FP_BOTTOM_PASTE,
	LAY_FP_TOP_COPPER,
	LAY_FP_INNER_COPPER,
	LAY_FP_BOTTOM_COPPER,
	NUM_FP_LAYERS
} FPLAYER;

static char layer_str[32][64] = 
{ 
	"selection",
	"background",
	"visible grid",
	"highlight",
	"DRC error",
	"board outline",
	"rat line",
	"top silk",
	"bottom silk",
	"top sm cutout",
	"bot sm cutout",
	"thru pad",
	"top copper",
	"bottom copper",
	"inner 1",
	"inner 2",
	"inner 3",
	"inner 4",
	"inner 5",
	"inner 6",
	"inner 7",
	"inner 8",
	"inner 9",
	"inner 10",
	"inner 11",
	"inner 12",
	"inner 13",
	"inner 14",
	"inner 15",
	"inner 16",
	"undefined",
	"undefined"
};

static char fp_layer_str[NUM_FP_LAYERS][64] = 
{ 
	"selection",
	"background",
	"visible grid",
	"highlight",
	"top silk",
	"centroid",
	"adhesive",
	"thru pad",
	"top mask",
	"top paste",
	"bottom mask",
	"bottom paste",
	"top copper",
	"inner",
	"bottom"
};

static char layer_char[17] = "12345678QWERTYUI";



// definition of ID structure used by FreePCB
//

// struct id : this structure is used to identify PCB design elements
// such as instances of parts or nets, and their subelements
// Each element will have its own id.
// An id is attached to each item of the Display List so that it can
// be linked back to the PCB design element which drew it.
// These are mainly used to identify items selected by clicking the mouse
//
// In general:
//		id.type	= type of PCB element (e.g. part, net, text)
//		id.st	= subelement type (e.g. part pad, net connection)
//		id.i	= subelement index (zero-based)
//		id.sst	= subelement of subelement (e.g. net connection segment)
//		id.ii	= subsubelement index (zero-based)
//
// For example, the id for segment 0 of connection 4 of net 12 would be
//	id = { ID_NET, 12, ID_CONNECT, 4, ID_SEG, 0 };
//
//
class id {
public:
	// constructor
	id( int qt=0, int qst=0, int qis=0, int qsst=0, int qiis=0 )
	{ type=qt; st=qst; i=qis; sst=qsst; ii=qiis; }
	// operators
	friend int operator ==(id id1, id id2)
	{ return (id1.type==id2.type
			&& id1.st==id2.st
			&& id1.sst==id2.sst
			&& id1.i==id2.i
			&& id1.ii==id2.ii );
	}
	// member functions
	void Clear()
	{ type=0; st=0; i=0; sst=0; ii=0; }
	void Set( int qt, int qst=0, int qis=0, int qsst=0, int qiis=0 )
	{ type=qt; st=qst; i=qis; sst=qsst; ii=qiis; }
	// member variables
	unsigned int type;	// type of element
	unsigned int st;	// type of subelement
	unsigned int i;		// index of subelement
	unsigned int sst;	// type of subsubelement
	unsigned int ii;	// index of subsubelement
};


// these are constants used in ids
// root types
enum {
	ID_NONE = 0,	// an undefined type or st (or an error)
	ID_BOARD,		// board outline
	ID_PART,		// part
	ID_NET,			// net
	ID_TEXT,		// free-standing text
	ID_DRC,			// DRC error
	ID_SM_CUTOUT,	// cutout for solder mask
	ID_CENTROID,	// centroid of footprint
	ID_GLUE,		// adhesive spot
	ID_MULTI		// if multiple selections
};

// subtypes of ID_PART
enum {
	ID_PAD = 1,		// pad_stack in a part
	ID_SEL_PAD,		// selection rectangle for pad_stack in a part
	ID_OUTLINE,		// part outline
	ID_REF_TXT,		// text showing ref num for part
	ID_VALUE_TXT,	// text showing value for part
	ID_FP_TXT,		// free text in footprint
	ID_ORIG,		// part origin
	ID_SEL_RECT,	// selection rectangle for part
	ID_SEL_REF_TXT,		// selection rectangle for ref text
	ID_SEL_VALUE_TXT	// selection rectangle for value text
};

// subtypes of ID_TEXT
enum {
	ID_SEL_TXT = 1,		// selection rectangle
	ID_STROKE			// stroke for text
};

// subtypes of ID_NET
enum {
	ID_ENTIRE_NET = 0,
	ID_CONNECT,		// connection
	ID_AREA			// copper area
};

// subtypes of ID_BOARD
enum {
	ID_BOARD_OUTLINE = 1,
};

// subsubtypes of ID_NET.ID_CONNECT
enum {
	ID_ENTIRE_CONNECT = 0,
	ID_SEG,
	ID_SEL_SEG,
	ID_VERTEX,
	ID_SEL_VERTEX,
	ID_VIA
};

// subsubtypes of ID_NET.ID_AREA, ID_BOARD.ID_BOARD_OUTLINE, ID_SM_CUTOUT
enum {
	ID_SIDE = 1,
	ID_SEL_SIDE,
	ID_SEL_CORNER,
	ID_HATCH,
	ID_PIN_X,	// only used by ID_AREA
	ID_STUB_X	// only used by ID_AREA
};

// subtypes of ID_DRC
// for subsubtypes, use types in DesignRules.h
enum {
	ID_DRE = 1,
	ID_SEL_DRE
};

// subtypes of ID_CENTROID
enum {
	ID_CENT = 1,
	ID_SEL_CENT
};

// subtypes of ID_GLUE
enum {
	ID_SPOT = 1,
	ID_SEL_SPOT
};

