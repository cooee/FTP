#ifndef _FTP_H
#define _FTP_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define FTP_PORT 21
#define BUF_LEN  512
#define USER_PSWD_ERROR "530"
#define NO_SUCH_FILE    "550"

int connect_port(unsigned short port, const char *ip);
int ftp_send_cmd(int cmd_fd, const char *cmd, const char *arg);
int ftp_login(const char *ip);
#endif
