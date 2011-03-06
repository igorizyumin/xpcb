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
#include "Area.h"

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
	QVERIFY(reader.isEndElement());

}

void XmlLoadTest::testArc()
{
	QXmlStreamReader reader("<arc width='600' layer='2' x1='100' y1='200' x2='1000' y2='2000' dir='cw'/>");
	reader.readNextStartElement();
	Arc a = Arc::newFromXml(reader);
	QCOMPARE(a.width(), 600);
	QCOMPARE(a.layer(), (PCBLAYER)2);
	QCOMPARE(a.start(), QPoint(100, 200));
	QCOMPARE(a.end(), QPoint(1000, 2000));
	QCOMPARE(a.isCw(), true);
	QVERIFY(reader.isEndElement());

}

void XmlLoadTest::testText()
{
	QXmlStreamReader reader("<text layer='2' x='100' y='200' rot='180' lineWidth='300' textSize='400'> testing 123  </text>");
	reader.readNextStartElement();

	Text* t = Text::newFromXML(reader);

	QCOMPARE(t->layer(), (PCBLAYER)2);
	QCOMPARE(t->angle(), 180);
	QCOMPARE(t->pos(), QPoint(100,200));
	QCOMPARE(t->strokeWidth(), 300);
	QCOMPARE(t->fontSize(), 400);
	QCOMPARE(t->text(), QString(" testing 123  "));
	QVERIFY(reader.isEndElement());

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
	QCOMPARE(ps->mName(), QString("pstest"));
	QCOMPARE(ps->holeSize(), 1000);
	QCOMPARE(ps->startPad().isNull(), false);
	QCOMPARE(ps->startPad().shape(), Pad::PAD_SQUARE);
	QCOMPARE(ps->startPad().width(), 10000);
	QCOMPARE(ps->innerPad().isNull(), false);
	QCOMPARE(ps->innerPad().shape(), Pad::PAD_ROUND);
	QCOMPARE(ps->innerPad().width(), 15000);
	QCOMPARE(ps->endPad().isNull(), false);
	QCOMPARE(ps->endPad().shape(), Pad::PAD_RECT);
	QCOMPARE(ps->endPad().width(), 20000);
	QCOMPARE(ps->endPad().length(), 5000);
	QCOMPARE(ps->startMask().isNull(), true);
	QCOMPARE(ps->endMask().isNull(), true);
	QCOMPARE(ps->startPaste().isNull(), true);
	QCOMPARE(ps->endPaste().isNull(), true);
	QVERIFY(reader.isEndElement());
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
							"</segments><!-- asdf --></traces><next/>");
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
	QVERIFY(reader.isEndElement());
}

void XmlLoadTest::testPolygon()
{
	QXmlStreamReader reader("<polygon>"
							"<outline>"
							"<start x='0' y='0'/>"
							"<lineTo x='0' y='1000'/>"
							"<arcTo x='1000' y='0' dir='cw'/>"
							"<lineTo x='0' y='0'/>"
							"</outline>"
							"<!-- more random comments -->"
							"<hole>"
							"<start x='100' y='100'/>"
							"<arcTo x='200' y='200' dir='cw'/>"
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
	QCOMPARE(p->outline()->segment(3).type, PolyContour::Segment::LINE);
	QCOMPARE(p->outline()->segment(3).end, QPoint(0,0));
	QCOMPARE(p->hole(0)->numSegs(), 4);
	QCOMPARE(p->hole(0)->segment(0).type, PolyContour::Segment::START);
	QCOMPARE(p->hole(0)->segment(0).end, QPoint(100,100));
	QCOMPARE(p->hole(0)->segment(1).type, PolyContour::Segment::ARC_CW);
	QCOMPARE(p->hole(0)->segment(1).end, QPoint(200,200));
	QCOMPARE(p->hole(0)->segment(2).type, PolyContour::Segment::LINE);
	QCOMPARE(p->hole(0)->segment(2).end, QPoint(200,100));
	QCOMPARE(p->hole(0)->segment(3).type, PolyContour::Segment::LINE);
	QCOMPARE(p->hole(0)->segment(3).end, QPoint(100,100));
	QVERIFY(reader.isEndElement());


	delete p;
}

void XmlLoadTest::testFootprint()
{
	QXmlStreamReader reader("<footprint>"
							"<name>RES0805</name>"
							"<units>mm</units>"
							"<author>igor</author>"
							"<source>test</source>"
							"<desc/>"
							"<centroid x='10' y='20' custom='0'/>"
							"<line width='600' layer='2' x1='100' y1='200' x2='1000' y2='2000' />"
							"<pins>"
							"<pin name='1' x='100' y='200' rot='90' padstack='101'/>"
							"<pin name='2' x='400' y='600' rot='0' padstack='101'/>"
							"</pins>"
							"<refText x='1000' y='1100' rot='90' lineWidth='99' textSize='200'/>"
							"<valueText x='1200' y='1400' rot='0' lineWidth='102' textSize='220'/>"
							"</footprint>");
	reader.readNextStartElement();

	QHash<int, Padstack*> padstacks;
	padstacks.insert(101, new Padstack());

	Footprint* fp = Footprint::newFromXML(reader, padstacks);
	QCOMPARE(fp->name(), QString("RES0805"));
	QCOMPARE(fp->author(), QString("igor"));
	QCOMPARE(fp->source(), QString("test"));
	QCOMPARE(fp->desc(), QString());
	QCOMPARE(fp->units(), MM);
	QCOMPARE(fp->centroid(), QPoint(10,20));
	QCOMPARE(fp->isCustomCentroid(), false);
	QCOMPARE(fp->numPins(), 2);
	QCOMPARE(fp->getPin(0)->padstack(), padstacks.value(101));
	QCOMPARE(fp->getPin(0)->pos(), QPoint(100,200));
	QCOMPARE(fp->getPin(0)->angle(), 90);
	QCOMPARE(fp->getPin(1)->padstack(), padstacks.value(101));
	QCOMPARE(fp->getPin(1)->pos(), QPoint(400,600));
	QCOMPARE(fp->getPin(1)->angle(), 0);
	QCOMPARE(fp->getRefText().pos(), QPoint(1000,1100));
	QCOMPARE(fp->getRefText().angle(), 90);
	QCOMPARE(fp->getRefText().strokeWidth(), 99);
	QCOMPARE(fp->getRefText().fontSize(), 200);
	QCOMPARE(fp->getValueText().pos(), QPoint(1200,1400));
	QCOMPARE(fp->getValueText().angle(), 0);
	QCOMPARE(fp->getValueText().strokeWidth(), 102);
	QCOMPARE(fp->getValueText().fontSize(), 220);
	QVERIFY(reader.isEndElement());
	delete fp;
}

void XmlLoadTest::testPart()
{
	// create test objects
	PCBDoc doc;
	Padstack* ps = new Padstack();
	doc.mPadstacks.append(ps);
	QHash<int, Padstack*> padstacks;
	padstacks.insert(101, ps);
	QXmlStreamReader fpxml("<footprint>"
							"<name>RES0805</name>"
							"<units>mm</units>"
							"<author>igor</author>"
							"<source>test</source>"
							"<desc/>"
							"<centroid x='10' y='20' custom='0'/>"
							"<line width='600' layer='2' x1='100' y1='200' x2='1000' y2='2000' />"
							"<pins>"
							"<pin name='1' x='100' y='200' rot='90' padstack='101'/>"
							"<pin name='2' x='400' y='600' rot='0' padstack='101'/>"
							"</pins>"
							"<refText x='1000' y='1100' rot='90' lineWidth='99' textSize='200'/>"
							"<valueText x='1200' y='1400' rot='0' lineWidth='102' textSize='220'/>"
							"</footprint>");
	fpxml.readNextStartElement();
	Footprint* fp = Footprint::newFromXML(fpxml, padstacks);
	doc.mFootprints.append(fp);
	QXmlStreamReader partxml("<part refdes='P1' value='val' footprint='RES0805' x='1' y='2' rot='270' side='bot' locked='1'>"
							"<refText x='0' y='0' rot='270' lineWidth='42' textSize='212'/>"
							"<valueText x='1' y='1' rot='180' lineWidth='55' textSize='443'/>"
							"</part>");
	partxml.readNextStartElement();
	Part* p = Part::newFromXML(partxml, &doc);
	QVERIFY(p != NULL);
	QCOMPARE(p->refdes(), QString("P1"));
	QCOMPARE(p->value(), QString("val"));
	QCOMPARE(p->pos(), QPoint(1,2));
	QCOMPARE(p->angle(), 270);
	QCOMPARE(p->side(), SIDE_BOTTOM);
	QCOMPARE(p->locked(), true);
	QCOMPARE(p->refdesText()->pos(), QPoint(1,2));
	QCOMPARE(p->refdesText()->angle(), 270);
	QCOMPARE(p->refdesText()->strokeWidth(), 42);
	QCOMPARE(p->refdesText()->fontSize(), 212);
	QCOMPARE(p->valueText()->pos(), QPoint(0,1));
	QCOMPARE(p->valueText()->angle(), 180);
	QCOMPARE(p->valueText()->strokeWidth(), 55);
	QCOMPARE(p->valueText()->fontSize(), 443);
	delete p;
}

void XmlLoadTest::testNet()
{
	// create test objects
	// we need a document, a padstack, a footprint, 2 part objects
	PCBDoc doc;
	Padstack* ps = new Padstack();
	doc.mPadstacks.append(ps);
	QHash<int, Padstack*> padstacks;
	padstacks.insert(101, ps);
	QXmlStreamReader fpxml("<footprint>"
							"<name>RES0805</name>"
							"<units>mm</units>"
							"<author>igor</author>"
							"<source>test</source>"
							"<desc/>"
							"<centroid x='10' y='20' custom='0'/>"
							"<line width='600' layer='2' x1='100' y1='200' x2='1000' y2='2000' />"
							"<pins>"
							"<pin name='1' x='100' y='200' rot='90' padstack='101'/>"
							"<pin name='2' x='400' y='600' rot='0' padstack='101'/>"
							"</pins>"
							"<refText x='1000' y='1100' rot='90' lineWidth='99' textSize='200'/>"
							"<valueText x='1200' y='1400' rot='0' lineWidth='102' textSize='220'/>"
							"</footprint>");
	fpxml.readNextStartElement();
	Footprint* fp = Footprint::newFromXML(fpxml, padstacks);
	doc.mFootprints.append(fp);
	QXmlStreamReader partxml("<part refdes='P1' value='val' footprint='RES0805' x='1' y='2' rot='270' side='bot' locked='1'>"
							"<refText x='2000' y='2200' rot='270' lineWidth='42' textSize='212'/>"
							"<valueText x='3200' y='3400' rot='180' lineWidth='55' textSize='443'/>"
							"</part>");
	partxml.readNextStartElement();
	Part* p = Part::newFromXML(partxml, &doc);
	doc.mParts.append(p);
	QXmlStreamReader partxml2("<part refdes='P2' value='val' footprint='RES0805' x='1000' y='2000' rot='270' side='bot' locked='1'>"
							"<refText x='2000' y='2200' rot='270' lineWidth='42' textSize='212'/>"
							"<valueText x='3200' y='3400' rot='180' lineWidth='55' textSize='443'/>"
							"</part>");
	partxml2.readNextStartElement();
	Part* p2 = Part::newFromXML(partxml2, &doc);
	doc.mParts.append(p2);

	// now test the net
	QXmlStreamReader netxml("<net name='n1' visible='1' defViaPadstack='101'>"
							"<pinRef partref='P1' pinname='1'/>"
							"<pinRef partref='P2' pinname='1'/>"
							"<pinRef partref='P1' pinname='2'/>"
							"</net>");
	netxml.readNextStartElement();
	Net* n = Net::newFromXML(netxml, &doc, padstacks);
	QVERIFY(n != NULL);
	QCOMPARE(n->name(), QString("n1"));
	QCOMPARE(n->visible(), true);
	QCOMPARE(n->getViaPs(), ps);
	QSet<PartPin*> pins = n->getPins();
	QCOMPARE(pins.size(), 3);
	foreach(PartPin* pin, pins)
	{
		QVERIFY( ( (pin->getPart()->refdes() == "P1") && (pin->getName() == "1") ) ||
				 ( (pin->getPart()->refdes() == "P1") && (pin->getName() == "2") ) ||
				 ( (pin->getPart()->refdes() == "P2") && (pin->getName() == "1") ) );
	}
	delete n;
}

void XmlLoadTest::testArea()
{
	// create test objects
	// we need a document, a padstack, a footprint, a part object, and a net object
	PCBDoc doc;
	Padstack* ps = new Padstack();
	doc.mPadstacks.append(ps);
	QHash<int, Padstack*> padstacks;
	padstacks.insert(101, ps);
	QXmlStreamReader fpxml("<footprint>"
							"<name>RES0805</name>"
							"<units>mm</units>"
							"<author>igor</author>"
							"<source>test</source>"
							"<desc/>"
							"<centroid x='10' y='20' custom='0'/>"
							"<line width='600' layer='2' x1='100' y1='200' x2='1000' y2='2000' />"
							"<pins>"
							"<pin name='1' x='100' y='200' rot='90' padstack='101'/>"
							"<pin name='2' x='400' y='600' rot='0' padstack='101'/>"
							"</pins>"
							"<refText x='1000' y='1100' rot='90' lineWidth='99' textSize='200'/>"
							"<valueText x='1200' y='1400' rot='0' lineWidth='102' textSize='220'/>"
							"</footprint>");
	fpxml.readNextStartElement();
	Footprint* fp = Footprint::newFromXML(fpxml, padstacks);
	doc.mFootprints.append(fp);
	QXmlStreamReader partxml("<part refdes='P1' value='val' footprint='RES0805' x='1' y='2' rot='270' side='bot' locked='1'>"
							"<refText x='2000' y='2200' rot='270' lineWidth='42' textSize='212'/>"
							"<valueText x='3200' y='3400' rot='180' lineWidth='55' textSize='443'/>"
							"</part>");
	partxml.readNextStartElement();
	Part* p = Part::newFromXML(partxml, &doc);
	doc.mParts.append(p);
	QXmlStreamReader netxml("<net name='n1' visible='1' defViaPadstack='101'>"
							"<pinRef partref='P1' pinname='1'/>"
							"<pinRef partref='P1' pinname='2'/>"
							"</net>");
	netxml.readNextStartElement();
	Net* n = Net::newFromXML(netxml, &doc, padstacks);
	doc.mNets.append(n);


	// now test the area
	QXmlStreamReader areaxml("<area net='n1' layer='5' hatch='full' connectSmt='1'>"
							 "<polygon>"
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
							 "</area>");
	areaxml.readNextStartElement();
	Area* a = Area::newFromXML(areaxml, doc);
	QCOMPARE(a->layer(), (PCBLAYER)5);
	QCOMPARE(a->hatchStyle(), Area::DIAGONAL_FULL);
	QCOMPARE(a->net(), n);
	QCOMPARE(a->connSmt(), true);
	QCOMPARE(a->poly()->numHoles(), 1);
	QCOMPARE(a->poly()->outline()->numSegs(), 4);
	QCOMPARE(a->poly()->hole(0)->numSegs(), 4);

	delete a;
}

void XmlLoadTest::testDoc()
{
	QTemporaryFile file;
	file.open();
	file.write("<xpcbBoard>"
				"<props>"
				"<units>mm</units>"
				"<numLayers>2</numLayers>"
				"<name>test board</name>"
				"<defaultPadstack>100</defaultPadstack>"
				"</props>\n"

				"<padstacks>"
				"<padstack name='pstest' id='100' holesize='1000'><!-- fark -->"
				"<startpad><pad shape='square' width='10000'/></startpad>"
				"<innerpad><pad shape='round' width='15000'/><!-- asdf --></innerpad>"
				"<!-- comment -->"
				"<endpad><!-- another comment --><pad shape='rect' width='20000' height='5000'/><!--more comments --></endpad>"
				"<startmask/><endmask/><startpaste/><endpaste/>"
				"</padstack>"
				"</padstacks>"

				"<footprints>"
				"<footprint>"
				"<name>RES0805</name>"
				"<units>mm</units>"
				"<author>igor</author>"
				"<source>test</source>"
				"<desc/>"
				"<centroid x='10' y='20' custom='0'/>"
				"<line width='600' layer='2' x1='100' y1='200' x2='1000' y2='2000' />"
				"<pins>"
				"<pin name='1' x='100' y='200' rot='90' padstack='100'/>"
				"<pin name='2' x='400' y='600' rot='0' padstack='100'/>"
				"</pins>"
				"<refText x='1000' y='1100' rot='90' lineWidth='99' textSize='200'/>"
				"<valueText x='1200' y='1400' rot='0' lineWidth='102' textSize='220'/>"
				"</footprint>"
				"</footprints>"

				"<outline>"
				"<polygon>"
				"<outline>"
				"<start x='0' y='0'/>"
				"<lineTo x='0' y='1000'/>"
				"<arcTo x='1000' y='0' dir='cw'/>"
				"<lineTo x='0' y='0'/>"
				"</outline>"
				"<!-- more random comments -->"
				"<hole>"
				"<start x='100' y='100'/>"
				"<arcTo x='200' y='200' dir='cw'/>"
				"<lineTo x='200' y='100'/>"
				"<lineTo x='100' y='100'/>"
				"<!-- random comment -->"
				"</hole>"
				"</polygon>"
				"</outline>"

				"<parts>"
				"<part refdes='P1' value='val' footprint='RES0805' x='1' y='2' rot='270' side='bot' locked='1'>"
				"<refText x='2000' y='2200' rot='270' lineWidth='42' textSize='212'/>"
				"<valueText x='3200' y='3400' rot='180' lineWidth='55' textSize='443'/>"
				"</part>"
				"<part refdes='P2' value='val' footprint='RES0805' x='1' y='2' rot='270' side='bot' locked='1'>"
				"<refText x='2000' y='2200' rot='270' lineWidth='42' textSize='212'/>"
				"<valueText x='3200' y='3400' rot='180' lineWidth='55' textSize='443'/>"
				"</part>"
				"</parts>"

				"<nets>"
				"<net name='n1' visible='1' defViaPadstack='100'>"
				"<pinRef partref='P1' pinname='1'/>"
				"<pinRef partref='P2' pinname='2'/>"
				"</net>"
				"<net name='n2' visible='1' defViaPadstack='100'>"
				"<pinRef partref='P2' pinname='1'/>"
				"<pinRef partref='P1' pinname='2'/>"
				"</net>"
				"</nets>"

				"<traces>"
				"<!-- asdf --><vertices>"
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
				"</segments><!-- asdf -->"
				"</traces>"

				"<areas>"
				"<area net='n1' layer='5' hatch='full' connectSmt='1'>"
				"<polygon>"
				"<outline>"
				"<start x='0' y='0'/>"
				"<lineTo x='0' y='1000'/>"
				"<arcTo x='1000' y='0' dir='cw'/>"
				"<lineTo x='0' y='0'/>"
				"</outline>"
				"<!-- more random comments -->"
				"<hole>"
				"<start x='100' y='100'/>"
				"<arcTo x='200' y='200' dir='cw'/>"
				"<lineTo x='200' y='100'/>"
				"<lineTo x='100' y='100'/>"
				"<!-- random comment -->"
				"</hole>"
				"</polygon>"
				"</area>"
				"<area net='n2' layer='3' hatch='full' connectSmt='1'>"
				"<polygon>"
				"<outline>"
				"<start x='0' y='0'/>"
				"<lineTo x='0' y='1000'/>"
				"<arcTo x='1000' y='0' dir='cw'/>"
				"<lineTo x='0' y='0'/>"
				"</outline>"
				"<!-- more random comments -->"
				"<hole>"
				"<start x='100' y='100'/>"
				"<arcTo x='200' y='200' dir='cw'/>"
				"<lineTo x='200' y='100'/>"
				"<lineTo x='100' y='100'/>"
				"<!-- random comment -->"
				"</hole>"
				"</polygon>"
				"</area>"
				"</areas>"

				"<texts>"
				"<text layer='2' x='100' y='200' rot='180' lineWidth='300' textSize='400'> testing 123  </text>"
				"<text layer='2' x='200' y='500' rot='180' lineWidth='300' textSize='400'> testing 123  </text>"

				"</texts>"

				"</xpcbBoard>"
				);
	file.reset();


	PCBDoc d;
	bool ret = d.loadFromFile(file);

	QCOMPARE(ret, true);
	QCOMPARE(d.name(), QString("test board"));
	QCOMPARE(d.units(), MM);
	QCOMPARE(d.mTraceList->segments().size(), 3);
	QCOMPARE(d.mTraceList->vertices().size(), 4);
	QCOMPARE(d.mAreas.size(), 2);
	QCOMPARE(d.mTexts.size(), 2);
	QCOMPARE(d.mNets.size(), 2);
	QCOMPARE(d.mParts.size(), 2);

	file.close();
}
