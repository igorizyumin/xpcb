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

#ifndef UNITLINEEDIT_H
#define UNITLINEEDIT_H

#include <QLineEdit>
#include <QComboBox>
#include <QValidator>
#include "global.h"

class UnitValidator : public QValidator
{
public:
	UnitValidator(QObject* parent = 0)
		: QValidator(parent), mMin(0), mMax(XPcb::mmToPcb(100)) {}
	UnitValidator(int min, int max, QObject* parent = 0)
		: QValidator(parent), mMin(min), mMax(max) {}

	virtual QValidator::State validate(QString &, int &) const;
	virtual void fixup(QString &) const;

	Dimension valueFromText(const QString& text) const;
	QString textFromValue(Dimension value) const;

	int maximum() const { return mMax; }
	int minimum() const { return mMin; }
	void setMinimum(int min) { mMin = min; }
	void setMaximum(int max) { mMax = max; }

	// this is a hack to use fixup() to enforce validity
	void setPreviousText(QString prev) { mPrevTxt = prev; }

private:
	Dimension validateAndInterpret(const QString &input,
								  QValidator::State &state) const;

	int mMin;
	int mMax;
	QString mPrevTxt;
};

// A spinbox would be more logical to subclass for this.  Unfortunately,
// the spinbox classes in Qt are very badly designed. Some important
// functionality is hidden in the pimpl class, which makes it
// impossible to subclass QAbstractSpinBox in a useful way without
// rewriting almost all of the code.
class UnitLineEdit : public QLineEdit
{
    Q_OBJECT
	Q_PROPERTY(int minimum READ minimum WRITE setMinimum USER true)
	Q_PROPERTY(int maximum READ maximum WRITE setMaximum USER true)
	Q_PROPERTY(Dimension value READ value WRITE setValue NOTIFY valueChanged USER true	)

public:
	explicit UnitLineEdit(QWidget *parent = 0);

	int maximum() const { return mValidator.maximum(); }
	int minimum() const { return mValidator.minimum(); }
	void setMinimum(int min) { mValidator.setMinimum(min); }
	void setMaximum(int max) { mValidator.setMaximum(max); }

	Dimension value() const { return mValidator.valueFromText(text()); }
signals:
	void valueChanged(Dimension val);
	void valueChanged(const QString &val);

public slots:
	void setValue(Dimension val) { setText(mValidator.textFromValue(val)); }

private slots:
	void onEditFinished();

private:
	UnitValidator mValidator;
};

class UnitComboBox : public QComboBox
{
	Q_OBJECT
public:
	explicit UnitComboBox(QWidget *parent = 0);

	int maximum() const { return mValidator.maximum(); }
	int minimum() const { return mValidator.minimum(); }
	void setMinimum(int min) { mValidator.setMinimum(min); }
	void setMaximum(int max) { mValidator.setMaximum(max); }

	Dimension value() const { return mValidator.valueFromText(currentText()); }
signals:
	void valueChanged(Dimension val);
	void valueChanged(const QString &val);

public slots:
//	void setValue(Dimension val) { setText(mValidator.textFromValue(val)); }

private slots:
//	void onEditFinished();

private:
	UnitValidator mValidator;
};

#endif // UNITLINEEDIT_H
