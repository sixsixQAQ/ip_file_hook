#include <map>
#include <string>
#include <set>
#include <memory>

#include <cstring>
#include <dlfcn.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>

#include "config.h"
#include "tool.h"
#include "log.h"

namespace YQ {

using std::map;
using std::string;
using std::set;
using std::unique_ptr;

static bool
allow_open (const char *exe, const char *path, string &return_full_path) noexcept (true)
try
{
	if (exe == NULL || path == NULL)
		return false;
	const string real_absolute_path = get_real_path (get_absolute_path (path).c_str());
	return_full_path = real_absolute_path;
	for (auto &i : file_black_list()) {
		if (i.first == real_absolute_path) { //文件受黑名单保护
			if (i.second.count (exe) > 0) //程序在黑名单
				return false;
		}
	}
	
	for (auto &i : file_white_list()) {
		if (i.first == real_absolute_path) { //文件受白名单保护
			if (i.second.count (exe) == 0) //程序不在白名单
				return false;
		}
	}
	return true;
} catch (...)
{
	return true;
}

static bool
allow_ipv4 (int sockfd, const struct sockaddr *address, socklen_t addrlen, string &return_ip) noexcept (true)
try
{
	char buf[INET_ADDRSTRLEN];
	
	struct sockaddr_in *addr_in = (struct sockaddr_in *) address;
	if (addrlen < sizeof (sockaddr_in) ||
	    addr_in->sin_family != AF_INET) //这里一定要加上长度判断，防止内存越界!!!
		return true;
	if (inet_ntop (AF_INET, &addr_in->sin_addr, buf, sizeof (buf)) == NULL)
		return true;
	const string ip = string (buf);
	return_ip = ip;
	const string exe =  get_real_exe_by_pid (getpid());
	for (auto &i : ip_black_list()) {
		if (i.first == ip) {
			if (i.second.count (exe) > 0)
				return false;
		}
	}
	for (auto &i : ip_white_list()) {
		if (i.first == ip) {
			if (i.second.count (exe) == 0)
				return false;
		}
	}
	return true;
} catch (...)
{
	return true;
}


}

extern "C" {

	using YQ::allow_open;
	using YQ::allow_ipv4;
	using YQ::get_real_exe_by_pid;
	using YQ::log;
	using YQ::RESULT;
	using YQ::TYPE;
	
	
	
	typedef int (*open_func_t) (const char *, int, ...);
	int
	open (const char *path, int flags, ...)
	{
		static open_func_t old_open = NULL;
		if (old_open == NULL)
			old_open = (open_func_t) dlsym (RTLD_NEXT, "open");
			
		mode_t mode = 0;
		if (flags & O_CREAT) {
			va_list args;
			
			va_start (args, flags);
			mode = va_arg (args, mode_t);
			va_end (args);
		}
		std::string full_path;
		if (allow_open (get_real_exe_by_pid (getpid()).c_str(), path, full_path)) {
			log (RESULT::ALLOW, TYPE::FILE, full_path, get_real_exe_by_pid (getpid()), getpid());
			return old_open (path, flags, mode);
		} else {
			log (RESULT::DENY, TYPE::FILE, full_path, get_real_exe_by_pid (getpid()), getpid());
			errno = EPERM;
			return -1;
		}
	}
	
	typedef int (*openat_func_t) (int fd, const char *, int, ...);
	int
	openat (int fd, const char *path, int flags, ...)
	{
		static openat_func_t old_openat = NULL;
		if (old_openat == NULL)
			old_openat = (openat_func_t) dlsym (RTLD_NEXT, "openat");
			
		mode_t mode = 0;
		if (flags & O_CREAT) {
			va_list args;
			
			va_start (args, flags);
			mode = va_arg (args, mode_t);
			va_end (args);
		}
		std::string full_path;
		if (allow_open (get_real_exe_by_pid (getpid()).c_str(), path, full_path)) {
			log (RESULT::ALLOW, TYPE::FILE, full_path, get_real_exe_by_pid (getpid()), getpid());
			return old_openat (fd, path, flags, mode);
		} else {
		
			log (RESULT::DENY, TYPE::FILE, full_path, get_real_exe_by_pid (getpid()), getpid());
			
			errno = EPERM;
			return -1;
		}
	}
	
	
	
	
	typedef int (*connect_func_t) (int, const struct sockaddr *, socklen_t);
	int
	connect (int sockfd, const struct sockaddr *address, socklen_t addrlen)
	{
		static connect_func_t old_connect = NULL;
		if (old_connect == NULL)
			old_connect = (connect_func_t) dlsym (RTLD_NEXT, "connect");
		std::string ip = "Unknown_IP";
		if (allow_ipv4 (sockfd, address, addrlen, ip)) {
			log (RESULT::ALLOW, TYPE::IP, ip, get_real_exe_by_pid (getpid()), getpid());
			return old_connect (sockfd, address, addrlen);
		} else {
			log (RESULT::DENY, TYPE::IP, ip, get_real_exe_by_pid (getpid()), getpid());
			errno = EPERM;
			return -1;
		}
	}
	
	typedef ssize_t (*sendto_func_t)
	(
	    int sockfd,
	    const void *buf,
	    size_t len,
	    int flags,
	    const struct sockaddr *dest_addr,
	    socklen_t addrlen
	);
	ssize_t
	sendto (int sockfd, const void *buf, size_t len, int flags,
	        const struct sockaddr *dest_addr, socklen_t addrlen)
	{
		static sendto_func_t old_sendto = NULL;
		if (old_sendto == NULL)
			old_sendto = (sendto_func_t) dlsym (RTLD_NEXT, "sendto");
			
		std::string ip = "Unknown_IP";
		if (allow_ipv4 (sockfd, dest_addr, addrlen, ip)) {
			log (RESULT::ALLOW, TYPE::IP, ip, get_real_exe_by_pid (getpid()), getpid());
			return old_sendto (sockfd, buf, len, flags, dest_addr, addrlen);
		} else {
			log (RESULT::DENY, TYPE::IP, ip, get_real_exe_by_pid (getpid()), getpid());
			errno = EPERM;
			return -1;
		}
	}
	
}