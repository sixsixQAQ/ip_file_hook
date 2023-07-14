#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <err.h>

int main (int argc, char **argv)
{
	if (argc != 2) {
		fprintf (stderr, "Usage: %s <file>\n", argv[0]);
		return 0;
	}
	
	int fd = open (argv[1], O_RDONLY | O_CREAT, 0644);
	if (fd == -1) {
		fprintf (stderr, "Failed to open file:%s\n", strerror (errno));
		return -1;
	} else
		fprintf (stderr, "Open successfully\n");
	close (fd);
}
