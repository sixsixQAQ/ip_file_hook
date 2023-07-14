#include "logger.h"
#include "exception.h"
#include "format.h"

namespace YQ {


Logger::Logger (const string &output_file)
	: m_output_file (output_file)
{
	m_ofstream.open (m_output_file, std::ios::out | std::ios::app);
	throw_if_fail();
}

struct LoggerException : Exception {
	using Exception::Exception;
};

void
Logger::throw_if_fail (const string &file, const string &caller, size_t line)
{
	if (m_ofstream.fail())
		throw LoggerException (
		    format ("Failed to operate on file %s", m_output_file.c_str()), file, caller, line
		);
}

Logger &
operator<< (Logger &logger, ostream & (*op) (ostream &))
{
	(*op) (logger.m_ofstream);
	logger.throw_if_fail();
	return logger;
}

void
Logger::reset()
{
	m_ofstream.clear();
}

}