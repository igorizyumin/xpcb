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

#include "tst_UnitSpinboxTest.h"
#include "UnitLineEdit.h"

Q_DECLARE_METATYPE(QValidator::State);

UnitLineEditTest::UnitLineEditTest() :
	QObject()
{
}

void UnitLineEditTest::testValidator_data()
{
	QTest::addColumn<QString>("text");
	QTest::addColumn<int>("pos");
	QTest::addColumn<QValidator::State>("state");

	QTest::newRow("1") << "" << 0 << QValidator::Intermediate;
	QTest::newRow("2") << "1" << 0 << QValidator::Intermediate;
	QTest::newRow("3") << "1," << 0 << QValidator::Intermediate;
	QTest::newRow("4") << "1,000" << 0 << QValidator::Intermediate;
	QTest::newRow("5") << "1,000.015" << 0 << QValidator::Intermediate;
	QTest::newRow("6") << "1,000.015mm" << 0 << QValidator::Acceptable;
	QTest::newRow("7") << "1,000.015mil" << 0 << QValidator::Acceptable;
	QTest::newRow("8") << "+1,000.015mil" << 0 << QValidator::Acceptable;
	QTest::newRow("9") << "-1,000.015mil" << 0 << QValidator::Acceptable;
	QTest::newRow("10") << "1,00-0.015mil" << 0 << QValidator::Invalid;
	QTest::newRow("11") << "1,00+0.015mil" << 0 << QValidator::Invalid;
	QTest::newRow("12") << "1,000.0,15mil" << 0 << QValidator::Invalid;
	QTest::newRow("13") << "mil" << 0 << QValidator::Intermediate;
	QTest::newRow("14") << "-mil" << 0 << QValidator::Intermediate;
	QTest::newRow("15") << "-.mil" << 0 << QValidator::Intermediate;
}

void UnitLineEditTest::testValidator()
{
	QFETCH(QString, text);
	QFETCH(int, pos);
	QFETCH(QValidator::State, state);

	UnitValidator v;
	v.setMinimum(-1000000000);
	v.setMaximum(1000000000);

	QCOMPARE(v.validate(text, pos), state);
}
