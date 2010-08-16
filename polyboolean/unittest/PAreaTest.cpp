#include <QtTest/QtTest>
#include "AutoTest.h"
#include <polybool.h>
#include "pbio.h"

using namespace POLYBOOLEAN;

class PAreaTest : public QObject
{
	Q_OBJECT

public:

private Q_SLOTS:
	void testArea();
	void testBool();


	// empty slots so we don't get annoying QWARN output
	void init() {}
	void cleanup() {}
	void initTestCase() {}
	void cleanupTestCase() {}
};

void PAreaTest::testArea()
{
	static GRID2 b[11] =
	{ GRID2(-5, -6), GRID2(7,-6), GRID2(7, 4), GRID2(-5, 4), GRID2(0, 0), GRID2(0, 2),
	GRID2(5, 2), GRID2(5, -4), GRID2(0, -4), GRID2(0, 0), GRID2(-5, 0) };
	static GRID2 a1[4] =
	{ GRID2(-7, 8), GRID2(-7, -3), GRID2(2, -3), GRID2(2, 8) };
	static GRID2 a2[4] =
	{ GRID2(-5, 6), GRID2(0, 6), GRID2(0, 0), GRID2(-5, 0) };

	PLINE2 pla1(a1, 4);
	pla1.Prepare();
	if (!pla1.IsOuter())
		pla1.Invert();

	PLINE2 pla2(a2, 4);
	pla2.Prepare();
	if (pla2.IsOuter())
		pla2.Invert();

	PLINE2 plb(b, 11);
	plb.Prepare();
	if (!plb.IsOuter())
		plb.Invert();

	PAREA *list1 = NULL, *list2 = NULL;
	PAREA::AddPlineToList(&list1, pla1.Copy());
	PAREA::AddPlineToList(&list1, pla2.Copy());
	PAREA::AddPlineToList(&list2, plb.Copy());

	// inside pline
	static GRID2 testgr[4] = { GRID2(-6, 7), GRID2(-6,-2), GRID2(1, -2), GRID2(1, 7) };
	PLINE2 pltest(testgr, 4);
	pltest.Prepare();
	pltest.makeOuter();

	// test area inside
	bool result;
	// inside area
	result = list1->GridInside(GRID2(1, -1));
	QCOMPARE(result, true);

	// outside area
	result = list1->GridInside(GRID2(-8, 8));
	QCOMPARE(result, false);

	// in hole
	result = list1->GridInside(GRID2(-1, 5));
	QCOMPARE(result, false);

	// test pline
	QCOMPARE(list1->PlineInside(pltest), true);

	PAREA::Del(&list1);
	PAREA::Del(&list2);
}

void PAreaTest::testBool()
{
	static GRID2 a[4] = {GRID2(0,0), GRID2(10,0), GRID2(10,10), GRID2(0,10)};
	static GRID2 b[4] = {GRID2(5,0), GRID2(15,0), GRID2(15,10), GRID2(5,10)};
	PLINE2 pla(a,4);
	PLINE2 plb(b,4);
	QCOMPARE(pla.Prepare(), true);
	QCOMPARE(plb.Prepare(), true);
	pla.makeOuter();
	plb.makeOuter();

	PAREA *a1 = NULL, *a2 = NULL;
	PAREA::AddPlineToList(&a1, pla.Copy());
	PAREA::AddPlineToList(&a2, plb.Copy());

	PAREA *r = NULL;

	// test XOR
	PAREA::Boolean(a2, a1, &r, PAREA::XOR);
	SaveParea("xor_polygon", r);
	QCOMPARE(r->GridInside(GRID2(2,5)), true);
	QCOMPARE(r->GridInside(GRID2(7,5)), false);
	QCOMPARE(r->GridInside(GRID2(12,5)), true);
	QCOMPARE(r->GridInside(GRID2(20,5)), false);

	PAREA::Del(&r);


	// test OR
	PAREA::Boolean(a1, a2, &r, PAREA::OR);
	SaveParea("or_polygon", r);
	QCOMPARE(r->GridInside(GRID2(2,5)), true);
	QCOMPARE(r->GridInside(GRID2(7,5)), true);
	QCOMPARE(r->GridInside(GRID2(12,5)), true);
	QCOMPARE(r->GridInside(GRID2(20,5)), false);

	PAREA::Del(&r);

	// test AND
	PAREA::Boolean(a1, a2, &r, PAREA::AND);
	SaveParea("and_polygon", r);
	QCOMPARE(r->GridInside(GRID2(2,5)), false);
	QCOMPARE(r->GridInside(GRID2(7,5)), true);
	QCOMPARE(r->GridInside(GRID2(12,5)), false);
	QCOMPARE(r->GridInside(GRID2(20,5)), false);

	PAREA::Del(&r);

	// test SUB
	PAREA::Boolean(a1, a2, &r, PAREA::SUB);
	SaveParea("sub_polygon", r);
	QCOMPARE(r->GridInside(GRID2(2,5)), true);
	QCOMPARE(r->GridInside(GRID2(7,5)), false);
	QCOMPARE(r->GridInside(GRID2(12,5)), false);
	QCOMPARE(r->GridInside(GRID2(20,5)), false);

	PAREA::Del(&r);
}

DECLARE_TEST(PAreaTest);

#include "PAreaTest.moc"

