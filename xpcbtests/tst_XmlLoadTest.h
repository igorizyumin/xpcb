#ifndef TST_XMLLOADTEST_H
#define TST_XMLLOADTEST_H

#include <QtTest/QtTest>

class XmlLoadTest : public QObject
{
	Q_OBJECT

public:
	XmlLoadTest();

private Q_SLOTS:
	void testLine();
	void testArc();
	void testText();
	void testPadstack();
	void testTraceList();
	void testPolygon();
	void testFootprint();
	void testPart();
	void testNet();
	void testArea();
	void testDoc();
};

#endif // TST_XMLLOADTEST_H
