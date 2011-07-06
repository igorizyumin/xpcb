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

#ifndef GRIDTOOLBARWIDGET_H
#define GRIDTOOLBARWIDGET_H

#include "ui_GridToolbarWidget.h"

class GridToolbarWidget : public QWidget, private Ui::GridToolbarWidget
{
    Q_OBJECT

public:
    explicit GridToolbarWidget(QWidget *parent = 0);

signals:
	void viewGridChanged(int grid);
	void placeGridChanged(int grid);
	void routeGridChanged(int grid);
private:
	int parseUnits(QString str);
private slots:
	void onViewGrid(QString str);
	void onPlaceGrid(QString str);
	void onRouteGrid(QString str);
};

#endif // GRIDTOOLBARWIDGET_H
