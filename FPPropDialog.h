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

#ifndef FPPROPDIALOG_H
#define FPPROPDIALOG_H

#include <QDialog>
#include "ui_FPPropDialog.h"

class Footprint;

class FPPropDialog : public QDialog, private Ui::FPPropDialog
{
	Q_OBJECT
public:
	explicit FPPropDialog(QWidget* parent);

	void init(Footprint* fp);
	void updateFp(Footprint* fp);

protected:
	void accept();
};

#endif // FPPROPDIALOG_H
