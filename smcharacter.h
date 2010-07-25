#pragma once
#include <QFile>

struct CharVertex
{
	double X;
	double Y;
};

class SMCharacter
{
public:
	enum CHARVERTEX_TYPE
	{
		MOVE_TO,	//Pen up
		DRAW_TO,	//Pen down
		TERMINATE	//End of data
	};


private:
	qint32 cVertexCount;
	CharVertex * cVertex;

public:
	SMCharacter();
	SMCharacter(SMCharacter * pChar);
	~SMCharacter();
	void Read(QFile & infile);
	void Write(QFile & outfile);
	double GetMinX(void)
	{
		return(cVertex[0].X);
	}
	double GetMaxX(void)
	{
		return(cVertex[0].Y);
	}
	CHARVERTEX_TYPE GetFirstVertex(CharVertex &pVertex,
		int &pItter);
	CHARVERTEX_TYPE GetNextVertex(CharVertex &pVertex,
		int &pItter);
};

