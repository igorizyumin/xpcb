#include "tst_XmlLoadTest.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	XmlLoadTest loadTest;
	QTest::qExec(&loadTest);

	return 0;
}
