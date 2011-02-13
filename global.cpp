/*
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

QString XPcb::layerName(FPLAYER layer)
{
	switch(layer)
	{
	case LAY_FP_SELECTION:
		return "selection";
	case LAY_FP_BACKGND:
		return "background";
	case LAY_FP_VISIBLE_GRID:
		return "visible grid";
	case LAY_FP_SILK_TOP:
		return "top silk";
	case LAY_FP_CENTROID:
		return "centroid";
	case LAY_FP_DOT:
		return "adhesive";
	case LAY_FP_HOLE:
		return "drilled hole";
	case LAY_FP_TOP_MASK:
		return "top mask";
	case LAY_FP_TOP_PASTE:
		return "top paste";
	case LAY_FP_BOTTOM_MASK:
		return "bottom mask";
	case LAY_FP_BOTTOM_PASTE:
		return "bottom paste";
	case LAY_FP_TOP_COPPER:
		return "top copper";
	case LAY_FP_INNER_COPPER:
		return "inner copper";
	case LAY_FP_BOTTOM_COPPER:
		return "bottom copper";
	default:
		return "unknown layer";
	}
}

QString XPcb::layerName(PCBLAYER layer)
{
	switch(layer)
	{
	case LAY_SELECTION:
		return "selection";
	case LAY_BACKGND:
		return "background";
	case LAY_VISIBLE_GRID:
		return "visible grid";
	case LAY_DRC_ERROR:
		return "drc error";
	case LAY_BOARD_OUTLINE:
		return "board outline";
	case LAY_RAT_LINE:
		return "rat line";
	case LAY_SILK_TOP:
		return "top silk";
	case LAY_SILK_BOTTOM:
		return "bottom silk";
	case LAY_SM_TOP:
		return "top sm cutout";
	case LAY_SM_BOTTOM:
		return "bottom sm cutout";
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
	case LAY_MASK_TOP:
		return "top mask";
	case LAY_MASK_BOTTOM:
		return "bottom mask";
	case LAY_PASTE_TOP:
		return "top paste";
	case LAY_PASTE_BOTTOM:
		return "bottom paste";
	default:
		return "unknown layer";
	}
}
