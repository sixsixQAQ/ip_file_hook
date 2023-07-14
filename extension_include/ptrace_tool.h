#pragma once

#include <sys/types.h>
#include "exception.h"

namespace YQ {

int get_tracee_strlen (pid_t pid, char *remote_str) noexcept (false);
char *get_tracee_strdup (pid_t pid, char *remote_str) noexcept (false);

void get_tracee_bytes (pid_t pid, void *remote_src, void *local_dest, size_t len) noexcept (false);
void set_tracee_bytes (pid_t pid, void *remote_src, void *locat_dest, size_t len) noexcept (false);


struct PtraceException : Exception {
	using Exception::Exception;
};

}