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

#ifndef EDITPARTDIALOG_H
#define EDITPARTDIALOG_H

#include "ui_EditPartDialog.h"
#include "global.h"

class Part;

class EditPartDialog : public QDialog, private Ui::EditPartDialog
{
    Q_OBJECT

public:
    explicit EditPartDialog(QWidget *parent = 0);

	void init(Part* p = NULL);

	QPoint pos() const { return QPoint(toPCB(xPos->value()), toPCB(yPos->value())); }
	int angle() const { return angleBox->currentIndex() * 90; }

	bool isPosSet() const { return setPosRadio->isChecked(); }
	PCBSIDE side() const;

private slots:
	void on_unitsBox_currentIndexChanged(const QString &s);

private:
	void updateUnits();
	int toPCB(double value) const { return mInMM ? MM2PCB(value) : MIL2PCB(value); }
	bool mInMM;
	bool mFpSelected;
};

#endif // EDITPARTDIALOG_H
