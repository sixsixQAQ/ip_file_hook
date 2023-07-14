
#include "ptrace_tool.h"
#include <sys/inotify.h>
#include "config.h"
#include "exception.h"
#include <limits.h>
#include <unistd.h>
#include <string>
#include "tool.h"
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <syscall.h>
#include <fcntl.h>
#include <assert.h>

using namespace std;
using namespace YQ;

struct InotifyException : Exception {
	using Exception::Exception;
};

constexpr int inotify_event_size = sizeof (inotify_event) + NAME_MAX + 1;

pid_t get_pid_by_path (string exe_denied)
{
	string real_exe_denied = get_real_path (exe_denied.c_str());
	for (auto i : get_all_pids()) {
		string exe = get_real_exe_by_pid (i);
		if (exe == real_exe_denied)
			return i;
	}
	return -1;
}


void track (pid_t pid, string protected_file)
{
	fprintf (stderr, "pid:%d,protected_file:%s\n", pid, protected_file.c_str());
	
	if (ptrace (PTRACE_ATTACH, pid, NULL, NULL) == -1)
		throw PtraceException ();
	int status;
	struct user_regs_struct regs;
	while (waitpid (pid, &status, 0) != -1 && !WIFEXITED (status)) {
		if (ptrace (PTRACE_GETREGS, pid, NULL, &regs) == -1)
			throw PtraceException ();
		if (regs.orig_rax == SYS_open) {
			char *remote_pathname = (char *) regs.rdi;
			char *pathname =  get_tracee_strdup (pid, remote_pathname);
			string full_pathname = pathname;
			{
				char buf[PATH_MAX + 1];
				snprintf (buf, sizeof (buf), "/proc/%d/cwd", pid);
				char cwd[PATH_MAX + 1];
				if (readlink (buf, cwd, sizeof (cwd) - 1) == -1)
					throw PtraceException ();
				full_pathname = string (cwd) + "/" + pathname;
#ifdef OUTPUT
				fprintf (stderr, "openat():%s\n", full_pathname.c_str());
#endif
			}
			if (full_pathname != protected_file)
				ptrace (PTRACE_SYSCALL, pid, NULL, NULL);
			else {
				set_tracee_bytes (pid, (void *) "", remote_pathname, 1);
				ptrace (PTRACE_SYSCALL, pid, NULL, NULL);
			}
			free (pathname);
		} else if (regs.orig_rax == SYS_openat) {
			char *remote_pathname = (char *) regs.rsi;
			char *pathname =  get_tracee_strdup (pid, remote_pathname);//segment fault
			
			string full_pathname = pathname;
			
			{
				if (pathname[0] != '/') {
					const int dirfd = regs.rdi;
					if (dirfd == AT_FDCWD) {
						char buf[PATH_MAX + 1];
						snprintf (buf, sizeof (buf), "/proc/%d/cwd", pid);
						char cwd[PATH_MAX + 1];
						if (readlink (buf, cwd, sizeof (cwd) - 1) == -1)
							throw PtraceException ();
						full_pathname = string (cwd) + "/" + pathname;
					} else { //TODO 此处应验证dirfd是否合法
						// TODO 较为复杂，需要控制tracee获取描述符表示的路径。
						fprintf (stderr, "未实现，无法拦截\n");
					}
				}
#ifdef OUTPUT
				fprintf (stderr, "openat():%s\n", full_pathname.c_str());
#endif
			}
			if (full_pathname != protected_file) {
				if (ptrace (PTRACE_SYSCALL, pid, NULL, NULL) == -1)
					throw PtraceException ();
			} else {
				regs.rax = -1;
				regs.orig_rax = -1;
				if (ptrace (PTRACE_SETREGS, pid, NULL, &regs) == -1)
					throw PtraceException ();
				if (ptrace (PTRACE_CONT, pid, NULL, NULL) == -1)
					throw PtraceException ();
			}
			free (pathname);
		} else {
			if (ptrace (PTRACE_SYSCALL, pid, NULL, NULL) == -1)
				throw PtraceException ();
		}
	}
}

int main (void)
{
	int inotify_fd = inotify_init();
	
	map<int, string>wd_name_table;
	
	string protected_file;//第一个被保护文件
	for (auto &file_exes : file_black_list()) {
		protected_file  = file_exes.first;
		for (auto &exe : file_exes.second) {
			int wd = inotify_add_watch (inotify_fd, exe.c_str(), IN_OPEN);//添加保护文件的黑名单程序，以监控它们
			if (wd == -1)
				continue;
			else
				wd_name_table.insert (make_pair (wd, exe));
		}
		break;
	}
	
	char buf[inotify_event_size];
	inotify_event *p_event = (inotify_event *) buf;
	for (; ;) {
		ssize_t nread = read (inotify_fd, buf, sizeof (buf));
		if (nread != sizeof (inotify_event) + p_event->len)
			throw InotifyException ();
		if (p_event->mask & IN_OPEN) {//黑名单进程被open()，说明黑名单进程启动了
			fprintf (stderr, "监测到黑名单程序<%s>被打开\n", wd_name_table.at (p_event->wd).c_str());
			pid_t pid = get_pid_by_path (wd_name_table.at (p_event->wd)); //获取黑名单进程的PID
			track (pid, protected_file);//这里应该开一个detatch线程来调用track
		}
	}
	
}