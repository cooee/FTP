#include "ftpdownload.h"

FtpDownload::FtpDownload()
{
}

FtpDownload::FtpDownload(char *mHost, char *mUsername, char *mPwd, char *mCurPath, char *mFile, long mSize)
{
    int i;
    int downloadsize, offset;
    int info_fd;
    char tmpname[BUF_LEN];
    //char trans_filename[BUF_LEN];

    //sockfd = ftp_login();
    //sprintf(trans_filename, "/srv/ftp/%s", filename);
    //size = get_size(sockfd, trans_filename);

    mFileSize = mSize;
    cout << "mFileSize   " << mFileSize << endl;
    sprintf(tmpname, "%s.inf", mFile);

    info_fd = open(tmpname, O_RDWR);
    if(info_fd < 0)
    {
            for (i = 0; i < THREADNUM - 1; i++)
            {
                downloadsize = mSize / THREADNUM;
                offset = i * downloadsize;

                FtpThread *thread = new FtpThread(mHost, mUsername, mPwd,mCurPath, mFile);
                mThread[i] = thread;
                mThread[i]->setFileAttr(offset, mSize, downloadsize);

            }

            offset = i * (mSize / THREADNUM);
            downloadsize = mSize - offset;
            FtpThread *thread = new FtpThread(mHost, mUsername, mPwd, mCurPath, mFile);
            mThread[i] = thread;
            mThread[i]->setFileAttr(offset, mSize, downloadsize);

    }
    else
    {
            cout << "Continue loading..." << endl;
            for (i = 0; i < THREADNUM; i++)
            {
                read(info_fd, &(offset), sizeof(int));
                read(info_fd, &(downloadsize), sizeof(int));

                FtpThread *thread = new FtpThread(mHost, mUsername, mPwd, mCurPath, mFile);
                mThread[i] = thread;
                mThread[i]->setFileAttr(offset, mSize, downloadsize);
            }

            close(info_fd);
    }

    for (i = 0; i < THREADNUM ;i++)
    {
        connect(mThread[i],SIGNAL(sendData(char*,int )),this,SLOT(receiveData(char*,int )));
        mThread[i]->start();
    }

    this->mThreadNum = i;
}

void FtpDownload::receiveData(char*p,int len)
{
   emit sendData(NULL,100 * len / this->mFileSize);
   // emit sendData(NULL,len);
}
