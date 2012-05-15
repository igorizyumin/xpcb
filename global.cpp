/*
	Copyright (C) 2010-2011 Igor Izyumin	
	
	This file is part of xpcb.

	xpcb is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	xpcb is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with xpcb.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "global.h"
#include <QSettings>
#include <QPainter>

void XPcb::drawArc(QPainter* painter, QPoint start, QPoint end, bool cw)
{
	int x1, x2, y1, y2;

	// make x/y always be clockwise values
	if (cw)
	{
		x1 = start.x();
		y1 = start.y();
		x2 = end.x();
		y2 = end.y();
	}
	else
	{
		x2 = start.x();
		y2 = start.y();
		x1 = end.x();
		y1 = end.y();
	}
	QRect r;
	int startAngle;

	// figure out quadrant
	if (x1 < x2 && y1 > y2)
	{
		// quadrant 1
		r = QRect(2*x1-x2, 2*y2-y1, 2*(x2-x1), 2*(y1-y2));
		startAngle = 270*16;
	}
	else if (x1 < x2 && y1 < y2)
	{
		// quadrant 2
		r = QRect(x1, 2*y1-y2, 2*(x2-x1), 2*(y2-y1));
		startAngle = 180*16;
	}
	else if (x1 > x2 && y1 < y2)
	{
		// quadrant 3
		r = QRect(x2, y1, 2*(x1-x2), 2*(y2-y1));
		startAngle = 90*16;
	}
	else // x1 > x2, y1 > y2
	{
		// quadrant 4
		r = QRect(2*x2-x1, y2, 2*(x1-x2), 2*(y1-y2));
		startAngle = 0;
	}

	painter->drawArc(r, startAngle, 90*16);
}

Dimension::Dimension(double value, Unit unit)
	: mUnit(unit)
{
	switch(unit)
	{
	case mm:
		mValue = XPcb::mmToPcb(value);
		break;
	case mils:
		mValue = XPcb::milToPcb(value);
		break;
	case pcbu:
		mValue = value;
		break;
	}
}

bool Layer::isPhysical() const
{
	return  mType != LAY_BACKGND
			&& mType != LAY_SELECTION
			&& mType != LAY_VISIBLE_GRID
			&& mType != LAY_DRC
			&& mType != LAY_RAT_LINE;
}

bool Layer::isCopper() const
{
	return (mType >= LAY_TOP_COPPER && mType <= LAY_INNER14);
}

QString Layer::name(Type c)
{
	// Note: remember to update the defaults in main.cpp when changing
	// the names.
	// TODO Need a better system in place for retrieving colors.
	switch(c)
	{
	case LAY_BACKGND:
		return "background";
	case LAY_SELECTION:
		return "selection";
	case LAY_VISIBLE_GRID:
		return "visible grid";
	case LAY_DRC:
		return "drc error";
	case LAY_BOARD_OUTLINE:
		return "board outline";
	case LAY_RAT_LINE:
		return "rat line";
	case LAY_SILK_TOP:
		return "top silk";
	case LAY_SILK_BOTTOM:
		return "bottom silk";
	case LAY_SMCUT_TOP:
		return "top sm cutout";
	case LAY_SMCUT_BOTTOM:
		return "bot sm cutout";
	case LAY_HOLE:
		return "drilled hole";
	case LAY_TOP_COPPER:
		return "top copper";
	case LAY_BOTTOM_COPPER:
		return "bottom copper";
	case LAY_INNER1:
		return "inner 1";
	case LAY_INNER2:
		return "inner 2";
	case LAY_INNER3:
		return "inner 3";
	case LAY_INNER4:
		return "inner 4";
	case LAY_INNER5:
		return "inner 5";
	case LAY_INNER6:
		return "inner 6";
	case LAY_INNER7:
		return "inner 7";
	case LAY_INNER8:
		return "inner 8";
	case LAY_INNER9:
		return "inner 9";
	case LAY_INNER10:
		return "inner 10";
	case LAY_INNER11:
		return "inner 11";
	case LAY_INNER12:
		return "inner 12";
	case LAY_INNER13:
		return "inner 13";
	case LAY_INNER14:
		return "inner 14";
	case LAY_PASTE_TOP:
		return "top paste";
	case LAY_PASTE_BOTTOM:
		return "bottom paste";
	case LAY_START:
		return "start pad";
	case LAY_INNER:
		return "inner pad";
	case LAY_END:
		return "end pad";
	case LAY_CENTROID:
		return "centroid";
	case LAY_GLUE:
		return "adhesive";
	case LAY_UNKNOWN:
		return "unknown layer";
	}
	return "INVALID";
}

QColor Layer::color(Type c)
{
	QSettings s;
	return s.value(QString("colors/%1").arg(Layer::name(c))).value<QColor>();
}
