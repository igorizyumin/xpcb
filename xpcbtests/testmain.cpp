#include "tst_XmlLoadTest.h"
#include "tst_TextTest.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	XmlLoadTest loadTest;
	QTest::qExec(&loadTest);
	TextTest textTest;
	QTest::qExec(&textTest);

	return 0;
}
