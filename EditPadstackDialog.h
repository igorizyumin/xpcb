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

#ifndef EDITPADSTACKDIALOG_H
#define EDITPADSTACKDIALOG_H

#include "ui_EditPadstackDialog.h"
#include "Footprint.h"

class NewPadstackCmd;
class EditPadstackCmd;
class Document;

class EditPadstackDialog : public QDialog, private Ui::EditPadstackDialog
{
    Q_OBJECT

public:
    explicit EditPadstackDialog(QWidget *parent = 0);

	void init(QSharedPointer<Padstack> ps);
	Padstack toPadstack();

public slots:
	virtual void accept();

private slots:
	void on_unitsBox_currentIndexChanged(const QString &s);
	void on_topCuShape_currentIndexChanged(int i);
	void on_topSmShape_currentIndexChanged(int i);
	void on_topPmShape_currentIndexChanged(int i);
	void on_botCuShape_currentIndexChanged(int i);
	void on_botSmShape_currentIndexChanged(int i);
	void on_botPmShape_currentIndexChanged(int i);
	void on_innerShape_currentIndexChanged(int i);

private:
	void updateUnits();
	double toUnits(int pcbu);
	int toPcb(double unit);
	void errorMsg(QString err);

	bool mInMM;

};



#endif // EDITPADSTACKDIALOG_H
