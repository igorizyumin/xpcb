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

#ifndef CTRLACTION_H
#define CTRLACTION_H

#include <QObject>
#include <QString>

/// Action triggered by a function key
class CtrlAction : public QObject
{
	Q_OBJECT

public:
	CtrlAction(int key, QString text) : mKey(key), mText(text) {}

	int key() const { return mKey; }
	QString text() const { return mText; }
	void setText(QString text) { mText = text; }

public slots:
	void exec() { emit execFired(); }

signals:
	void execFired();

private:
	int mKey;
	QString mText;
};

#endif // CTRLACTION_H
