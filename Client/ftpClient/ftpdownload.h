#ifndef FTPDOWNLOAD_H
#define FTPDOWNLOAD_H
#include "ftp.h"
#define MAX_THREAD 10
#define THREADNUM 1

class FtpDownload: public QObject
{
    Q_OBJECT

public:
    FtpDownload();
    FtpDownload(char *mHost, char *mUser, char *mPasswd, char *mCurPath, char *mFileName, long mSize);
private:
    size_t mFileSize;
    int mThreadNum;
    char mIp[BUF_LEN];
    struct sockaddr_in mServerAddr;

    //list<FtpThread* > mThread;
    FtpThread* mThread[MAX_THREAD];

signals:
    void sendData(char*p,int len);

private slots:
    void receiveData(char*p,int len);
};

#endif // FTPDOWNLOAD_H
