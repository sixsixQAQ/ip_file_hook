#include "exception.h"
#include "format.h"

namespace YQ {

Exception::Exception (const string &msg,
                      const string &file,
                      const string &func,
                      size_t line
                     )
	: m_msg (msg), m_file (file), m_func (func), m_line (line)
{
	m_buf = format ("[file: %s, caller:%s line: %ld]:%s\n",
	                m_file.c_str(), m_func.c_str(), m_line, m_msg.c_str()
	               );
}

const char *
Exception::what() const noexcept
{
	return m_buf.c_str();
}


}