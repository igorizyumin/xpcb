#include "tst_XmlLoadTest.h"
#include "tst_TextTest.h"
#include "tst_UnitSpinboxTest.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	XmlLoadTest loadTest;
	QTest::qExec(&loadTest);
	TextTest textTest;
	QTest::qExec(&textTest);
	UnitSpinboxTest sbTest;
	QTest::qExec(&sbTest);

	return 0;
}
