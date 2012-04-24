#ifndef FTP_H
#define FTP_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <QThread>
#include <string.h>

#define FTP_PORT 21
#define BUF_LEN 1024
#define USER_PSWD_ERROR "530"
#define NO_SUCH_FILE    "550"

#define MAX_THREAD_NUM 1

#include <iostream>
using namespace std;

class FtpThread : public QThread
{
    Q_OBJECT

public:
    FtpThread();
    FtpThread(char *mHost, char *mUser, char *mCurPath, char *mPasswd, char *mFileName);
    //int connect_port(unsigned short port, const char *ip);
    unsigned short getDataPort();
    int getSize(int sockfd, const char *filename);
    int ftpLogin();
    int ftpSendCommand(const char *cmd, const char *arg);
    int setFileAttr(int offset, long long size, long long downloadsize);
    void stop();
    void contin();

protected:
    void run();

private:
    int mSockFd;
    int mDataFd;
    int mFileFd;
    unsigned short mDataPort;
    char mUser[BUF_LEN];
    char mPasswd[BUF_LEN];
    char mIp[BUF_LEN];
    char mFileName[BUF_LEN];
    char mCurrentPath[BUF_LEN];
    struct sockaddr_in mServerAddr;

    long long mSize;
    long long mDownloadsize;
    int mOffset;

     bool mAlive;
     pthread_mutex_t lock;
     pthread_t tid;

signals:

    void sendData(char*p,long long size);
};

class Ftp
{

};

#endif // FTP_H
