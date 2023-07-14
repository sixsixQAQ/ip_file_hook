#pragma once

#include <string>
#include <vector>

namespace YQ {

using std::string;
using std::vector;

string get_real_path (const char *path);
string get_remote_ip_by_fd (int sockfd) noexcept (false);
string get_absolute_path (const char *path);
string get_real_exe_by_pid (pid_t pid);

vector<pid_t> get_all_pids();

class SocketException;

}