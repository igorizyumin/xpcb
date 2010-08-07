#ifndef LOG_H
#define LOG_H

#include <QString>

/// Log implements a singleton for centralized message logging in the application
class Log
{
public:
	static Log& instance();

	void message(QString msg);
	void warning(QString msg);
	void error(QString msg);

private:
    Log();
	Log(Log& other);

	static Log* mInstance;
};

#endif // LOG_H
