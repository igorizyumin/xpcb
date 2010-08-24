#include <QtCore/QString>
#include <QtTest/QtTest>
#include "tst_XmlLoadTest.h"
#include "Line.h"
#include "Text.h"
#include "Net.h"
#include "Shape.h"
#include "PCBDoc.h"
#include "Part.h"
#include "Trace.h"
#include "Polygon.h"

XmlLoadTest::XmlLoadTest()
{
}

void XmlLoadTest::testLine()
{
	QXmlStreamReader reader("<line width='600' layer='2' x1='100' y1='200' x2='1000' y2='2000' />");
	reader.readNextStartElement();
	Line l = Line::newFromXml(reader);
	QCOMPARE(l.width(), 600);
	QCOMPARE(l.layer(), (PCBLAYER)2);
	QCOMPARE(l.start(), QPoint(100, 200));
	QCOMPARE(l.end(), QPoint(1000, 2000));
}

void XmlLoadTest::testArc()
{
	QXmlStreamReader reader("<arc width='600' layer='2' x1='100' y1='200' x2='1000' y2='2000' ctrX='400' ctrY='500' dir='cw'/>");
	reader.readNextStartElement();
	Arc a = Arc::newFromXml(reader);
	QCOMPARE(a.width(), 600);
	QCOMPARE(a.layer(), (PCBLAYER)2);
	QCOMPARE(a.start(), QPoint(100, 200));
	QCOMPARE(a.end(), QPoint(1000, 2000));
	QCOMPARE(a.ctr(), QPoint(400, 500));
	QCOMPARE(a.isCw(), true);
}

void XmlLoadTest::testText()
{
	QXmlStreamReader reader("<text layer='2' x='100' y='200' rot='180' lineWidth='300' textSize='400'> testing 123  </text>");
	reader.readNextStartElement();

	Text t = Text::newFromXML(reader);

	QCOMPARE(t.layer(), (PCBLAYER)2);
	QCOMPARE(t.angle(), 180);
	QCOMPARE(t.pos(), QPoint(100,200));
	QCOMPARE(t.strokeWidth(), 300);
	QCOMPARE(t.fontSize(), 400);
	QCOMPARE(t.text(), QString(" testing 123  "));
}

#if 0
void XmlLoadTest::testNet()
{
	QXmlStreamReader reader("<net name='testNet' visible='1' defViaPadstack='201'><pinRef partref='R1' pinname='1'/></net>");
	reader.readNextStartElement();
	QHash<int, Padstack*> padstacks;
	Padstack* ps = new Padstack();
	padstacks.insert(201, ps);
	PCBDoc* doc = new PCBDoc();
	doc->mParts.insert(new Part(doc))


	delete ps;

}
#endif

void XmlLoadTest::testPadstack()
{
	QXmlStreamReader reader(
			"<padstack name='pstest' id='100' holesize='1000'><!-- fark -->"
			"<startpad><pad shape='square' width='10000'/></startpad>"
			"<innerpad><pad shape='round' width='15000'/><!-- asdf --></innerpad>"
			"<!-- comment -->"
			"<endpad><!-- another comment --><pad shape='rect' width='20000' height='5000'/><!--more comments --></endpad>"
			"<startmask/><endmask/><startpaste/><endpaste/>"
			"</padstack>");
	reader.readNextStartElement();
	Padstack* ps = Padstack::newFromXML(reader);
	QVERIFY(ps != NULL);
	QCOMPARE(ps->getName(), QString("pstest"));
	QCOMPARE(ps->getHole(), 1000);
	QCOMPARE(ps->getStartPad().isNull(), false);
	QCOMPARE(ps->getStartPad().shape(), Pad::PAD_SQUARE);
	QCOMPARE(ps->getStartPad().width(), 10000);
	QCOMPARE(ps->getInnerPad().isNull(), false);
	QCOMPARE(ps->getInnerPad().shape(), Pad::PAD_ROUND);
	QCOMPARE(ps->getInnerPad().width(), 15000);
	QCOMPARE(ps->getEndPad().isNull(), false);
	QCOMPARE(ps->getEndPad().shape(), Pad::PAD_RECT);
	QCOMPARE(ps->getEndPad().width(), 20000);
	QCOMPARE(ps->getEndPad().height(), 5000);
	QCOMPARE(ps->getStartMask().isNull(), true);
	QCOMPARE(ps->getEndMask().isNull(), true);
	QCOMPARE(ps->getStartPaste().isNull(), true);
	QCOMPARE(ps->getEndPaste().isNull(), true);
	delete ps;
}

void XmlLoadTest::testTraceList()
{
	QXmlStreamReader reader("<traces><!-- asdf --><vertices>"
							"<vertex id='101' x='100' y='200'/>"
							"<vertex id='102' x='300' y='400'/>"
							"<!-- asdf -->"
							"<vertex id='103' x='500' y='600'/>"
							"<vertex id='104' x='700' y='800'/>"
							"</vertices><!-- asdf --><segments>"
							"<segment start='101' end='102' layer='8' width='1000'/>"
							"<!-- test -->"
							"<segment start='103' end='104' layer='8' width='2000'/>"
							"<segment start='102' end='103' layer='8' width='3000'/>"
							"</segments><!-- asdf --></traces>");
	reader.readNextStartElement();
	TraceList tl;
	tl.loadFromXml(reader);
	QSet<Vertex*> vtx = tl.vertices();
	QSet<Segment*> seg = tl.segments();
	QCOMPARE(vtx.size(), 4);
	QCOMPARE(seg.size(), 3);
	foreach(Vertex* v, vtx)
	{
		if (v->pos() == QPoint(100, 200))
		{
			// vertex 101
			QCOMPARE(v->segments().size(), 1);
			QCOMPARE(v->segments().toList()[0]->width(), 1000);
			QCOMPARE(v->segments().toList()[0]->layer(), (PCBLAYER)8);
			QCOMPARE(v->segments().toList()[0]->otherVertex(v)->pos(), QPoint(300,400));
		}
		else if (v->pos() == QPoint(300, 400))
		{
			// vertex 102
			QCOMPARE(v->segments().size(), 2);
			QList<Segment*> l = v->segments().toList();
			int first = 1;
			int second = 0;
			if (l[0]->width() == 1000)
			{
				first = 0;
				second = 1;
			}
			QCOMPARE(v->segments().toList()[first]->width(), 1000);
			QCOMPARE(v->segments().toList()[first]->layer(), (PCBLAYER)8);
			QCOMPARE(v->segments().toList()[first]->otherVertex(v)->pos(), QPoint(100,200));
			QCOMPARE(v->segments().toList()[second]->width(), 3000);
			QCOMPARE(v->segments().toList()[second]->layer(), (PCBLAYER)8);
			QCOMPARE(v->segments().toList()[second]->otherVertex(v)->pos(), QPoint(500,600));
		}
		else if (v->pos() == QPoint(500, 600))
		{
			// vertex 103
			QCOMPARE(v->segments().size(), 2);
			QList<Segment*> l = v->segments().toList();
			int first = 1;
			int second = 0;
			if (l[0]->width() == 2000)
			{
				first = 0;
				second = 1;
			}
			QCOMPARE(v->segments().toList()[first]->width(), 2000);
			QCOMPARE(v->segments().toList()[first]->layer(), (PCBLAYER)8);
			QCOMPARE(v->segments().toList()[first]->otherVertex(v)->pos(), QPoint(700,800));
			QCOMPARE(v->segments().toList()[second]->width(), 3000);
			QCOMPARE(v->segments().toList()[second]->layer(), (PCBLAYER)8);
			QCOMPARE(v->segments().toList()[second]->otherVertex(v)->pos(), QPoint(300,400));
		}
		else if (v->pos() == QPoint(700, 800))
		{
			// vertex 104
			QCOMPARE(v->segments().size(), 1);
			QCOMPARE(v->segments().toList()[0]->width(), 2000);
			QCOMPARE(v->segments().toList()[0]->layer(), (PCBLAYER)8);
			QCOMPARE(v->segments().toList()[0]->otherVertex(v)->pos(), QPoint(500,600));
		}
		else
			QVERIFY(false);
	}
}

void XmlLoadTest::testPolygon()
{
	QXmlStreamReader reader("<polygon>"
							"<outline>"
							"<start x='0' y='0'/>"
							"<lineTo x='0' y='1000'/>"
							"<arcTo x='1000' y='0' dir='cw' ctrX='0' ctrY='0'/>"
							"<lineTo x='0' y='0'/>"
							"</outline>"
							"<!-- more random comments -->"
							"<hole>"
							"<start x='100' y='100'/>"
							"<arcTo x='200' y='200' ctrX='200' ctrY='100' dir='cw'/>"
							"<lineTo x='200' y='100'/>"
							"<lineTo x='100' y='100'/>"
							"<!-- random comment -->"
							"</hole>"
							"</polygon>"
							);
	reader.readNextStartElement();
	Polygon *p = Polygon::newFromXML(reader);
	QVERIFY(p != NULL);
	QCOMPARE(p->numHoles(), 1);
	QCOMPARE(p->outline()->numSegs(), 4);
	QCOMPARE(p->outline()->segment(0).type, PolyContour::Segment::START);
	QCOMPARE(p->outline()->segment(0).end, QPoint(0,0));
	QCOMPARE(p->outline()->segment(1).type, PolyContour::Segment::LINE);
	QCOMPARE(p->outline()->segment(1).end, QPoint(0,1000));
	QCOMPARE(p->outline()->segment(2).type, PolyContour::Segment::ARC_CW);
	QCOMPARE(p->outline()->segment(2).end, QPoint(1000,0));
	QCOMPARE(p->outline()->segment(2).arcCenter, QPoint(0, 0));
	QCOMPARE(p->outline()->segment(3).type, PolyContour::Segment::LINE);
	QCOMPARE(p->outline()->segment(3).end, QPoint(0,0));
	QCOMPARE(p->hole(0)->numSegs(), 4);
	QCOMPARE(p->hole(0)->segment(0).type, PolyContour::Segment::START);
	QCOMPARE(p->hole(0)->segment(0).end, QPoint(100,100));
	QCOMPARE(p->hole(0)->segment(1).type, PolyContour::Segment::ARC_CW);
	QCOMPARE(p->hole(0)->segment(1).end, QPoint(200,200));
	QCOMPARE(p->hole(0)->segment(1).arcCenter, QPoint(200,100));
	QCOMPARE(p->hole(0)->segment(2).type, PolyContour::Segment::LINE);
	QCOMPARE(p->hole(0)->segment(2).end, QPoint(200,100));
	QCOMPARE(p->hole(0)->segment(3).type, PolyContour::Segment::LINE);
	QCOMPARE(p->hole(0)->segment(3).end, QPoint(100,100));

	delete p;
}
