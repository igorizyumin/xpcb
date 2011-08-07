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

#ifndef NETLISTDIALOG_H
#define NETLISTDIALOG_H

#include "ui_NetlistDialog.h"
#include "Net.h"

class NLNet;

class NetlistDialog : public QDialog, private Ui::NetlistDialog
{
    Q_OBJECT

public:
	explicit NetlistDialog(QWidget *parent, QSharedPointer<Netlist> netlist);

protected:
	virtual void closeEvent(QCloseEvent *);
	virtual void hideEvent(QHideEvent *);

private:
	void populateLists();
	void addItemsForNet(const NLNet& net);


	void saveGeom();
	void loadGeom();

	QSharedPointer<Netlist> mNetlist;
	QIcon mYesIcon;
	QIcon mNoIcon;
	QIcon mErrIcon;
	QIcon mWarnIcon;
};

#endif // NETLISTDIALOG_H
