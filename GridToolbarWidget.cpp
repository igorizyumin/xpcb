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

#include "GridToolbarWidget.h"
#include "global.h"

GridToolbarWidget::GridToolbarWidget(QWidget *parent) :
	QWidget(parent)
{
    setupUi(this);
	connect(this->placeGridBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onPlaceGrid(QString)));
	connect(this->routGridBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onRouteGrid(QString)));
	connect(this->visGridBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onViewGrid(QString)));
}

int GridToolbarWidget::parseUnits(QString str)
{
	bool in_mm = false;
	if (str.contains("mm")) in_mm = true;
	double val = str.remove(" mm").toDouble();
	if (in_mm) return XPcb::MM2PCB(val);
	else return XPcb::IN2PCB(val/1000);
}

void GridToolbarWidget::onViewGrid(QString str)
{
	emit viewGridChanged(parseUnits(str));
}

void GridToolbarWidget::onPlaceGrid(QString str)
{
	emit placeGridChanged(parseUnits(str));
}

void GridToolbarWidget::onRouteGrid(QString str)
{
	emit routeGridChanged(parseUnits(str));
}
