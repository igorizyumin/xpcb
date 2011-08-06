/*
	Copyright (C) 2010-2011 Igor Izyumin

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

#ifndef LOG_H
#define LOG_H

#include <QString>

/// Log implements a singleton for centralized message logging in the application
class Log
{
public:
	static Log& instance();

	static void message(QString msg) { instance().instMessage(msg); }
	static void warning(QString msg) { instance().instWarning(msg); }
	static void error(QString msg) { instance().instError(msg); }

private:
    Log();
	Log(Log& other);

	void instMessage(QString msg);
	void instWarning(QString msg);
	void instError(QString msg);


	static Log* mInstance;
};

#endif // LOG_H
