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

#ifndef EDITLINEDIALOG_H
#define EDITLINEDIALOG_H

#include <QSharedPointer>
#include "global.h"
#include "Line.h"
#include "ui_EditLineDialog.h"

class EditLineDialog : public QDialog, private Ui::EditLineDialog
{
    Q_OBJECT

public:
    explicit EditLineDialog(QWidget *parent = 0);

	void init(QSharedPointer<Line> line);
	int width() const { return toPcb(widthBox->value()); }
	Layer layer() const;
private slots:
	void on_unitsBox_currentIndexChanged(const QString& s);

private:
	double toUnits(int pcbu) const;
	int toPcb(double unit) const;
	void updateUnits();

	bool mInMM;
};

#endif // EDITLINEDIALOG_H
