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

#ifndef EDITTEXTDIALOG_H
#define EDITTEXTDIALOG_H

#include <QDialog>
#include "ui_EditTextDialog.h"
#include "global.h"

class Text;

class EditTextDialog : public QDialog, private Ui::EditTextDialog
{
    Q_OBJECT

public:
	explicit EditTextDialog(QWidget *parent = 0);
	void init(Text* t = NULL);
	QPoint pos() const { return QPoint(toPCB(xPos->value()), toPCB(yPos->value())); }
	int angle() const { return angleBox->currentIndex() * 90; }
	bool isMirrored() const { return mirrorImageBox->isChecked(); }
	bool isNegative() const { return negativeTextBox->isChecked(); }
	int textWidth() const { return toPCB(widthBox->value()); }
	int textHeight() const { return toPCB(heightBox->value()); }
	QString text() const { return textEdit->text(); }

	bool isPosSet() const { return setPosRadio->isChecked(); }
	bool isWidthSet() const { return setWidthRadio->isChecked(); }

	PCBLAYER layer() const { return mLayer; }


private slots:
	void on_unitsBox_currentIndexChanged(const QString &s);
	void on_setPosRadio_toggled(bool checked);
	void on_setWidthRadio_toggled(bool checked);

private:
	void updateUnits();
	int toPCB(double value) const { return mInMM ? MM2PCB(value) : MIL2PCB(value); }
	bool mInMM;
	PCBLAYER mLayer;
};

#endif // EDITTEXTDIALOG_H
