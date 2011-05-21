
namespace XPcb
{
%TypeHeaderCode
#include "global.h"
%End
	enum UNIT
	{
		NM,		// nanometers
		MM,		// millimeters
		MIL,	// mils (1/1000 inch)
		MM_MIL,	// both mm and mils (for text output)
		NATIVE	// native units (for text output )
	};

	const int PCBU_PER_MIL;
	const int PCBU_PER_MM;

	const int PCB_BOUND;

	int IN2PCB(double x);
	int MM2PCB(double x);
	int MIL2PCB(double x);
	double PCB2IN(int x);
	double PCB2MM(int x);
	double PCB2MIL(int x);
};

class Layer
{
%TypeHeaderCode
#include "global.h"
%End
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

	Layer(Type c);
//	Layer(int indx);

	Type type() const;
	bool isPhysical() const;
	bool isCopper() const;
	QString name() const;
	QColor color() const;
	int toInt() const;
	static QString name(Type c);
	static QColor color(Type c);
	bool operator==(const Layer& other) const;
	bool operator!=(const Layer& other) const;
};

