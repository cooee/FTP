#include "ftp.h"

int main(int argc, char *argv[])
{
	int sockfd;

	if (argc < 2)
	{
		printf("Usage : ./ftp ip\n");
		return -1;
	}

	sockfd = ftp_login(argv[1]);
	if (sockfd < 0)
	{
		printf("login failed\n");
		return sockfd;
	}

	return 0;
}
