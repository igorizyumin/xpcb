// Global definitions
// Layers, units, ids, etc.
#pragma once

#include <QString>
#include <QColor>

namespace XPcb
{
	// units for length
	enum UNIT
	{
		NM,		// nanometers
		MM,		// millimeters
		MIL,	// mils (1/1000 inch)
		MM_MIL,	// both mm and mils (for text output)
		NATIVE	// native units (for text output )
	};

	const int PCBU_PER_MIL = 254;
	const int PCBU_PER_MM = 10000;

	const int PCB_BOUND	= 32000*PCBU_PER_MIL;	// boundary

	inline int IN2PCB(double x) { return x * 1000 * PCBU_PER_MIL; }
	inline int MM2PCB(double x) {return x * PCBU_PER_MM; }
	inline int MIL2PCB(double x) { return x * PCBU_PER_MIL; }
	inline double PCB2IN(int x) { return double(x) / (1000 * PCBU_PER_MIL); }
	inline double PCB2MM(int x) {return double(x) / PCBU_PER_MM; }
	inline double PCB2MIL(int x) { return double(x) / PCBU_PER_MIL; }

};

class Layer
{
public:
	enum Type
	{
		LAY_BACKGND,
		LAY_SELECTION,
		LAY_VISIBLE_GRID,
		LAY_DRC,
		LAY_BOARD_OUTLINE,
		LAY_RAT_LINE,
		LAY_SILK_TOP,
		LAY_SILK_BOTTOM,
		LAY_SMCUT_TOP,
		LAY_SMCUT_BOTTOM,
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
		LAY_CENTROID,
		LAY_GLUE,
		LAY_PASTE_TOP,
		LAY_PASTE_BOTTOM,
		LAY_START,
		LAY_INNER,
		LAY_END,
		LAY_UNKNOWN
	};

	Layer(Type c = LAY_UNKNOWN)
		: mType(c) {}
	Layer(int indx)
		: mType(static_cast<Type>(indx)) {}

	Type type() const { return mType; }
	bool isPhysical() const;
	bool isCopper() const;
	QString name() const { return Layer::name(mType); }
	QColor color() const { return Layer::color(mType); }
	int toInt() const { return static_cast<int>(mType); }
	static QString name(Type c);
	static QColor color(Type c);
	bool operator==(const Layer& other) const { return mType == other.mType; }
	bool operator!=(const Layer& other) const { return mType != other.mType; }
private:
	Type mType;
};

