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

#include <QMessageBox>
#include "EditPartDialog.h"
#include "Part.h"
#include "Document.h"
#include "SelectFPDialog.h"

EditPartDialog::EditPartDialog(QWidget *parent, PCBDoc* doc)
	: QDialog(parent), mInMM(false), mFpChanged(false), mPart(NULL), mDoc(doc)
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
		xPos->setValue(XPcb::pcbToMil(XPcb::mmToPcb(xPos->value())));
		yPos->setValue(XPcb::pcbToMil(XPcb::mmToPcb(yPos->value())));

		xPos->setDecimals(0);
		yPos->setDecimals(0);

		xPos->setSuffix(" mil");
		yPos->setSuffix(" mil");
	}
	else
	{
		xPos->setDecimals(3);
		yPos->setDecimals(3);

		xPos->setValue(XPcb::pcbToMm(XPcb::milToPcb(xPos->value())));
		yPos->setValue(XPcb::pcbToMm(XPcb::milToPcb(yPos->value())));

		xPos->setSuffix(" mm");
		yPos->setSuffix(" mm");
	}
}

void EditPartDialog::init(QSharedPointer<Part> p)
{
	unitsBox->setCurrentIndex(0);
	mPart = p;
	if (p)
	{
		this->refdesEdit->setText(p->refdes());
		this->refdesVis->setChecked(p->refVisible());
		this->valueEdit->setText(p->value());
		this->valueVis->setChecked(p->valueVisible());
		this->mCurrFpUuid = p->footprint()->uuid();
		updateFp();
		this->setPosRadio->setChecked(true);
		this->sideBox->setCurrentIndex(static_cast<int>(p->side()));
		this->angleBox->setCurrentIndex(p->angle() / 90);
		this->xPos->setValue(XPcb::pcbToMil(p->pos().x()));
		this->yPos->setValue(XPcb::pcbToMil(p->pos().y()));
		this->mFpChanged = false;
	}
	else
	{
		this->mFpChanged = false;
		this->dragPosRadio->setChecked(true);
	}
}

void EditPartDialog::updateFp()
{
	QSharedPointer<Footprint> fp = mDoc->getFootprint(mCurrFpUuid);
	if (fp.isNull()) return;
	this->fpNameEdit->setText(fp->name());
	this->descEdit->setText(fp->desc());
	this->authorEdit->setText(fp->author());
	this->sourceEdit->setText(fp->source());
}

void EditPartDialog::on_fpSelButton_clicked()
{
	QScopedPointer<SelectFPDialog> d(new SelectFPDialog(this));
	int res = d->exec();
	if (res == QDialog::Accepted && d->isFpSelected())
	{
		mCurrFpUuid = d->uuid();
		mFpChanged = true;
		updateFp();
	}
}

void EditPartDialog::accept()
{
	if(this->refdesEdit->text().isEmpty())
	{
		QMessageBox mb(QMessageBox::Warning, "No reference designator", "You have not entered a reference designator.  Please enter a reference designator and try again.", QMessageBox::Ok, this);
		mb.exec();
	}
	else if (mCurrFpUuid.isNull())
	{
		QMessageBox mb(QMessageBox::Warning, "No footprint selected", "You have not selected a footprint.  Please select a footprint and try again.", QMessageBox::Ok, this);
		mb.exec();
	}

	else
		QDialog::accept();
}
