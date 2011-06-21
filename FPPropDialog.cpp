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

#include <QMessageBox>
#include "FPPropDialog.h"
#include "Shape.h"

FPPropDialog::FPPropDialog(QWidget* parent)
	:	QDialog(parent)
{
	setupUi(this);
}

void FPPropDialog::init(Footprint *fp)
{
	this->nameEdit->setText(fp->name());
	this->authorEdit->setText(fp->author());
	this->sourceEdit->setText(fp->source());
	this->descEdit->setText(fp->desc());
}

void FPPropDialog::updateFp(Footprint *fp)
{
	fp->setName(this->nameEdit->text());
	fp->setAuthor(this->authorEdit->text());
	fp->setSource(this->sourceEdit->text());
	fp->setDesc(this->descEdit->text());
}

void FPPropDialog::accept()
{
	if (this->nameEdit->text().isEmpty())
	{
		QMessageBox mb(QMessageBox::Warning, "No name entered", "You have not entered a name for the footprint.  Please enter a name and try again.", QMessageBox::Ok, this);
		mb.exec();
		return;
	}
	QDialog::accept();
}
