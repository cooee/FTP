#ifndef FTPDOWNLOAD_H
#define FTPDOWNLOAD_H
#include "ftp.h"
#define MAX_THREAD 10
#define THREADNUM 1

class FtpDownload: public QObject
{
    Q_OBJECT

public:
    friend class MainWindow;
    FtpDownload();
    FtpDownload(char *mHost, char *mUser, char *mPasswd, char *mCurPath, char *mFileName, long long mSize);
    void stop();
    void contin();

private:
    long long mFileSize;
    char mSrcFileName[BUF_LEN];
    char mDstFileName[BUF_LEN];
    int mProcess;
    int mThreadNum;
    bool mDowloadFlag;
    int mFinish;
    char mIp[BUF_LEN];
    struct sockaddr_in mServerAddr;

    //list<FtpThread* > mThread;
    FtpThread* mThread[MAX_THREAD];

signals:
    void sendData(char*p,int len);

private slots:
    void receiveData(char*p,long long len);
};

#endif // FTPDOWNLOAD_H
