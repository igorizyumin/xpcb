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

#include "tst_TextTest.h"
#include "Text.h"

TextTest::TextTest()
{

}

void TextTest::testConstruct_data()
{
	QTest::addColumn<QPoint>("pos");
	QTest::addColumn<int>("angle");
	QTest::addColumn<bool>("mirrored");
	QTest::addColumn<bool>("negative");
	QTest::addColumn<Layer>("layer");
	QTest::addColumn<int>("fontsize");
	QTest::addColumn<int>("strokewidth");
	QTest::addColumn<QString>("text");

	QTest::newRow("text1") << QPoint(100, 200) << 0 << true << true << Layer(Layer::LAY_TOP_COPPER) << 200 << 1004 << "test 2342 5234 asdf";
	QTest::newRow("text2") << QPoint(100, 200) << 90 << true << false << Layer(Layer::LAY_SILK_TOP) << 45 << 55 << " 5234 asdf";
	QTest::newRow("text3") << QPoint(100, 0) << 180 << false << true << Layer(Layer::LAY_SILK_BOTTOM) << 6 << 1004 << "";
	QTest::newRow("text4") << QPoint(0, 200) << 270 << false << false << Layer(Layer::LAY_INNER6) << 0 << 1004 << "test 234";
}

void TextTest::testConstruct()
{
	QFETCH(QPoint, pos);
	QFETCH(int, angle);
	QFETCH(bool, mirrored);
	QFETCH(bool, negative);
	QFETCH(Layer, layer);
	QFETCH(int, fontsize);
	QFETCH(int, strokewidth);
	QFETCH(QString, text);

	Text t(pos, angle, mirrored, negative, layer, fontsize, strokewidth, text);
	QCOMPARE(t.pos(), pos);
	QCOMPARE(t.angle(), angle);
	QCOMPARE(t.isMirrored(), mirrored);
	QCOMPARE(t.isNegative(), negative);
	QCOMPARE(t.layer(), layer);
	QCOMPARE(t.fontSize(), fontsize);
	QCOMPARE(t.strokeWidth(), strokewidth);
	QCOMPARE(t.text(), text);
}


class TestDummy : public PCBObject
{
public:
	virtual void accept(PCBObjectVisitor *v) {}
	virtual void draw(QPainter *painter, const Layer& layer) const {}
	virtual QRect bbox() const { return QRect(); }
	virtual QTransform transform() const { return mTransform; }

	virtual PCBObjState getState() const { return PCBObjState(); }
	virtual bool loadState(PCBObjState &) { return false; }

	QTransform mTransform;
};

void TextTest::testParent()
{
	TestDummy d;
	d.mTransform.translate(100, 150);
	Text t(QPoint(10, 100), 90, false, false, Layer::LAY_TOP_COPPER, 100, 10, ">");
	QCOMPARE(t.pos(), QPoint(10, 100));
	QRect bb = t.bbox();
	t.setParent(&d);
	QCOMPARE(t.pos(), QPoint(110, 250));
	QCOMPARE(t.bbox(), d.mTransform.mapRect(bb));
	d.mTransform.translate(10, 50);
	t.parentChanged();
	QCOMPARE(t.pos(), QPoint(120, 300));
	QCOMPARE(t.bbox(), d.mTransform.mapRect(bb));
	t.setPos(QPoint(100, 10));
	QCOMPARE(t.pos(), QPoint(100, 10));
	QCOMPARE(t.bbox(), d.mTransform.mapRect(bb).translated(QPoint(-20, -290)));
	// total transform at this point: +110, +200
	t.setParent(NULL);
	QCOMPARE(t.pos(), QPoint(-10, -190));
	QCOMPARE(t.bbox(), bb.translated(QPoint(-20, -290)));
}
