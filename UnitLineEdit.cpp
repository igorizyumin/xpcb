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

#include "UnitLineEdit.h"
#include <QDebug>


#define ULE_DEBUG
#ifdef ULE_DEBUG
#define ULEDBG() qDebug()
#else
#define ULEDBG() if (false) qDebug()
#endif

UnitLineEdit::UnitLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
	this->setText(mValidator.textFromValue(0));
	mValidator.setPreviousText(text());
	this->setValidator(&mValidator);
	connect(this, SIGNAL(editingFinished()),
			this, SLOT(onEditFinished()));
}

void UnitLineEdit::onEditFinished()
{
	mValidator.setPreviousText(text());
	emit valueChanged(text());
	emit valueChanged(value());
}

///////////////////////////////////////////////////////////////////////////////

UnitComboBox::UnitComboBox(QWidget *parent)
	: QComboBox(parent)
{
	setLineEdit(new UnitLineEdit(this));
}


///////////////////////////////////////////////////////////////////////////////

void UnitValidator::fixup(QString &input) const
{
	ULEDBG() << "fixup";
	QValidator::State s;
	Dimension d = validateAndInterpret(input, s);
	if (s == QValidator::Intermediate)
	{
		// check if the problem is max/min and clamp the value
		Dimension max(mMax);
		Dimension min(mMin);
		max.setUnits(d.units());
		min.setUnits(d.units());
		if (d.toPcb() > mMax)
		{
			input = textFromValue(max);
			ULEDBG() << "clamping to max: " << max.toPcb() << input;
		}
		else if (d.toPcb() < mMin)
		{
			input = textFromValue(min);
			ULEDBG() << "clamping to min: " << min.toPcb() << input;
		}
		else
		{
			input = mPrevTxt;
			ULEDBG() << "reset to prev from interm";
		}
	}
	else
	{
		input = mPrevTxt;
		ULEDBG() << "reset to prev from invalid";
	}
}

QValidator::State UnitValidator::validate(QString &input, int &) const
{
	QValidator::State state;
	validateAndInterpret(input, state);
	return state;
}

Dimension UnitValidator::validateAndInterpret(const QString &input,
										   QValidator::State &state) const
{
	int pos;

	// first, separate the number and unit parts
	QRegExp split("^((?:\\d|\\+|-|\\.|,)*)([a-zA-Z]*)$");
	if (!split.exactMatch(input))
	{
		state = QValidator::Invalid;
		return Dimension();
	}

	QString num = split.cap(1);
	QString unit = split.cap(2);

	QRegExp re_num("^((?:\\+|\\-)?(?:(?:(?:\\d(?:\\d|,\\d)*)(?:\\.(?:(?:\\d)+)?)?)|(?:\\.(?:\\d)+)))$");
	QRegExpValidator vn(re_num, 0);
	QValidator::State numstate = vn.validate(num, pos);

	QRegExp re_unit("^(mm|mil)$");
	QRegExpValidator vu(re_unit, 0);
	QValidator::State unitstate = vu.validate(unit, pos);

	if (numstate == QValidator::Invalid
			|| unitstate == QValidator::Invalid)
	{
		state = QValidator::Invalid;
		return Dimension();
	}
	if (numstate == QValidator::Intermediate
			|| unitstate == QValidator::Intermediate)
	{
		state = QValidator::Intermediate;
		return Dimension();
	}

	// both number and unit are acceptable if we got here
	state = QValidator::Acceptable;
	Q_ASSERT(unit == "mm" || unit == "mil");
	Dimension out;
	// strip separators
	num.remove(',');
	bool ok = false;
	if (unit == "mm")
		out = Dimension(num.toDouble(&ok), Dimension::mm);
	else
		out = Dimension(num.toDouble(&ok), Dimension::mils);
	Q_ASSERT(ok);

	if (out.toPcb() > mMax)
	{
		state = QValidator::Intermediate;
	}
	else if (out.toPcb() < mMin)
	{
		state = QValidator::Intermediate;
	}
	return out;
}

Dimension UnitValidator::valueFromText(const QString &text) const
{
	QValidator::State state = QValidator::Acceptable;
	return validateAndInterpret(text, state);
}

QString UnitValidator::textFromValue(Dimension value) const
{
	Dimension::Unit unit = value.units();
	QString str;
	if (unit == Dimension::pcbu)
		unit = Dimension::mm;
	if (unit == Dimension::mils)
	{
		str = locale().toString(value.toMils(), 'f', 1);
		str.append("mil");
	}
	else
	{
		str = locale().toString(value.toMm(), 'f', 3);
		str.append("mm");
	}
	return str;
}
