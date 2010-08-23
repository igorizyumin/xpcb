//               Copyright 1996 Coherent Research Inc.
//      Author:Randy More
//     Created:11/6/96
//   $Revision: 1.3 $
//Last Changed:   7/25/2010  Igor Izyumin
//				  Removed MFC dependencies, ported to Qt
//
//				  $Author: Allan $ $Date: 2003/08/03 23:51:52 $
//                Added this header block and comment blocks

#include <math.h>
#include "smcharacter.h"
#include "smfontutil.h"

#include <QFile>
#include <QPainter>
#include <QList>

static QString smfpath;
static QString smffile = "Hershey.smf";
static QString xtbfile = "Hershey.xtb";

#define BASE_CHARACTER_WIDTH 16.0
#define BASE_CHARACTER_HEIGHT 22.0
#define TEXT_DROP 2
#define WADJUST 1.5
#define HADJUST 1.0

SMFontUtil* SMFontUtil::mInstance = NULL;

SMFontUtil & SMFontUtil::instance()
{
	if (!SMFontUtil::mInstance)
	{
		SMFontUtil::mInstance = new SMFontUtil(smffile);
	}
	return *(SMFontUtil::mInstance);
}


//------------------------------------------------------------------
//+CRi+M Start of CRi method block
//------------------------------------------------------------------
//      Class: SMFontUtil
//
//     Method: SMFontUtil
//
//    Returns: 
//
//  Arguments:
//
//Description:
//         Constructor method for SMFontUtil class
//
//------------------------------------------------------------------
//-CRi-M  End of CRi method block
//------------------------------------------------------------------

SMFontUtil::SMFontUtil(const QString & path)
{
	smfpath = path;
	int outer, inner;
	for(outer = 0; outer < 12; outer++)
	{
		for(inner = 0; inner < 256; inner++)
		{
			cXlationTable[outer][inner]=-1;
		}
	}
	int err = LoadFontData();
	Q_ASSERT(!err);
	LoadXlationData();
}


//------------------------------------------------------------------
//+CRi+M Start of CRi method block
//------------------------------------------------------------------
//      Class: SMFontUtil
//
//     Method: ~SMFontUtil
//
//    Returns: 
//
//  Arguments:
//
//Description:
//         Destructor method for SMFontUtil class
//
//------------------------------------------------------------------
//-CRi-M  End of CRi method block
//------------------------------------------------------------------
SMFontUtil::~SMFontUtil()
{
	foreach(SMCharacter* chr, SMCharList)
	{
		delete chr;
	}
}








//------------------------------------------------------------------
//+CRi+M Start of CRi method block
//------------------------------------------------------------------
//      Class: SMFontUtil
//
//     Method: LoadFontData
//
//    Returns: 0 if success, 1 if error
//
//  Arguments:
//         void
//
//Description:
//         Perform the LoadFontData operation    *AG*
//
//------------------------------------------------------------------
//-CRi-M  End of CRi method block
//------------------------------------------------------------------
int SMFontUtil::LoadFontData(void) 
{

	SMCharacter * chr;
	quint32 numChars;
	QString full_path = smfpath + "/" + smffile;
	QFile infile(full_path);
	if(!infile.open(QIODevice::ReadOnly))
	{
		// AfxMessageBox( "Font stroke file was not found" );
		return 1;
	}

	infile.read((char*)&numChars, 4);
	if (numChars <= 0)
	{
		Q_ASSERT(0);
		return 1;
	}
	for(unsigned int i = 0; i < numChars; i++)
	{
		chr = new SMCharacter();
		chr->Read(infile);
		SMCharList.append(chr);
	}
	infile.close();
	return 0;

}









//------------------------------------------------------------------
//+CRi+M Start of CRi method block
//------------------------------------------------------------------
//      Class: SMFontUtil
//
//     Method: LoadXlationData
//
//    Returns: void
//
//  Arguments:
//         void
//
//Description:
//         Perform the LoadXlationData operation    *AG*
//
//------------------------------------------------------------------
//-CRi-M  End of CRi method block
//------------------------------------------------------------------
int SMFontUtil::LoadXlationData(void)
{
	qint32 i,j,k;
	QString full_path = smfpath + "/" + xtbfile;
	QFile infile(full_path);
	if(!infile.open(QIODevice::ReadOnly))
	{
	//	AfxMessageBox( "Font translation file was not found" );
		return 1;
	}
	else
	{
		for(i=0; i<12; i++)
		{
			for(j=0; j<128; j++)
			{
				if(!infile.atEnd())
				{
					infile.read((char*)(&k), 4);
					cXlationTable[i][j] = k;
				}
			}
		}
		for(i=0; i<12; i++)
		{
			for(j=128; j<256; j++)
			{
				if(!infile.atEnd())
				{
					infile.read((char*)(&k), 4);
					cXlationTable[i][j] = k;
				}
			}
		}
		infile.close();
		return 0;
	}
}









//------------------------------------------------------------------
//+CRi+M Start of CRi method block
//------------------------------------------------------------------
//      Class: SMFontUtil
//
//     Method: SaveXlationData
//
//    Returns: void
//
//  Arguments:
//         void
//
//Description:
//         Perform the SaveXlationData operation    *AG*
//
//------------------------------------------------------------------
//-CRi-M  End of CRi method block
//------------------------------------------------------------------
void SMFontUtil::SaveXlationData(void)
{
	qint32 i,j,k;
	QFile outfile(xtbfile);
	if (!outfile.open(QIODevice::WriteOnly)) return;
	for(i=0; i<12; i++)
	{
		for(j=0; j<128; j++)
		{
			k = cXlationTable[i][j];
			outfile.write((const char*)(&k), 4);
		}
	}
	for(i=0; i<12; i++)
	{
		for(j=128; j<256; j++)
		{
			k = cXlationTable[i][j];
			outfile.write((const char*)(&k), 4);
		}
	}
	outfile.close();
}







//------------------------------------------------------------------
//+CRi+M Start of CRi method block
//------------------------------------------------------------------
//      Class: SMFontUtil
//
//     Method: SaveFontData
//
//    Returns: void
//
//  Arguments:
//         void
//
//Description:
//         Perform the SaveFontData operation    *AG*
//
//------------------------------------------------------------------
//-CRi-M  End of CRi method block
//------------------------------------------------------------------
void SMFontUtil::SaveFontData(void)
{
	quint32 numChars;
	QFile outfile(smffile);
	numChars = SMCharList.size();
	if (!outfile.open(QIODevice::WriteOnly)) return;
	outfile.write((const char*)(&numChars), 4);
	for(quint32 i = 0; i < numChars; i++)
	{
		SMCharList[i]->Write(outfile);
	}
	outfile.close();

}




//------------------------------------------------------------------
//+CRi+M Start of CRi method block
//------------------------------------------------------------------
//      Class: SMFontUtil
//
//     Method: GetCharID
//
//    Returns: int
//
//  Arguments:
//         unsigned char pCharValue
//         FONT_TYPE pFont
//
//Description:
//         Perform the GetCharID operation    *AG*
//
//------------------------------------------------------------------
//-CRi-M  End of CRi method block
//------------------------------------------------------------------
int SMFontUtil::GetCharID(unsigned char pCharValue,
	FONT_TYPE pFont)
{
	int index;
	int charindex;
	index = (int)pFont;
	charindex = pCharValue;
	if(index < 0)
		return(-1);
	if(index > (int)GOTHIC)
		return(-1);
	return(cXlationTable[index][charindex]);
}

#if 0
void SMFontUtil::DrawString(
	QPainter *painter,				//painter to draw to
	const QPoint & pStart,			//starting point
	double pRotation,		//rotation angle clockwise in radians (0 = 12:00)
	double pCharWidth,		//width of each character
	double pCharHeight,		//height of each character
	FONT_TYPE pFontType,	//the font to use
	const QString & pString)		//the string
{
	int curx = pStart.x();
	int cury = pStart.y();
	int length = pString.length();
	double x_scale = (pCharWidth / WADJUST) / BASE_CHARACTER_WIDTH;
	double y_scale = (pCharHeight / HADJUST)  / BASE_CHARACTER_HEIGHT;

	double sin_val = sin(pRotation);
	double cos_val = cos(pRotation);

	double cZoomValue = 1.0;

	double deltax = sin_val * ((pCharWidth / WADJUST) + 2);
	double deltay = cos_val * ((pCharWidth / WADJUST) + 2);

	for(int loop = 0; loop<length; loop++)
	{
		int charid = GetCharID(pString.at(loop),pFontType);
		if(charid>=0)
		{
			CharVertex chrvertex;
			int iter;
			SMCharacter * chr = GetCharacter(charid);
			SMCharacter::CHARVERTEX_TYPE result = 
				chr->GetFirstVertex(chrvertex,iter);
			QPoint pt1, pt2;
			while(result != SMCharacter::TERMINATE)
			{
				chrvertex.X = chrvertex.X * x_scale;
				chrvertex.Y = (chrvertex.Y+TEXT_DROP) * y_scale;
				if(result == SMCharacter::MOVE_TO)
				{
					pt1 = QPoint(
						(int)(((curx + (sin_val * chrvertex.X - 
							cos_val * chrvertex.Y))) / cZoomValue),
						(int)(((cury + (cos_val * chrvertex.X + 
							sin_val * chrvertex.Y))) / cZoomValue));
				}
				else
				{
					pt2 = QPoint((int)(((curx + (sin_val * chrvertex.X -
												 cos_val * chrvertex.Y))) / cZoomValue),
											 (int)(((cury + (cos_val * chrvertex.X +
												 sin_val * chrvertex.Y))) / cZoomValue));
					painter->drawLine(pt1, pt2);
					pt1 = pt2;
				}
				result = chr->GetNextVertex(chrvertex,iter);
			}
		}
		curx += (int)deltax;
		cury += (int)deltay;
	}
}
#endif

// GetCharStrokes ... get description of font character including array of strokes
//
// added by Allan Wright Feb. 2003
//
// enter with:
//		coords = pointer to array[max_coords][4] of doubles
//		max_coords = size of coords array
//
// on return: 
//		return value is number of strokes, or negative number if errors
//		coords array is filled with stroke data where:
//			coords[i][0] = starting x for stroke i	
//			coords[i][1] = starting y for stroke i	
//			coords[i][2] = ending x for stroke i	
//			coords[i][3] = ending y for stroke i
//		min_x, min_y, max_x, max_y are filled with boundary box values
//
int SMFontUtil::GetCharStrokes( char ch, FONT_TYPE pFont, double * min_x, double * min_y, 
				   double * max_x, double * max_y, double coords[][4], int max_strokes )
{
	double xi = 0.0, yi = 0.0;
	double min_xx = 9999999.9, min_yy = 9999999.9;
	double max_xx = -9999999.9, max_yy = -9999999.9;
	double x, y;
	int charid = GetCharID( ch, pFont );
	int n_strokes = 0;
	if( charid>=0 )
	{
		CharVertex chrvertex;
		SMCharacter::CHARVERTEX_TYPE result;
		SMCharacter * chr;
		int iter;
		chr = GetCharacter( charid );
		result = chr->GetFirstVertex( chrvertex, iter );
		while( result != SMCharacter::TERMINATE )
		{
			x = chrvertex.X;
			y = -chrvertex.Y;
			if( x < min_xx )
				min_xx = x;
			if( x > max_xx )
				max_xx = x;
			if( y < min_yy )
				min_yy = y;
			if( y > max_yy )
				max_yy = y;
			if( result == SMCharacter::MOVE_TO )
			{
				xi = x;
				yi = y;
			}
			else if( result == SMCharacter::DRAW_TO )
			{
				if( n_strokes > max_strokes )
					return -2;		// too many strokes 
				if( coords )
				{
					coords[n_strokes][0] = xi;
					coords[n_strokes][1] = yi;
					coords[n_strokes][2] = x;
					coords[n_strokes][3] = y;
				}
				n_strokes++;
				xi = x;
				yi = y;
			}
			result = chr->GetNextVertex( chrvertex, iter );
		}
		if( min_x )
			*min_x = min_xx;
		if( min_y )
			*min_y = min_yy;
		if( max_x )
			*max_x = max_xx;
		if( max_y )
			*max_y = max_yy;
		return n_strokes;
	}
	else
		return -1;		// illegal charid i.e. unable to find character
}

// returns list of QPainterPaths for character
// appends things to the paths list, does not clear bbox
int SMFontUtil::GetCharPath( char ch, FONT_TYPE pFont, QPoint offset, double scale,
							 QRect &bbox, QList<QPainterPath> &paths )
{
	double x, y;

	int charid = GetCharID( ch, pFont );

	if( charid<0 )
		return -1; // illegal charid i.e. unable to find character

	CharVertex chrvertex;
	SMCharacter::CHARVERTEX_TYPE result;
	SMCharacter * chr = GetCharacter( charid );
	int iter;
	result = chr->GetFirstVertex( chrvertex, iter );

	Q_ASSERT(result == SMCharacter::MOVE_TO);
	QPainterPath path(QPoint(int(chrvertex.X * scale), int(-chrvertex.Y * scale)));
	result = chr->GetNextVertex( chrvertex, iter );

	while( result != SMCharacter::TERMINATE )
	{
		x = chrvertex.X * scale;
		y = -chrvertex.Y * scale;

		if( result == SMCharacter::MOVE_TO )
		{
			path.translate(offset);
			paths.append(path);
			bbox |= path.boundingRect().toRect();
			path = QPainterPath(QPoint(x, y));
		}
		else if( result == SMCharacter::DRAW_TO )
			path.lineTo(x, y);
		result = chr->GetNextVertex( chrvertex, iter );
	}
	path.translate(offset);
	paths.append(path);
	bbox |= path.boundingRect().toRect();

	return paths.size();

}
