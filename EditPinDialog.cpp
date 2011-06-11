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

#include "EditPinDialog.h"
#include "PCBDoc.h"
#include "ManagePadstacksDialog.h"
#include "Shape.h"

Q_DECLARE_METATYPE(Padstack*);

EditPinDialog::EditPinDialog(QWidget *parent, Document *doc) :
	QDialog(parent), mInMM(false), mPin(NULL), mDoc(doc)
{
    setupUi(this);
	updatePsList();
}

void EditPinDialog::init(Pin *p)
{
	mPin = p;
	if (mPin)
	{
		// name
		pinName->setText(mPin->name());
		// padstack
		Padstack* ps = mPin->padstack();
		psList->setCurrentIndex(psList->findData(QVariant::fromValue(ps)));
		// position
		this->setPosRadio->setChecked(true);
		xPosBox->setValue(toUnits(mPin->pos().x()));
		yPosBox->setValue(toUnits(mPin->pos().y()));
		angleBox->setCurrentIndex(mPin->angle() / 90);
		// disable add row checkbox
		addRowCheck->setChecked(false);
		addRowCheck->setEnabled(false);
	}
}

void EditPinDialog::on_unitsBox_currentIndexChanged(const QString &s)
{
	mInMM = (s == "mm");
	updateUnits();
}

void EditPinDialog::updateUnits()
{
	const int numBoxes = 3;
	QDoubleSpinBox* const boxes[] = { xPosBox, yPosBox, spacingBox };

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

void EditPinDialog::updatePsList()
{
	psList->clear();
	foreach(Padstack* p, mDoc->padstacks())
	{
		psList->addItem(p->name(), QVariant::fromValue(p));
	}
}

void EditPinDialog::on_manageBtn_clicked()
{
	ManagePadstacksDialog d(this, mDoc);
	d.exec();
	updatePsList();
}

QList<Pin*> EditPinDialog::makePins(Footprint* fp)
{
	// initial position
	QPoint firstPos(0, 0);
	int firstAngle = 0;
	// get padstack from list
	Padstack* ps = padstack();
	Q_ASSERT(ps);
	// set initial position to user value if set
	if (!dragToPos())
	{
		firstPos = pos();
		firstAngle = angle();
	}

	// make first pin
	Pin* first = new Pin(fp);
	QString name = this->name();
	first->setName(name);
	first->setPadstack(ps);
	first->setPos(firstPos);
	first->setAngle(firstAngle);

	// return just this pin if box is not checked
	QList<Pin*> list;
	list.append(first);
	if (!addRowCheck->isChecked())
		return list;

	// split the name into the basename and number components
	// for example: pin name = A1B2C5
	// basename = A1B2C
	// number = 5
	QRegExp re("^((?:.*\\D)?)(\\d*)$");
	if (!re.exactMatch(name))
		Q_ASSERT(false);
	QString baseName = re.cap(1);
	int startNum = 0;
	if (re.cap(2) != "")
		startNum = re.cap(2).toInt();

	QPoint delta;
	if (orientBox->currentIndex() == 0) // horizontal
		delta.setX(toPcb(spacingBox->value()));
	else
		delta.setY(toPcb(spacingBox->value()));

	// add other pins
	for(int i = 1; i < numPinsBox->value(); i++)
	{
		Pin* p = new Pin(fp);
		p->setName(baseName + QString::number(startNum + i*incrementBox->value()));
		p->setPadstack(ps);
		p->setAngle(firstAngle);
		p->setPos(firstPos + delta * i);
		list.append(p);
	}

	return list;
}

int EditPinDialog::toPcb(double unit) const
{
	if (mInMM) return XPcb::MM2PCB(unit);
	else return XPcb::MIL2PCB(unit);
}

double EditPinDialog::toUnits(int pcbu) const
{
	if (mInMM) return XPcb::PCB2MM(pcbu);
	else return XPcb::PCB2MIL(pcbu);
}

Padstack* EditPinDialog::padstack() const
{
	return psList->itemData(psList->currentIndex()).value<Padstack*>();
}