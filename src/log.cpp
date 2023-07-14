#include "log.h"
#include "option.h"
#include "logger.h"
#include "format.h"
#include "exception.h"

#include "config.h"

namespace YQ {

static string
now_time()
{
	time_t current = time (NULL);
	char buf[1024];
	strftime (buf, sizeof (buf),
	          "%Y/%m/%d %H:%M:%S"
	          ,  localtime (&current));
	return buf;
}


string
log_format (RESULT result, TYPE type, const string &protectee, const string &exe, pid_t pid)
{
	return format ("%s  %s  %s\t%s\t\t\texe:[%s  %ld]",
	               now_time().c_str(),
	               result == RESULT::ALLOW ? "ALLOW" : "DENY",
	               type == TYPE::FILE ? "open" : "connect",
	               protectee.c_str(),
	               exe.c_str(),
	               pid
	              );
}

Logger &get_logger()
{
	static Logger logger (LOG_FILE_PATH);
	return logger;
}

void
log (RESULT result, TYPE type, const string &protectee, const string &exe, pid_t pid)
{
	if (!ENABLE_ALLOW_LOG && result == RESULT::ALLOW)
		return;
	if (!ENABLE_DENY_LOG && result == RESULT::DENY)
		return;
	get_logger() << log_format (result, type, protectee, exe, pid) << "\n";
}

}