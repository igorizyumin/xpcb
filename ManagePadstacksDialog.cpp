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

#include "ManagePadstacksDialog.h"
#include "EditPadstackDialog.h"

Q_DECLARE_METATYPE(QSharedPointer<Padstack>);

ManagePadstacksDialog::ManagePadstacksDialog(QWidget *parent, Document *doc)
	: QDialog(parent), mDoc(doc)
{
    setupUi(this);
	updateItems();
}

void ManagePadstacksDialog::on_newButton_clicked()
{
	EditPadstackDialog dialog(this);
	if (dialog.exec() == QDialog::Rejected)
		return;
	NewPadstackCmd *cmd = new NewPadstackCmd(NULL, mDoc, dialog.toPadstack());
	mDoc->doCommand(cmd);
	updateItems();
}


void ManagePadstacksDialog::on_editButton_clicked()
{
	QSharedPointer<Padstack> curr = currItem();
	if (!curr)
		return;
	EditPadstackDialog dialog(this);
	dialog.init(curr);
	if (dialog.exec() == QDialog::Rejected)
		return;
	EditPadstackCmd *cmd = new EditPadstackCmd(NULL, curr, dialog.toPadstack());
	mDoc->doCommand(cmd);
	updateItems();
}

void ManagePadstacksDialog::updateItems()
{
	psList->clear();
	QList<QSharedPointer<Padstack> > l = mDoc->padstacks();
	foreach(QSharedPointer<Padstack> ps, l)
	{
		QListWidgetItem *item = new QListWidgetItem(ps->name(), this->psList);
		item->setData(Qt::UserRole, QVariant::fromValue(ps));
		this->psList->addItem(item);
	}
	on_psList_itemSelectionChanged();
}

QSharedPointer<Padstack> ManagePadstacksDialog::currItem()
{
	if (psList->currentItem() == NULL)
		return QSharedPointer<Padstack>();
	return psList->currentItem()->data(Qt::UserRole).value<QSharedPointer<Padstack> >();
}

void ManagePadstacksDialog::on_psList_itemSelectionChanged()
{
	if (psList->currentItem())
	{
		editButton->setEnabled(true);
		copyButton->setEnabled(true);
		deleteButton->setEnabled(true);
	}
	else
	{
		editButton->setEnabled(false);
		copyButton->setEnabled(false);
		deleteButton->setEnabled(false);
	}
}

///////////////////////// UNDO / REDO ///////////////////////////////////

NewPadstackCmd::NewPadstackCmd(QUndoCommand *parent, Document *doc, Padstack ps)
	: QUndoCommand(parent), mPs(QSharedPointer<Padstack>(new Padstack(ps))), mDoc(doc)
{
}

void NewPadstackCmd::redo()
{
	mDoc->addPadstack(mPs);
}

void NewPadstackCmd::undo()
{
	mDoc->removePadstack(mPs);
}

EditPadstackCmd::EditPadstackCmd(QUndoCommand *parent, QSharedPointer<Padstack> ps,
								 Padstack newPs)
	: QUndoCommand(parent), mPrev(*ps), mNew(newPs), mPs(ps)
{
}

void EditPadstackCmd::redo()
{
	*mPs = mNew;
}

void EditPadstackCmd::undo()
{
	*mPs = mPrev;
}
