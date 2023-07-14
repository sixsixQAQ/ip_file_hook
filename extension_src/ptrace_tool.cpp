#include "ptrace_tool.h"
#include "exception.h"

#include <sys/ptrace.h>
#include <sys/user.h>
#include <string.h>
#include <stdint.h>

typedef int64_t WORD;

namespace YQ {

[[deprecated]]static void
get_tracee_words (pid_t pid, WORD *src, WORD *dest, size_t len) noexcept (false)
{
	for (size_t i = 0; i < len; ++i) {
		dest[i] = ptrace (PTRACE_PEEKDATA, pid, src + i, NULL);
		if (dest[i] == -1)
			throw PtraceException ();
	}
}

[[deprecated]]static void
set_tracee_words (pid_t pid,  WORD *src,  WORD *dest, size_t len)
{
	for (size_t i = 0; i < len; ++i) {
		if (ptrace (PTRACE_POKEDATA, pid, &dest[i], src[i]) == -1)
			throw PtraceException ();
	}
}

int
get_tracee_strlen (pid_t pid, char *remote_str)
{
	user_regs_struct regs;
	int len = 0;
	for (int i = 0; ; ++i) {
		WORD word = ptrace (PTRACE_PEEKDATA, pid, remote_str + i * sizeof (WORD), NULL);
		if (word == -1)
			throw PtraceException ();
		char *end = (char *) memchr (&word, '\0', sizeof (word));
		if (end == NULL)
			len += sizeof (WORD) / sizeof (char);
		else {
			len += end - (char *) &word;
			break;
		}
	}
	return len;
}

char *
get_tracee_strdup (pid_t pid, char *remote_str)
{
	const int len = get_tracee_strlen (pid, remote_str);
	char buf[len + 1];
	get_tracee_bytes (pid, remote_str, buf, len + 1);
	return strdup (buf);
}

void
get_tracee_bytes (pid_t pid, void *remote_src, void *local_dest, size_t len)
{
	const int word_len = len / sizeof (WORD);
	for (size_t i = 0; i < word_len ; ++i) {
		WORD ret = ptrace (PTRACE_PEEKDATA, pid, (WORD *) remote_src + i, NULL);
		if (ret == -1)
			throw PtraceException ();
		( (WORD *) local_dest) [i] =  ret;
	}
	if (word_len > 1) {
		void *dest_last_word = (WORD *) ( (char *) local_dest + len) - 1;
		void *src_last_word = (WORD *) ( (char *) remote_src + len) - 1;
		if (len % sizeof (WORD) != 0) {
			WORD ret = ptrace (PTRACE_PEEKDATA, pid, src_last_word, NULL);
			if (ret == -1)
				throw PtraceException ();
			* (WORD *) dest_last_word = ret;
		}
	}
}

void set_tracee_bytes (pid_t pid, void *remote_src, void *local_dest, size_t len)
{
	const int word_len = len / sizeof (WORD);
	for (size_t i = 0; i < word_len ; ++i) {
		if (ptrace (PTRACE_POKEDATA, pid, (WORD *) local_dest + i, * ( (WORD *) remote_src + i)) == -1)
			throw PtraceException ();
	}
	if (word_len > 1) {
		void *dest_last_word = (WORD *) ( (char *) local_dest + len) - 1;
		void *src_last_word = (WORD *) ( (char *) remote_src + len) - 1;
		if (len % sizeof (WORD) != 0) {
			if (ptrace (PTRACE_POKEDATA, pid,  dest_last_word, * (WORD *) src_last_word) == -1)
				throw PtraceException ();
		}
	}
}


}