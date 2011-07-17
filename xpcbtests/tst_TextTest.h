#ifndef TST_TEXTTEST_H
#define TST_TEXTTEST_H

#include <QtTest/QtTest>
#include "global.h"

Q_DECLARE_METATYPE(Layer);

class TextTest : public QObject
{
	Q_OBJECT

public:
	TextTest();

private Q_SLOTS:
	void testConstruct_data();
	void testConstruct();
	void testParent();

};


#endif // TST_XMLLOADTEST_H
