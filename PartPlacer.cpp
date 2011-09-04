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

#include <QSettings>
#include "PartPlacer.h"
#include "Controller.h"
#include "Document.h"

PartPlacer::PartPlacer(QWidget *parent, PCBController* ctrl) :
	QDockWidget(parent),
	mCtrl(ctrl),
	mYesIcon(":/Resources/icon-yes.png"),
	mNoIcon(":/Resources/icon-no.png"),
	mErrIcon(":/Resources/icon-error.png"),
	mWarnIcon(":/Resources/icon-warning.png")
{
    setupUi(this);
	populateItems();
	loadGeom();
}

void PartPlacer::closeEvent(QCloseEvent *e)
{
	saveGeom();
	QDockWidget::closeEvent(e);
}

void PartPlacer::hideEvent(QHideEvent *e)
{
	saveGeom();
	QDockWidget::hideEvent(e);
}

void PartPlacer::saveGeom()
{
	QSettings s;
	s.setValue("partPlacer/geometry", QVariant(saveGeometry()));
	s.setValue("partPlacer/headerState",
			   QVariant(partsTree->header()->saveState()));
}

void PartPlacer::loadGeom()
{
	QSettings s;
	restoreGeometry(s.value("partPlacer/geometry").toByteArray());
	partsTree->header()->restoreState(
				s.value("partPlacer/headerState").toByteArray());
}

void PartPlacer::populateItems()
{
	partsTree->clear();
	PCBDoc* doc = mCtrl->pcbDoc();
	if (!doc) return;
	foreach(const NLPart& part, doc->netlist()->parts())
	{
		QTreeWidgetItem *partItem = new QTreeWidgetItem(this->partsTree);

		partItem->setText(1, part.refdes());
		partItem->setText(2, part.footprint());
		switch(part.checkPart(doc))
		{
		case NLPart::OK:
			partItem->setIcon(0, mYesIcon);
			partItem->setFlags(Qt::ItemIsEnabled);
			break;
		case NLPart::Missing:
			partItem->setIcon(0, mNoIcon);
			partItem->setFlags(Qt::ItemIsEnabled |
							  Qt::ItemIsSelectable);
			break;
		case NLPart::Mismatch:
			partItem->setIcon(0, mWarnIcon);
			partItem->setFlags(Qt::ItemIsEnabled);
			break;
		default:
			partItem->setIcon(0, mErrIcon);
			partItem->setFlags(0);
			break;
		}
	}
}

void PartPlacer::updateList()
{
	populateItems();
}

void PartPlacer::on_placeBtn_clicked()
{
	PCBDoc* doc = mCtrl->pcbDoc();
	if (!doc) return;
	QList<NLPart> parts;
	foreach(QTreeWidgetItem* item, partsTree->selectedItems())
	{
		parts.append(doc->netlist()->part(item->data(1, Qt::DisplayRole).toString()));
	}
	if (parts.isEmpty()) return;
	mCtrl->placeParts(parts);
}
