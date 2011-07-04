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

#ifndef EDITPINDIALOG_H
#define EDITPINDIALOG_H

#include "ui_EditPinDialog.h"
#include "Shape.h"

class NewPinCmd;
class PinEditCmd;
class Document;
class Padstack;

class EditPinDialog : public QDialog, private Ui::EditPinDialog
{
    Q_OBJECT

public:
	explicit EditPinDialog(QWidget *parent, Document* doc);

	void init(QSharedPointer<Pin> p = QSharedPointer<Pin>());

	QList<QSharedPointer<Pin> > makePins(QSharedPointer<Footprint> fp);
	bool dragToPos() const { return dragRadio->isChecked(); }
	QPoint pos() const { return QPoint(toPcb(xPosBox->value()), toPcb(yPosBox->value())); }
	int angle() const { return angleBox->currentIndex() * 90; }
	QString name() const { return pinName->text().trimmed(); }
	QSharedPointer<Padstack> padstack() const;
private slots:
	void on_unitsBox_currentIndexChanged(const QString &s);
	void on_manageBtn_clicked();

protected:
	void accept();

private:
	void updatePsList();
	void updateUnits();
	double toUnits(int pcbu) const;
	int toPcb(double unit) const;

	bool mInMM;
	QSharedPointer<Pin> mPin;
	Document* mDoc;
};

#endif // EDITPINDIALOG_H
