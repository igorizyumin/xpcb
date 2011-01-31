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

#ifndef SELFILTERWIDGET_H
#define SELFILTERWIDGET_H

#include <QSignalMapper>
#include "ui_SelFilterWidget.h"
#include "Controller.h"

class SelFilterWidget : public QWidget, private Ui::SelFilterWidget
{
    Q_OBJECT

public:
    explicit SelFilterWidget(QWidget *parent = 0);

signals:
	void selMaskChanged(Controller::SelectionMaskT type, bool enabled);

private slots:
	void on_partsBox_stateChanged(int state) { emit selMaskChanged(Controller::SM_PARTS, state ? true : false); }
	void on_refdesBox_stateChanged(int state) { emit selMaskChanged(Controller::SM_REFDES, state ? true : false); }
	void on_valueBox_stateChanged(int state) { emit selMaskChanged(Controller::SM_VALUE, state ? true : false); }
	void on_pinsBox_stateChanged(int state) { emit selMaskChanged(Controller::SM_PINS, state ? true : false); }
	void on_tracesBox_stateChanged(int state) { emit selMaskChanged(Controller::SM_TRACES, state ? true : false); }
	void on_verticesBox_stateChanged(int state) { emit selMaskChanged(Controller::SM_VERTICES, state ? true : false); }
	void on_areasBox_stateChanged(int state) { emit selMaskChanged(Controller::SM_AREAS, state ? true : false); }
	void on_textsBox_stateChanged(int state) { emit selMaskChanged(Controller::SM_TEXT, state ? true : false); }
	void on_cutoutsBox_stateChanged(int state) { emit selMaskChanged(Controller::SM_CUTOUTS, state ? true : false); }
	void on_outlineBox_stateChanged(int state) { emit selMaskChanged(Controller::SM_OUTLINE, state ? true : false); }
	void on_drcBox_stateChanged(int state) { emit selMaskChanged(Controller::SM_DRC, state ? true : false); }
};

#endif // SELFILTERWIDGET_H
