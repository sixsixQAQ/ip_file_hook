#pragma once

#include <stdexcept>
#include <string>

namespace YQ {

using std::exception;
using std::string;

class Exception : public exception {
public:
	Exception (const string &msg = "",
	           const string &file = __builtin_FILE(),
	           const string &func = __builtin_FUNCTION(),
	           size_t line = __builtin_LINE()
	          );
	          
	virtual ~Exception() = default;
	const char *what() const noexcept override;
private:
	string m_buf;
	string m_msg;
	string m_file;
	string m_func;
	size_t m_line;
};

}