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

#include "SegmentWidthDialog.h"
#include "ManagePadstacksDialog.h"

Q_DECLARE_METATYPE(QSharedPointer<Padstack>);

SegmentWidthDialog::SegmentWidthDialog(QWidget *parent) :
	QDialog(parent)
{
    setupUi(this);
}

void SegmentWidthDialog::init(const Segment *seg)
{
	if (seg)
	{
		widthBox->setValue(seg->width());
	}
	else
		widthBox->setValue(Dimension(10.0, Dimension::mils));
}

SegmentWidthDialog::ApplyToObj SegmentWidthDialog::applyTo() const
{
	if (this->segButton->isChecked())
		return SEGMENT;
	else if (this->traceButton->isChecked())
		return TRACE;
	else
		return NET;
}

// XXX TODO move this to an edit via dialog
#if 0
QSharedPointer<Padstack> SegmentWidthDialog::padstack() const
{
	if (useDefaultVia->isChecked())
		return QSharedPointer<Padstack>();
	return psList->itemData(psList->currentIndex())
			.value<QSharedPointer<Padstack> >();
}
void SegmentWidthDialog::populatePadstacks()
{
	this->psList->clear();
	foreach(QSharedPointer<Padstack> p, mCtrl->doc()->padstacks())
	{
		psList->addItem(p->name(), QVariant::fromValue(p));
	}
}

void SegmentWidthDialog::populateWidthBox()
{
	// XXX get widths from document
}

void SegmentWidthDialog::accept()
{
	if (!this->padstack() && !this->useDefaultVia->isChecked())
	{
		QMessageBox mb(QMessageBox::Warning, "No padstack selected", "You have not selected a padstack for this pin.  Please select a padstack and try again.", QMessageBox::Ok, this);
		mb.exec();
		return;
	}
	QDialog::accept();
}


void SegmentWidthDialog::on_managePadstacks_clicked()
{
	ManagePadstacksDialog d(this, mDoc);
	d.exec();
	populatePadstacks();
}
#endif

