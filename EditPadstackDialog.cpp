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

#include "EditPadstackDialog.h"
#include "global.h"
#include <QMessageBox>



/// Returns selection box index for pad shape
int getShapeIndex(Pad::PADSHAPE shape, bool hasDefault)
{
	if (!hasDefault) // for copper selector box (no default option)
	{
		if (shape == Pad::PAD_NONE) return 0;
		return static_cast<int>(shape)-1;
	}
	else
	{
		// swap none and default
		if (shape == Pad::PAD_NONE) return 1;
		if (shape == Pad::PAD_DEFAULT) return 0;
		return static_cast<int>(shape);
	}
}

Pad::PADSHAPE getShape(int index, bool hasDefault)
{
	if (!hasDefault)
	{
		return static_cast<Pad::PADSHAPE>(index + 1);
	}
	else
	{
		if (index == 0) return Pad::PAD_DEFAULT;
		if (index == 1) return Pad::PAD_NONE;
		return static_cast<Pad::PADSHAPE>(index);
	}
}

EditPadstackDialog::EditPadstackDialog(QWidget *parent)
	: QDialog(parent), mInMM(false)
{
    setupUi(this);
	updateUnits();
}

int EditPadstackDialog::toPcb(double unit)
{
	if (mInMM) return XPcb::MM2PCB(unit);
	else return XPcb::MIL2PCB(unit);
}

double EditPadstackDialog::toUnits(int pcbu)
{
	if (mInMM) return XPcb::PCB2MM(pcbu);
	else return XPcb::PCB2MIL(pcbu);
}

void EditPadstackDialog::init(QSharedPointer<Padstack> ps)
{
	psName->setText(ps->name());
	sameAsTopPad->setChecked(false);

	if (ps->isSmt())
	{
		smtRadio->setChecked(true);
		holeDiameter->setValue(0);
	}
	else
	{
		pthRadio->setChecked(true);
		holeDiameter->setValue(toUnits(ps->holeSize()));
	}

	switch(ps->startPad().connFlag())
	{
	case Pad::CONN_DEFAULT:
		topAreaDef->setChecked(true);
		break;
	case Pad::CONN_NEVER:
		topAreaNC->setChecked(true);
		break;
	case Pad::CONN_THERMAL:
		topAreaTherm->setChecked(true);
		break;
	case Pad::CONN_NOTHERMAL:
		topAreaNoTh->setChecked(true);
		break;
	}

	switch(ps->innerPad().connFlag())
	{
	case Pad::CONN_DEFAULT:
		innerAreaDef->setChecked(true);
		break;
	case Pad::CONN_NEVER:
		innerAreaNC->setChecked(true);
		break;
	case Pad::CONN_THERMAL:
		innerAreaTherm->setChecked(true);
		break;
	case Pad::CONN_NOTHERMAL:
		innerAreaNoTh->setChecked(true);
		break;
	}

	switch(ps->endPad().connFlag())
	{
	case Pad::CONN_DEFAULT:
		botAreaDef->setChecked(true);
		break;
	case Pad::CONN_NEVER:
		botAreaNC->setChecked(true);
		break;
	case Pad::CONN_THERMAL:
		botAreaTherm->setChecked(true);
		break;
	case Pad::CONN_NOTHERMAL:
		botAreaNoTh->setChecked(true);
		break;
	}

	topCuShape->setCurrentIndex(getShapeIndex(ps->startPad().shape(), false));
	topCuLength->setValue(toUnits(ps->startPad().length()));
	topCuWidth->setValue(toUnits(ps->startPad().width()));
	topCuRadius->setValue(toUnits(ps->startPad().radius()));

	topSmShape->setCurrentIndex(getShapeIndex(ps->startMask().shape(), true));
	topSmLength->setValue(toUnits(ps->startMask().length()));
	topSmWidth->setValue(toUnits(ps->startMask().width()));
	topSmRadius->setValue(toUnits(ps->startMask().radius()));

	topPmShape->setCurrentIndex(getShapeIndex(ps->startPaste().shape(), true));
	topPmLength->setValue(toUnits(ps->startPaste().length()));
	topPmWidth->setValue(toUnits(ps->startPaste().width()));
	topPmRadius->setValue(toUnits(ps->startPaste().radius()));

	innerShape->setCurrentIndex(getShapeIndex(ps->innerPad().shape(), false));
	innerLength->setValue(toUnits(ps->innerPad().length()));
	innerWidth->setValue(toUnits(ps->innerPad().width()));
	innerRadius->setValue(toUnits(ps->innerPad().radius()));

	botCuShape->setCurrentIndex(getShapeIndex(ps->endPad().shape(), false));
	botCuLength->setValue(toUnits(ps->endPad().length()));
	botCuWidth->setValue(toUnits(ps->endPad().width()));
	botCuRadius->setValue(toUnits(ps->endPad().radius()));

	botSmShape->setCurrentIndex(getShapeIndex(ps->endMask().shape(), true));
	botSmLength->setValue(toUnits(ps->endMask().length()));
	botSmWidth->setValue(toUnits(ps->endMask().width()));
	botSmRadius->setValue(toUnits(ps->endMask().radius()));

	botPmShape->setCurrentIndex(getShapeIndex(ps->endPaste().shape(), true));
	botPmLength->setValue(toUnits(ps->endPaste().length()));
	botPmWidth->setValue(toUnits(ps->endPaste().width()));
	botPmRadius->setValue(toUnits(ps->endPaste().radius()));
}

Padstack EditPadstackDialog::toPadstack()
{
	Padstack out;
	out.setName(psName->text());

	if (smtRadio->isChecked())
		out.setHoleSize(0);
	else
		out.setHoleSize(toPcb(holeDiameter->value()));

	Pad::PADCONNTYPE ct;
	if (topAreaDef->isChecked())
		ct = Pad::CONN_DEFAULT;
	else if (topAreaNC->isChecked())
		ct = Pad::CONN_NEVER;
	else if (topAreaTherm->isChecked())
		ct = Pad::CONN_THERMAL;
	else
		ct = Pad::CONN_NOTHERMAL;
	out.startPad() = Pad(getShape(topCuShape->currentIndex(), false),
						 toPcb(topCuWidth->value()),
						 toPcb(topCuLength->value()),
						 toPcb(topCuRadius->value()),
						 ct);
	out.startMask() = Pad(getShape(topSmShape->currentIndex(), false),
						 toPcb(topSmWidth->value()),
						 toPcb(topSmLength->value()),
						 toPcb(topSmRadius->value()));
	out.startPaste() = Pad(getShape(topPmShape->currentIndex(), false),
						 toPcb(topPmWidth->value()),
						 toPcb(topPmLength->value()),
						 toPcb(topPmRadius->value()));

	if (innerAreaDef->isChecked())
		ct = Pad::CONN_DEFAULT;
	else if (innerAreaNC->isChecked())
		ct = Pad::CONN_NEVER;
	else if (innerAreaTherm->isChecked())
		ct = Pad::CONN_THERMAL;
	else
		ct = Pad::CONN_NOTHERMAL;
	out.innerPad() = Pad(getShape(innerShape->currentIndex(), false),
						 toPcb(innerWidth->value()),
						 toPcb(innerLength->value()),
						 toPcb(innerRadius->value()),
						 ct);

	if (botAreaDef->isChecked())
		ct = Pad::CONN_DEFAULT;
	else if (botAreaNC->isChecked())
		ct = Pad::CONN_NEVER;
	else if (botAreaTherm->isChecked())
		ct = Pad::CONN_THERMAL;
	else
		ct = Pad::CONN_NOTHERMAL;
	if (!sameAsTopPad->isChecked())
	{
		out.endPad() = Pad(getShape(botCuShape->currentIndex(), false),
						   toPcb(botCuWidth->value()),
						   toPcb(botCuLength->value()),
						   toPcb(botCuRadius->value()),
						   ct);
		out.endMask() = Pad(getShape(botSmShape->currentIndex(), false),
							toPcb(botSmWidth->value()),
							toPcb(botSmLength->value()),
							toPcb(botSmRadius->value()));
		out.endPaste() = Pad(getShape(botPmShape->currentIndex(), false),
							 toPcb(botPmWidth->value()),
							 toPcb(botPmLength->value()),
							 toPcb(botPmRadius->value()));
	}
	else
	{
		out.endPad() = out.startPad();
		out.endMask() = out.startMask();
		out.endPaste() = out.startPaste();
		out.endPad().setConnFlag(ct);
	}

	return out;
}

void EditPadstackDialog::updateUnits()
{
	QList<QDoubleSpinBox*> boxes;
	boxes << topCuLength << topCuWidth << topCuRadius
			<< topPmLength << topPmWidth << topPmRadius
			<< topSmLength << topSmWidth << topSmRadius
			<< botCuLength << botCuWidth << botCuRadius
			<< botPmLength << botPmWidth << botPmRadius
			<< botSmLength << botSmWidth << botSmRadius
			<< innerLength << innerWidth << innerRadius
			<< holeDiameter;

	foreach(QDoubleSpinBox* box, boxes)
	{
		if (!mInMM)
		{
			box->setValue(XPcb::PCB2MIL(XPcb::MM2PCB(box->value())));
			box->setDecimals(0);
			box->setSuffix(" mil");
		}
		else
		{
			box->setDecimals(3);
			box->setValue(XPcb::PCB2MM(XPcb::MIL2PCB(box->value())));
			box->setSuffix(" mm");
		}
	}
}

void EditPadstackDialog::on_unitsBox_currentIndexChanged(const QString &s)
{
	mInMM = (s == "mm");
	updateUnits();
}

void EditPadstackDialog::accept()
{
	// validate everything
	// check name
	if(psName->text().isEmpty())
	{
		errorMsg("Padstack must have a name.");
		return;
	}
	// check hole size
	if (pthRadio->isChecked() && holeDiameter->value() == 0)
	{
		errorMsg("Hole size may not be zero for a through-hole pad.");
		return;
	}
	if (smtRadio->isChecked())
	{
		// must have at least one pad
		if (topCuShape->currentIndex() == 0 &&
			(sameAsTopPad->isChecked() || botCuShape->currentIndex() == 0))
		{
			errorMsg("SMT padstack must have at least one copper pad.");
			return;
		}
	}

	// all ok
	QDialog::accept();
}

void EditPadstackDialog::errorMsg(QString err)
{
	QMessageBox msgbox(this);
	msgbox.setText(err);
	msgbox.setWindowTitle("Error");
	msgbox.setIcon(QMessageBox::Warning);
	msgbox.exec();
}

void EditPadstackDialog::on_topCuShape_currentIndexChanged(int i)
{
	switch(i)
	{
	default:
	case 0: // none
		topCuWidth->setEnabled(false);
		topCuLength->setEnabled(false);
		topCuRadius->setEnabled(false);
		break;
	case 1: // round
	case 2: // square
	case 6: // octagon
		topCuWidth->setEnabled(true);
		topCuLength->setEnabled(false);
		topCuRadius->setEnabled(false);
		break;
	case 3: // rect
	case 5: // oval
		topCuWidth->setEnabled(true);
		topCuLength->setEnabled(true);
		topCuRadius->setEnabled(false);
		break;
	case 4:	// rounded-rect
		topCuWidth->setEnabled(true);
		topCuLength->setEnabled(true);
		topCuRadius->setEnabled(true);
		break;
	}
}

void EditPadstackDialog::on_topSmShape_currentIndexChanged(int i)
{
	switch(i)
	{
	default:
	case 0: // default
	case 1: // none
		topSmWidth->setEnabled(false);
		topSmLength->setEnabled(false);
		topSmRadius->setEnabled(false);
		break;
	case 2: // round
	case 3: // square
	case 7: // octagon
		topSmWidth->setEnabled(true);
		topSmLength->setEnabled(false);
		topSmRadius->setEnabled(false);
		break;
	case 4: // rect
	case 6: // oval
		topSmWidth->setEnabled(true);
		topSmLength->setEnabled(true);
		topSmRadius->setEnabled(false);
		break;
	case 5:	// rounded-rect
		topSmWidth->setEnabled(true);
		topSmLength->setEnabled(true);
		topSmRadius->setEnabled(true);
		break;
	}
}

void EditPadstackDialog::on_topPmShape_currentIndexChanged(int i)
{
	switch(i)
	{
	default:
	case 0: // default
	case 1: // none
		topPmWidth->setEnabled(false);
		topPmLength->setEnabled(false);
		topPmRadius->setEnabled(false);
		break;
	case 2: // round
	case 3: // square
	case 7: // octagon
		topPmWidth->setEnabled(true);
		topPmLength->setEnabled(false);
		topPmRadius->setEnabled(false);
		break;
	case 4: // rect
	case 6: // oval
		topPmWidth->setEnabled(true);
		topPmLength->setEnabled(true);
		topPmRadius->setEnabled(false);
		break;
	case 5:	// rounded-rect
		topPmWidth->setEnabled(true);
		topPmLength->setEnabled(true);
		topPmRadius->setEnabled(true);
		break;
	}
}

void EditPadstackDialog::on_botCuShape_currentIndexChanged(int i)
{
	switch(i)
	{
	default:
	case 0: // none
		botCuWidth->setEnabled(false);
		botCuLength->setEnabled(false);
		botCuRadius->setEnabled(false);
		break;
	case 1: // round
	case 2: // square
	case 6: // octagon
		botCuWidth->setEnabled(true);
		botCuLength->setEnabled(false);
		botCuRadius->setEnabled(false);
		break;
	case 3: // rect
	case 5: // oval
		botCuWidth->setEnabled(true);
		botCuLength->setEnabled(true);
		botCuRadius->setEnabled(false);
		break;
	case 4:	// rounded-rect
		botCuWidth->setEnabled(true);
		botCuLength->setEnabled(true);
		botCuRadius->setEnabled(true);
		break;
	}
}

void EditPadstackDialog::on_botSmShape_currentIndexChanged(int i)
{
	switch(i)
	{
	default:
	case 0: // default
	case 1: // none
		botSmWidth->setEnabled(false);
		botSmLength->setEnabled(false);
		botSmRadius->setEnabled(false);
		break;
	case 2: // round
	case 3: // square
	case 7: // octagon
		botSmWidth->setEnabled(true);
		botSmLength->setEnabled(false);
		botSmRadius->setEnabled(false);
		break;
	case 4: // rect
	case 6: // oval
		botSmWidth->setEnabled(true);
		botSmLength->setEnabled(true);
		botSmRadius->setEnabled(false);
		break;
	case 5:	// rounded-rect
		botSmWidth->setEnabled(true);
		botSmLength->setEnabled(true);
		botSmRadius->setEnabled(true);
		break;
	}
}

void EditPadstackDialog::on_botPmShape_currentIndexChanged(int i)
{
	switch(i)
	{
	default:
	case 0: // default
	case 1: // none
		botPmWidth->setEnabled(false);
		botPmLength->setEnabled(false);
		botPmRadius->setEnabled(false);
		break;
	case 2: // round
	case 3: // square
	case 7: // octagon
		botPmWidth->setEnabled(true);
		botPmLength->setEnabled(false);
		botPmRadius->setEnabled(false);
		break;
	case 4: // rect
	case 6: // oval
		botPmWidth->setEnabled(true);
		botPmLength->setEnabled(true);
		botPmRadius->setEnabled(false);
		break;
	case 5:	// rounded-rect
		botPmWidth->setEnabled(true);
		botPmLength->setEnabled(true);
		botPmRadius->setEnabled(true);
		break;
	}
}

void EditPadstackDialog::on_innerShape_currentIndexChanged(int i)
{
	switch(i)
	{
	default:
	case 0: // none
		innerWidth->setEnabled(false);
		innerLength->setEnabled(false);
		innerRadius->setEnabled(false);
		break;
	case 1: // round
	case 2: // square
	case 6: // octagon
		innerWidth->setEnabled(true);
		innerLength->setEnabled(false);
		innerRadius->setEnabled(false);
		break;
	case 3: // rect
	case 5: // oval
		innerWidth->setEnabled(true);
		innerLength->setEnabled(true);
		innerRadius->setEnabled(false);
		break;
	case 4:	// rounded-rect
		innerWidth->setEnabled(true);
		innerLength->setEnabled(true);
		innerRadius->setEnabled(true);
		break;
	}
}
