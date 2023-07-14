#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#include <stdio.h>
#include <err.h>

#define _ERR(call) err(-1, #call"(), %d",__LINE__)

int main (int argc, char **argv)
{
	if (argc != 2 && argc != 3) {
		fprintf (stderr, "Usage: %s <ip> [port]\n", argv[0]);
		return 0;
	}
	
	uint16_t port = 80;
	const char *ip = argv[1];
	
	if (argc == 3)
		port = atoi (argv[2]);
		
	int sockfd = socket (AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
		_ERR (socket);
		
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port);
	if (inet_pton (AF_INET, ip, &addr.sin_addr) != 1)
		_ERR (inet_pton);
		
	if (connect (sockfd, (struct sockaddr *) &addr, sizeof (addr)) == -1)
		_ERR (connect);
		
	printf ("Connect successfully\n");
	
	if (close (sockfd) == -1)
		_ERR (close);
}