#include "format.h"
#include <cstdarg>
#include <memory>

namespace YQ {
using std::unique_ptr;

string
format (const string &format, ...)
{
	char *buf;
	
	va_list args;
	va_start (args, format);
	vasprintf (&buf, format.c_str(), args);
	va_end (args);
	
	unique_ptr<char, decltype (free) *> auto_freed (buf, free);
	return string (buf);
}

}
