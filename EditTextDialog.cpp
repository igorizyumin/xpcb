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

#include "EditTextDialog.h"
#include "Text.h"

EditTextDialog::EditTextDialog(QWidget *parent, int numLayers) :
	QDialog(parent), mInMM(false)
{
	setupUi(this);
	populateLayers(numLayers);
}

void EditTextDialog::init(Text *t)
{
	if (t)
	{
		mInMM = false;
		unitsBox->setCurrentIndex(0);
		textEdit->setText(t->text());
		layerBox->setCurrentIndex(mLayerInd.indexOf(t->layer()));
		mirrorImageBox->setChecked(t->isMirrored());
		negativeTextBox->setChecked(t->isNegative());
		setWidthRadio->setChecked(true);
		widthBox->setValue(PCB2MIL(t->strokeWidth()));
		heightBox->setValue(PCB2MIL(t->fontSize()));
		setPosRadio->setChecked(true);
		xPos->setValue(PCB2MIL(t->pos().x()));
		yPos->setValue(PCB2MIL(t->pos().y()));
		angleBox->setCurrentIndex(t->angle() / 90);
	}
	else
	{
		// reset to defaults
		mInMM = false;
		unitsBox->setCurrentIndex(0);
		textEdit->setText("");
		layerBox->setCurrentIndex(0);
		mirrorImageBox->setChecked(false);
		negativeTextBox->setChecked(false);
		negativeTextBox->setEnabled(false);
		heightBox->setValue(100);
		this->defaultWidthRadio->setChecked(true);
		widthBox->setValue(10);
		this->setPosRadio->setChecked(false);
		xPos->setValue(0);
		yPos->setValue(0);
		angleBox->setCurrentIndex(0);
	}
}

PCBLAYER EditTextDialog::layer() const
{
	return mLayerInd[layerBox->currentIndex()];
}

void EditTextDialog::on_unitsBox_currentIndexChanged(const QString &s)
{
	mInMM = (s == "mm");
	updateUnits();
}

void EditTextDialog::updateUnits()
{
	if (!mInMM)
	{
		widthBox->setValue(PCB2MIL(MM2PCB(widthBox->value())));
		heightBox->setValue(PCB2MIL(MM2PCB(heightBox->value())));
		xPos->setValue(PCB2MIL(MM2PCB(xPos->value())));
		yPos->setValue(PCB2MIL(MM2PCB(yPos->value())));

		widthBox->setDecimals(0);
		heightBox->setDecimals(0);
		xPos->setDecimals(0);
		yPos->setDecimals(0);

		widthBox->setSuffix(" mil");
		heightBox->setSuffix(" mil");
		xPos->setSuffix(" mil");
		yPos->setSuffix(" mil");
	}
	else
	{
		widthBox->setDecimals(3);
		heightBox->setDecimals(3);
		xPos->setDecimals(3);
		yPos->setDecimals(3);

		widthBox->setValue(PCB2MM(MIL2PCB(widthBox->value())));
		heightBox->setValue(PCB2MM(MIL2PCB(heightBox->value())));
		xPos->setValue(PCB2MM(MIL2PCB(xPos->value())));
		yPos->setValue(PCB2MM(MIL2PCB(yPos->value())));

		widthBox->setSuffix(" mm");
		heightBox->setSuffix(" mm");
		xPos->setSuffix(" mm");
		yPos->setSuffix(" mm");
	}
}

void EditTextDialog::populateLayers(int numLayers)
{
	layerBox->addItem(layer_str[(int)LAY_SILK_TOP], QVariant((int)LAY_SILK_TOP));
	mLayerInd.append(LAY_SILK_TOP);
	layerBox->addItem(layer_str[(int)LAY_SILK_BOTTOM], QVariant((int)LAY_SILK_BOTTOM));
	mLayerInd.append(LAY_SILK_BOTTOM);
	layerBox->addItem(layer_str[(int)LAY_TOP_COPPER], QVariant((int)LAY_TOP_COPPER));
	mLayerInd.append(LAY_TOP_COPPER);
	for(int i = 0; i < numLayers - 2; i++)
	{
		layerBox->addItem(layer_str[(int)LAY_INNER1+i], QVariant((int)LAY_INNER1+i));
		mLayerInd.append((PCBLAYER)((int)LAY_INNER1 + i));
	}
	layerBox->addItem(layer_str[(int)LAY_BOTTOM_COPPER], QVariant((int)LAY_BOTTOM_COPPER));
	mLayerInd.append(LAY_BOTTOM_COPPER);
}

