#include "Log.h"


Log& Log::instance()
{
	if (!mInstance)
		mInstance = new Log();
	return *mInstance;
}

Log::Log()
{
}

