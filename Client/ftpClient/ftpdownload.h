#ifndef FTPDOWNLOAD_H
#define FTPDOWNLOAD_H
#include "ftp.h"
#define MAX_THREAD 10
#define THREADNUM 2

enum state{
    WAIT,
    PASUE,
    RUNNING,
    FAILED,
    Downloaded
};

class FtpDownload: public QObject
{
    Q_OBJECT

public:
    friend class MainWindow;
    FtpDownload();
    FtpDownload(char *mHost, char *mUser, char *mPasswd, char *mCurPath, char *mFileName, char *dstFilename, long long mSize);
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
    bool mFinishFlag;
    int mState;
    char mIp[BUF_LEN];
    long long mTreadhDowloadSize[BUF_LEN];
    long long mThreadTotalSize;
    struct sockaddr_in mServerAddr;
    FtpThread* mThread[MAX_THREAD];

signals:
    void progressChang(int progress);
    void sendSave();
    void stateChange(int state, void *object);

private slots:
    void receiveDataCallBack(int pid,long long len);
    void doSaveFile();
    void receiveFinish();
};

#endif // FTPDOWNLOAD_H
