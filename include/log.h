#pragma once

#include <string>

namespace YQ {

using std::string;


enum class RESULT {
	ALLOW,
	DENY
};

enum class TYPE {
	FILE,
	IP
};

void log (RESULT result, TYPE type, const string &protectee, const string &exe, pid_t pid);

}