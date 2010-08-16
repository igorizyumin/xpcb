#include <QtTest/QtTest>
#include "AutoTest.h"
#include <polybool.h>

using namespace POLYBOOLEAN;

class PLineTest : public QObject
{
    Q_OBJECT

public:

private Q_SLOTS:
	// test creation of linked lists
	void testCreateLine_data();
	void testCreateLine();

	// test line Prepare(), IsOuter(), Invert()
	void testPrepareLine_data();
	void testPrepareLine();

	// test isinside
	void testIsInside();

	// empty slots so we don't get annoying QWARN output
	void init() {}
	void cleanup() {}
	void initTestCase() {}
	void cleanupTestCase() {}
};

void PLineTest::testCreateLine_data()
{
	static GRID2 b[11] =
	{ GRID2(-5, -6), GRID2(7,-6), GRID2(7, 4), GRID2(-5, 4), GRID2(0, 0), GRID2(0, 2),
	GRID2(5, 2), GRID2(5, -4), GRID2(0, -4), GRID2(0, 0), GRID2(-5, 0) };
	static GRID2 a1[4] =
	{ GRID2(-7, 8), GRID2(-7, -3), GRID2(2, -3), GRID2(2, 8) };
	static GRID2 a2[4] =
	{ GRID2(-5, 6), GRID2(0, 6), GRID2(0, 0), GRID2(-5, 0) };

	QTest::addColumn<void*>("vpoly");
	QTest::addColumn<unsigned int>("n_el");
	QTest::newRow("a1") << (void*)&(a1[0]) << (uint)4;
	QTest::newRow("a2") << (void*)&(a2[0]) << (uint)4;
	QTest::newRow("b") << (void*)&(b[0]) << (uint)11;
}

void PLineTest::testCreateLine()
{
	QFETCH(void*, vpoly);
	QFETCH(unsigned int, n_el);

	GRID2* poly = reinterpret_cast<GRID2*>(vpoly);

	PLINE2 pl(poly, n_el);

	QCOMPARE(pl.Count, n_el);
	VNODE2* vn = pl.head;
	for(unsigned int i = 0; i < n_el; i++)
	{
		QCOMPARE(vn->g, poly[i]);
		vn = vn->next;
	}
	QCOMPARE(vn, pl.head);
}

void PLineTest::testPrepareLine_data()
{
	// ccw
	static GRID2 b[11] =
	{ GRID2(-5, -6), GRID2(7,-6), GRID2(7, 4), GRID2(-5, 4), GRID2(0, 0), GRID2(0, 2),
	GRID2(5, 2), GRID2(5, -4), GRID2(0, -4), GRID2(0, 0), GRID2(-5, 0) };
	// ccw
	static GRID2 a1[4] =
	{ GRID2(-7, 8), GRID2(-7, -3), GRID2(2, -3), GRID2(2, 8) };
	// cw
	static GRID2 a2[4] =
	{ GRID2(-5, 6), GRID2(0, 6), GRID2(0, 0), GRID2(-5, 0) };

	// cw, with 3 redundant points
	static GRID2 c[8] =
	{ GRID2(-5, 5), GRID2(-3,5), GRID2(2, 5), GRID2(10, 0),
	  GRID2(10, -5), GRID2(0, -5), GRID2(-5, -5), GRID2(-10,5) };

	// all points collinear
	static GRID2 d[5] = {GRID2(0, 0), GRID2(0, 1), GRID2(0, 3), GRID2(0, 4), GRID2(0,5)};

	// 3 non-collinear points, cw
	static GRID2 e[5] = {GRID2(0, 0), GRID2(0, 0), GRID2(0, 3), GRID2(1, 4), GRID2(2,5)};

	// 3 non-collinear points, cw
	static GRID2 f[5] = {GRID2(0, 1), GRID2(0, 3), GRID2(1, 4), GRID2(2,5), GRID2(0, 0)};


	QTest::addColumn<void*>("vpoly");
	QTest::addColumn<unsigned int>("n_el");
	QTest::addColumn<bool>("clockwise");
	QTest::addColumn<unsigned int>("n_el_after");
	QTest::addColumn<bool>("is_ok");

	QTest::newRow("a1") << (void*)&(a1[0]) << (uint)4 << false << (uint)4 << true;
	QTest::newRow("a2") << (void*)&(a2[0]) << (uint)4 << true << (uint)4 << true;
	QTest::newRow("b") << (void*)&(b[0]) << (uint)11 << false << (uint)11 << true;
	QTest::newRow("c") << (void*)&(c[0]) << (uint)8 << true << (uint)5 << true;
	QTest::newRow("d") << (void*)&(d[0]) << (uint)5 << true << (uint)0 << false;
	QTest::newRow("e") << (void*)&(e[0]) << (uint)5 << true << (uint)3 << true;
	QTest::newRow("f") << (void*)&(f[0]) << (uint)5 << true << (uint)3 << true;

}

void PLineTest::testPrepareLine()
{
	// CCW = outer
	QFETCH(void*, vpoly);
	QFETCH(unsigned int, n_el);
	QFETCH(bool, clockwise);
	QFETCH(unsigned int, n_el_after);
	QFETCH(bool, is_ok);

	GRID2* poly = reinterpret_cast<GRID2*>(vpoly);

	PLINE2 pl(poly, n_el);
	QCOMPARE(pl.Count, n_el);
	bool result = pl.Prepare();
	QCOMPARE(result, is_ok);
	if (is_ok)
	{
		QCOMPARE(pl.Count, n_el_after);
		bool outer = pl.IsOuter();
		QCOMPARE(outer, !clockwise);
		pl.Invert();
		outer = pl.IsOuter();
		QCOMPARE(outer, clockwise);
	}
}

void PLineTest::testIsInside()
{
	static GRID2 p1[3] = {GRID2(-2, -2), GRID2(2, -2), GRID2(0, 2)};
	static GRID2 inside[3] = {GRID2(-1, -1), GRID2(1, -1), GRID2(0, 0)};
	static GRID2 outside[3] = {GRID2(-1, -1), GRID2(1, -1), GRID2(0, 10)};
	PLINE2 pl(p1, 3);
	PLINE2 plin(inside, 3);
	PLINE2 plout(outside, 3);
	bool result = pl.Prepare();
	QCOMPARE(result, true);
	result = plin.Prepare();
	QCOMPARE(result, true);
	result = plout.Prepare();
	QCOMPARE(result, true);

	QCOMPARE(pl.Count, (uint)3);
	QCOMPARE(plin.Count, (uint)3);
	QCOMPARE(plout.Count, (uint)3);

	result = pl.PlineInside(plin);
	QCOMPARE(result, true);
	result = pl.PlineInside(plout);
	QCOMPARE(result, false);
	result = pl.GridInside(GRID2(0, 0));
	QCOMPARE(result, true);
	result = pl.GridInside(GRID2(5, 5));
	QCOMPARE(result, false);
}

DECLARE_TEST(PLineTest);

#include "tst_PLineTest.moc"
