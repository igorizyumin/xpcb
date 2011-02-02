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
const int NM_PER_MIL = 25400;
const int NM_PER_MM = 1000000;

// there are four coordinate systems:
//	WU = window coords
//	screen coords (pixels)
//	PCBU = PCB coords (nanometers)
//	DU = display coords (mils)
//
// conversion factors
const int PCBU_PER_MIL = NM_PER_MIL;
const int PCBU_PER_MM = NM_PER_MM;

const int PCB_BOUND	= 32000*PCBU_PER_MIL;	// boundary

inline int IN2PCB(double x) { return x * 1000 * PCBU_PER_MIL; }
inline int MM2PCB(double x) {return x * PCBU_PER_MM; }
inline int MIL2PCB(double x) { return x * PCBU_PER_MIL; }
inline double PCB2IN(int x) { return double(x) / (1000 * PCBU_PER_MIL); }
inline double PCB2MM(int x) {return double(x) / PCBU_PER_MM; }
inline double PCB2MIL(int x) { return double(x) / PCBU_PER_MIL; }

typedef enum
{
	SIDE_TOP = 0,
	SIDE_BOTTOM
} PCBSIDE;

// define standard drawing layers
//

const int MAX_LAYERS = 27;

typedef enum
{
	// layout layers
	LAY_SELECTION = 0,
	LAY_BACKGND,
	LAY_VISIBLE_GRID,
//	LAY_HILITE, (is this redundant?)
	LAY_DRC_ERROR,
	LAY_BOARD_OUTLINE,
	LAY_RAT_LINE,
	LAY_SILK_TOP,
	LAY_SILK_BOTTOM,
	LAY_SM_TOP,
	LAY_SM_BOTTOM,
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
//	LAY_INNER15,
//	LAY_INNER16,
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
//	LAY_FP_HILITE,
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

static char layer_str[30][64] =
{ 
	"selection",
	"background",
	"visible grid",
//	"highlight",
	"DRC error",
	"board outline",
	"rat line",
	"top silk",
	"bottom silk",
	"top sm cutout",
	"bot sm cutout",
	"drilled hole",
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
	"inner 16"
};

static char fp_layer_str[NUM_FP_LAYERS][64] = 
{ 
	"selection",
	"background",
	"visible grid",
//	"highlight",
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
