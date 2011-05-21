#pragma once
#include "smcharacter.h"
#include <QList>
#include <QPainter>

enum FONT_TYPE
{
	SMALL_SIMPLEX,
	SMALL_DUPLEX,
	SIMPLEX,
	DUPLEX,
	TRIPLEX,
	MODERN,
	SCRIPT_SIMPLEX,
	SCRIPT_DUPLEX,
	ITALLIC_DUPLEX,
	ITALLIC_TRIPLEX,
	FANCY,
	GOTHIC
};


class SMFontUtil
{
public:
	/// Get singleton instance.
	static SMFontUtil& instance();

#if 0
	void DrawString(
			QPainter * painter,				//device context to draw to
			QPoint pStart,			//starting point
			double pRotation,		//rotation angle clockwise in radians (0 = 12:00)
			double pCharWidth,		//width of each character
			double pCharHeight,		//height of each character
			FONT_TYPE pFontType,	//the font to use
			QString pString);		//the string
#endif

	int GetMaxChar(void) {	return(SMCharList.size() - 1); }

	void AddCharacter(SMCharacter * pChar)
	{
		SMCharList.append(pChar);
	}

	void SetCharID(unsigned char pCharValue,
		FONT_TYPE pFont, int ID)
	{
		cXlationTable[static_cast<int>(pFont)][static_cast<int>(pCharValue)] = ID;
	}

	SMCharacter * GetCharacter(int pCharID)
	{
		return(SMCharList[pCharID]);
	}

	int GetCharID(unsigned char pCharValue,
		FONT_TYPE pFont);

	// added by Allan Wright Feb. 2002
	int GetCharStrokes( char ch, FONT_TYPE pFont, double * min_x, double * min_y, 
			double * max_x, double * max_y, double coords[][4], int max_strokes );
	int GetCharPath( char ch, FONT_TYPE pFont, QPoint offset, double scale,
					 QRect &bbox, QList<QPainterPath> &paths );
	void GetStrokes(const QString &text, int fontSize, int strokeWidth, QList<QPainterPath> &paths, QRect &bbox);

private:
	QList<SMCharacter*> SMCharList;
	int cXlationTable[12][256];
	int LoadFontData(void);
	void SaveFontData(void);
	int LoadXlationData(void);
	void SaveXlationData(void);
	SMFontUtil();
	~SMFontUtil();

	static SMFontUtil* mInstance;

};

