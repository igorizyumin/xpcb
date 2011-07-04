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

#ifndef MANAGEPADSTACKSDIALOG_H
#define MANAGEPADSTACKSDIALOG_H

#include "ui_ManagePadstacksDialog.h"
#include "PCBDoc.h"
#include <QUndoCommand>


class ManagePadstacksDialog : public QDialog, private Ui::ManagePadstacksDialog
{
    Q_OBJECT

public:
	explicit ManagePadstacksDialog(QWidget *parent, Document* doc);

private slots:
	void on_newButton_clicked();
	void on_editButton_clicked();
//	void on_copyButton_clicked();
//	void on_deleteButton_clicked();
	void on_psList_itemSelectionChanged();

private:
	void updateItems();
	QSharedPointer<Padstack> currItem();

	Document* mDoc;
};

class NewPadstackCmd : public QUndoCommand
{
public:
	NewPadstackCmd(QUndoCommand *parent, Document* doc, Padstack ps);

	virtual void undo();
	virtual void redo();
private:
	QSharedPointer<Padstack> mPs;
	Document* mDoc;
};

class EditPadstackCmd : public QUndoCommand
{
public:
	EditPadstackCmd(QUndoCommand *parent, QSharedPointer<Padstack> ps, Padstack newPs);

	virtual void undo();
	virtual void redo();
private:
	Padstack mPrev;
	Padstack mNew;
	QSharedPointer<Padstack> mPs;
};

#endif // MANAGEPADSTACKSDIALOG_H
