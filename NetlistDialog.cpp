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

#include "NetlistDialog.h"
#include "Net.h"
#include <QPixmap>
#include <QPair>
#include <QSettings>
#include <QTreeWidgetItem>
#include <QTableWidgetItem>

NetlistDialog::NetlistDialog(QWidget *parent, QSharedPointer<Netlist> netlist) :
	QDialog(parent),
	mNetlist(netlist),
	mYesIcon(":/Resources/icon-yes.png"),
	mNoIcon(":/Resources/icon-no.png"),
	mErrIcon(":/Resources/icon-error.png"),
	mWarnIcon(":/Resources/icon-warning.png")
{
    setupUi(this);
	populateLists();
	loadGeom();
}

void NetlistDialog::populateLists()
{
	foreach(const NLPart& part, mNetlist->parts())
	{
		QTreeWidgetItem *partItem = new QTreeWidgetItem(this->partsTree);
		partItem->setFlags(Qt::ItemIsEnabled |
						  Qt::ItemIsSelectable);
		partItem->setText(0, part.refdes());
		partItem->setText(1, part.value());
		partItem->setText(2, part.footprint());
		partItem->setIcon(3, mYesIcon);
	}
	foreach(const NLNet& net, mNetlist->nets())
	{
		addItemsForNet(net);
	}
}

void NetlistDialog::addItemsForNet(const NLNet &net)
{
	// add tree widget item for net
	QTreeWidgetItem *netItem = new QTreeWidgetItem(this->netsTree);
	netItem->setFlags(Qt::ItemIsUserCheckable |
					  Qt::ItemIsEnabled |
					  Qt::ItemIsSelectable);
	netItem->setText(0, net.name());
	netItem->setCheckState(1, Qt::Checked);
	netItem->setIcon(2, mYesIcon);

	// add pins
	foreach(NLPin p, net.pins())
	{
		QTreeWidgetItem *pinItem = new QTreeWidgetItem(netItem);
		pinItem->setFlags(Qt::ItemIsEnabled |
						  Qt::ItemIsSelectable);
		pinItem->setText(0, p.refdes() + "." + p.pinName());
		pinItem->setIcon(2, mYesIcon);
	}
}

void NetlistDialog::closeEvent(QCloseEvent *e)
{
	saveGeom();
	QDialog::closeEvent(e);
}

void NetlistDialog::hideEvent(QHideEvent *e)
{
	saveGeom();
	QDialog::hideEvent(e);
}

void NetlistDialog::saveGeom()
{
	QSettings s;
	s.setValue("netlistdlg/geometry", QVariant(saveGeometry()));
	s.setValue("netlistdlg/netsHeaderState", QVariant(netsTree->
													   header()->saveState()));
	s.setValue("netlistdlg/partsHeaderState", QVariant(partsTree->
													   header()->saveState()));
}

void NetlistDialog::loadGeom()
{
	QSettings s;
	restoreGeometry(s.value("netlistdlg/geometry").toByteArray());
	netsTree->header()->restoreState(s.value("netlistdlg/netsHeaderState").toByteArray());
	partsTree->header()->restoreState(
				s.value("netlistdlg/partsHeaderState").toByteArray());
}
