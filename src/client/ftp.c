#include "ftp.h"

int connect_port(unsigned short port, const char *ip)
{
	int sockfd, ret;
	struct sockaddr_in server_addr, local_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		return sockfd;
	}

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr));
	if (ret < 0) {
		printf("bind error\n");
		close(sockfd);
		return ret;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);

	ret = connect(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret < 0) {
		printf("connect error\n");
		close(sockfd);

		return ret;
	}

	return sockfd;
}

int ftp_send_cmd(int cmd_fd, const char *cmd, const char *arg)
{
	char buf[BUF_LEN];
	int ret;

	if (arg != NULL)
		snprintf(buf, BUF_LEN, "%s %s\r\n", cmd, arg);
	else
		sprintf(buf, "%s\r\n", cmd);

	ret = send(cmd_fd, buf, strlen(buf), 0);
	if (ret < 0) {
		printf("send error\n");
		return ret;
	}

	ret = recv(cmd_fd, buf, BUF_LEN - 1, 0);
	if (ret < 0) {
		printf("recv error\n");
		return ret;
	}

	buf[ret] = '\0';
	printf("%s", buf);

	if (!strncmp(buf, USER_PSWD_ERROR, 3)) {
		ret = -1;
	} else if (!strncmp(buf, NO_SUCH_FILE, 3)) {
		ret = -1;
	}

	return ret;
}

int ftp_login(const char *ip)
{
	int sockfd, ret;
	char buf[BUF_LEN];
	char user[BUF_LEN], pwd[BUF_LEN];

	sockfd = connect_port(FTP_PORT, ip);
	if (sockfd < 0)
	{
		return sockfd;
	}

	ret = recv(sockfd, buf, BUF_LEN, 0);
	buf[ret] = '\0';
	printf("%s\n", buf);

	printf("Name :");
	scanf("%s", user);
	ret = ftp_send_cmd(sockfd, "USER", user);

	printf("PassWord :");
	scanf("%s", pwd);
	ret = ftp_send_cmd(sockfd, "PASS", pwd);

	return sockfd;
}
