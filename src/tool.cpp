#include <string>
#include <limits.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include "exception.h"
#include <sys/types.h>
#include <dirent.h>
#include "format.h"

namespace YQ {

using std::string;
using std::vector;


struct SocketException : public Exception {
	using Exception::Exception;
};



string
get_real_path (const char *path)
{
	char buf[PATH_MAX + 1];
	if (realpath (path, buf) == NULL)
		return path;
	return buf;
}

string
get_remote_ip_by_fd (int sockfd) noexcept (false)
{
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof (addr);
	
	if (getpeername (sockfd, (sockaddr *) &addr, &addrlen) == -1)
		throw SocketException ();
		
	sockaddr_in *tcp_addr = (sockaddr_in *) &addr;
	char ip[INET_ADDRSTRLEN];
	if (inet_ntop (AF_INET, &tcp_addr->sin_addr, ip, sizeof (ip)) == NULL)
		throw SocketException ();
	return string (ip);
}

string
get_real_exe_by_pid (pid_t pid)
{
	string buf = format ("/proc/%d/exe", pid);
	char exe[PATH_MAX];
	ssize_t nread =  readlink (buf.c_str(), exe, sizeof (exe) - 1);
	if (nread == -1)
		return string ();
	exe[nread] = '\0';
	return get_real_path (exe);
}

string
get_absolute_path (const char *path)
{
	if (path == NULL || path[0] == '/')
		return path;
	char buf[PATH_MAX + 1];
	if (getcwd (buf, sizeof (buf) - 1) == NULL)
		return string();
	return string (buf) + "/" + path;
}

vector<pid_t>
get_all_pids()
{
	vector<pid_t> pids;
	
	DIR *p_dir = opendir ("/proc");
	if (p_dir == NULL)
		return pids;
	for (; ;) {
		dirent *p_file = readdir (p_dir);
		if (p_file == NULL)
			break;
		pid_t pid;
		if (p_file->d_type == DT_DIR && (pid = atoi (p_file->d_name)) != 0)
			pids.push_back (pid);
	}
	closedir (p_dir);
	return pids;
}

}