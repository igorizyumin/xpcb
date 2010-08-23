#include "Log.h"
#include <iostream>

using namespace std;

Log* Log::mInstance = NULL;

Log& Log::instance()
{
	if (!mInstance)
		mInstance = new Log();
	return *mInstance;
}

Log::Log()
{
}

void Log::error(QString msg)
{
	cerr << "ERROR: " << msg.toStdString() << endl;
}

void Log::warning(QString msg)
{
	cerr << "WARN: " << msg.toStdString() << endl;
}

void Log::message(QString msg)
{
	cerr << msg.toStdString() << endl;
}

