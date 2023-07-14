#include <string>
#include <fstream>
#include <ostream>

namespace YQ {

using std::string;
using std::ofstream;
using std::ostream;


class Logger {
	template<typename T>
	friend Logger &operator<< (Logger &, const T &);
	friend Logger &operator<< (Logger &, ostream & (*) (ostream &));
public:
	Logger (const string &output_file);
	void reset();
	void throw_if_fail (const string &file = __builtin_FILE(),
	                    const string &caller = __builtin_FUNCTION(),
	                    size_t line = __builtin_LINE());
private:
	ofstream m_ofstream;
	string m_output_file;
};

class LoggerException;

template<typename T>
Logger &
operator<< (Logger &logger, const T &var)
{
	logger.m_ofstream << var;
	logger.throw_if_fail();
	return logger;
}

Logger &operator<< (Logger &logger, ostream & (*op) (ostream &));

}