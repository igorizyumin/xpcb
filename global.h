// Global definitions
// Layers, units, ids, etc.
#pragma once

#include <QString>

class XPcb
{
public:
	// units for length
	enum UNIT
	{
		NM,		// nanometers
		MM,		// millimeters
		MIL,	// mils (1/1000 inch)
		MM_MIL,	// both mm and mils (for text output)
		NATIVE	// native units (for text output )
	};

	enum PCBLAYER
	{
		// layout layers
		LAY_SELECTION = 0,
		LAY_BACKGND,
		LAY_VISIBLE_GRID,
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
		NUM_PCB_LAYERS,
		// invisible layers
		LAY_MASK_TOP = -100,
		LAY_MASK_BOTTOM = -101,
		LAY_PASTE_TOP = -102,
		LAY_PASTE_BOTTOM = -103,
		LAY_CURR_ACTIVE = -104,
		LAY_UNKNOWN = -999
	};

	enum FPLAYER
	{
		// footprint layers
		LAY_FP_SELECTION = 0,
		LAY_FP_BACKGND,
		LAY_FP_VISIBLE_GRID,
		LAY_FP_SILK_TOP,
		LAY_FP_CENTROID,
		LAY_FP_DOT,
		LAY_FP_HOLE,
		LAY_FP_TOP_MASK,
		LAY_FP_TOP_PASTE,
		LAY_FP_BOTTOM_MASK,
		LAY_FP_BOTTOM_PASTE,
		LAY_FP_TOP_COPPER,
		LAY_FP_INNER_COPPER,
		LAY_FP_BOTTOM_COPPER,
		NUM_FP_LAYERS
	};

	static const int PCBU_PER_MIL = 254;
	static const int PCBU_PER_MM = 10000;

	static const int PCB_BOUND	= 32000*PCBU_PER_MIL;	// boundary

	static int IN2PCB(double x) { return x * 1000 * PCBU_PER_MIL; }
	static int MM2PCB(double x) {return x * PCBU_PER_MM; }
	static int MIL2PCB(double x) { return x * PCBU_PER_MIL; }
	static double PCB2IN(int x) { return double(x) / (1000 * PCBU_PER_MIL); }
	static double PCB2MM(int x) {return double(x) / PCBU_PER_MM; }
	static double PCB2MIL(int x) { return double(x) / PCBU_PER_MIL; }

	static QString layerName(PCBLAYER layer);
	static QString layerName(FPLAYER layer);

private:
	XPcb() {}
	XPcb(XPcb &) {}

};


