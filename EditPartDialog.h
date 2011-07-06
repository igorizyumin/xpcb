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

#ifndef EDITPARTDIALOG_H
#define EDITPARTDIALOG_H

#include "ui_EditPartDialog.h"
#include "global.h"
#include "Part.h"
#include "Net.h"

class EditPartDialog : public QDialog, private Ui::EditPartDialog
{
    Q_OBJECT

public:
	explicit EditPartDialog(QWidget *parent, PCBDoc* doc);

	void init(QSharedPointer<Part> p = QSharedPointer<Part>());

	QPoint pos() const { return QPoint(toPCB(xPos->value()), toPCB(yPos->value())); }
	int angle() const { return angleBox->currentIndex() * 90; }

	bool isPosSet() const { return setPosRadio->isChecked(); }
	Part::SIDE side() const { return this->sideBox->currentIndex() == 0 ? Part::SIDE_TOP : Part::SIDE_BOTTOM; }

	QString ref() const { return this->refdesEdit->text(); }
	QString value() const { return this->valueEdit->text(); }
	bool refVisible() const { return this->refdesVis->checkState(); }
	bool valueVisible() const { return this->valueVis->checkState(); }

	QUuid footprint() const { return mCurrFpUuid; }

private slots:
	void on_unitsBox_currentIndexChanged(const QString &s);
	void on_fpSelButton_clicked();
protected:
	virtual void accept();
private:
	void updateUnits();
	void updateFp();
	int toPCB(double value) const { return mInMM ? XPcb::MM2PCB(value) : XPcb::MIL2PCB(value); }
	bool mInMM;
	bool mFpChanged;
	QUuid mCurrFpUuid;
	QSharedPointer<Part> mPart;
	PCBDoc* mDoc;
};

#endif // EDITPARTDIALOG_H
