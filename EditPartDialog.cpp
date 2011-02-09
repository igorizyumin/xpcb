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

#include "EditPartDialog.h"
#include "Part.h"

EditPartDialog::EditPartDialog(QWidget *parent)
	: QDialog(parent), mInMM(false), mFpChanged(false), mCurrFp(NULL)
{
    setupUi(this);
}

void EditPartDialog::on_unitsBox_currentIndexChanged(const QString &s)
{
	mInMM = (s == "mm");
	updateUnits();
}

void EditPartDialog::updateUnits()
{
	if (!mInMM)
	{
		xPos->setValue(PCB2MIL(MM2PCB(xPos->value())));
		yPos->setValue(PCB2MIL(MM2PCB(yPos->value())));

		xPos->setDecimals(0);
		yPos->setDecimals(0);

		xPos->setSuffix(" mil");
		yPos->setSuffix(" mil");
	}
	else
	{
		xPos->setDecimals(3);
		yPos->setDecimals(3);

		xPos->setValue(PCB2MM(MIL2PCB(xPos->value())));
		yPos->setValue(PCB2MM(MIL2PCB(yPos->value())));

		xPos->setSuffix(" mm");
		yPos->setSuffix(" mm");
	}
}

void EditPartDialog::init(Part *p)
{
	unitsBox->setCurrentIndex(0);

	if (p)
	{
		this->refdesEdit->setText(p->refdes());
		this->refdesVis->setChecked(p->refVisible());
		this->valueEdit->setText(p->value());
		this->valueVis->setChecked(p->valueVisible());
		this->fpNameEdit->setText(p->footprint()->name());
		this->descEdit->setText(p->footprint()->desc());
		this->authorEdit->setText(p->footprint()->author());
		this->sourceEdit->setText(p->footprint()->source());
		this->setPosRadio->setChecked(true);
		this->sideBox->setCurrentIndex((int)p->side());
		this->angleBox->setCurrentIndex(p->angle() / 90);
		this->xPos->setValue(PCB2MIL(p->pos().x()));
		this->yPos->setValue(PCB2MIL(p->pos().y()));
		this->mFpChanged = false;
		this->mCurrFp = p->footprint();
	}
	else
	{
		this->mFpChanged = false;
		this->mCurrFp = NULL;
		this->dragPosRadio->setChecked(true);
	}
}
