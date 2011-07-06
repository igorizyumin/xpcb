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

#include "EditLineDialog.h"

EditLineDialog::EditLineDialog(QWidget *parent) :
	QDialog(parent), mInMM(false)
{
    setupUi(this);
}

Layer EditLineDialog::layer() const
{
	return layerBox->currentIndex() == 0 ?
				Layer(Layer::LAY_SILK_TOP)
			  : Layer(Layer::LAY_SILK_BOTTOM);
}

void EditLineDialog::updateUnits()
{
	const int numBoxes = 1;
	QDoubleSpinBox* const boxes[numBoxes] = { widthBox };

	for(int i = 0; i < numBoxes; i++)
	{
		if (!mInMM)
		{
			boxes[i]->setValue(XPcb::PCB2MIL(XPcb::MM2PCB(boxes[i]->value())));
			boxes[i]->setDecimals(0);
			boxes[i]->setSuffix(" mil");
		}
		else
		{
			boxes[i]->setDecimals(3);
			boxes[i]->setValue(XPcb::PCB2MM(XPcb::MIL2PCB(boxes[i]->value())));
			boxes[i]->setSuffix(" mm");
		}
	}
}

int EditLineDialog::toPcb(double unit) const
{
	if (mInMM) return XPcb::MM2PCB(unit);
	else return XPcb::MIL2PCB(unit);
}

double EditLineDialog::toUnits(int pcbu) const
{
	if (mInMM) return XPcb::PCB2MM(pcbu);
	else return XPcb::PCB2MIL(pcbu);
}

void EditLineDialog::init(QSharedPointer<Line> line)
{
	widthBox->setValue(toUnits(line->width()));
	int i = line->layer() == Layer::LAY_SILK_TOP ? 0 : 1;
	layerBox->setCurrentIndex(i);
}

void EditLineDialog::on_unitsBox_currentIndexChanged(const QString &s)
{
	mInMM = (s == "mm");
	updateUnits();
}
