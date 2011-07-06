/*
	This file is part of xpcb.

	xpcb is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	xpcb is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with xpcb.  If not, see <http://www.gnu.org/licenses/>.
*/

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

