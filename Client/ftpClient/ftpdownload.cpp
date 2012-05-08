#include "ftpdownload.h"

FtpDownload::FtpDownload()
{
}

FtpDownload::FtpDownload(char *mHost, char *mUsername, char *mPwd, char *mCurPath, char *mFile, char *mDowFilename, long long mSize)
{
    int i;
    long long downloadsize, offset, alreadydowsize;
    //int downloadsize, offset;
    int info_fd;
    char tmpname[BUF_LEN];
    //char trans_filename[BUF_LEN];

    mDowloadFlag = true;
    mFinish = 0;
    mFinishFlag = false;
    mThreadTotalSize = 0;
    mState = WAIT;

    strncpy(mSrcFileName, mFile, strlen(mFile) + 1);
    strncpy(mDstFileName, mDowFilename, strlen(mDowFilename) + 1);
    //sockfd = ftp_login();
    //sprintf(trans_filename, "/srv/ftp/%s", filename);
    //size = get_size(sockfd, trans_filename);

    mFileSize = mSize;
    sprintf(tmpname, "%s.inf", this->mDstFileName);

    info_fd = open(tmpname, O_RDWR);
    if(info_fd < 0)
    {
            for (i = 0; i < THREADNUM - 1; i++)
            {
                downloadsize = mSize / THREADNUM;
                offset = i * downloadsize;

                FtpThread *thread = new FtpThread(i, mHost, mUsername, mPwd,mCurPath, mFile, mDstFileName);
                mThread[i] = thread;
                mThread[i]->setFileAttr(offset, mSize, 0, downloadsize);
                cout << "offset" << offset << endl;
                cout << "downloadsize" << downloadsize << endl;

            }

            offset = i * (mSize / THREADNUM);
            downloadsize = mSize - offset;
            FtpThread *thread = new FtpThread(i, mHost, mUsername, mPwd, mCurPath, mFile, mDstFileName);
            mThread[i] = thread;
            mThread[i]->setFileAttr(offset, mSize, 0, downloadsize);

            cout << "offset" << offset << endl;
            cout << "downloadsize" << downloadsize << endl;
            //return ;

    }
    else
    {
            cout << "Continue loading..." << endl;
            cout << tmpname << endl;
            //cout << info_fd << endl;
            //cout << THREADNUM << endl;
            // return ;
            for (i = 0; i < THREADNUM; i++)
            {
                read(info_fd, &(offset), sizeof(long long));
                read(info_fd, &(alreadydowsize), sizeof(long long));
                read(info_fd, &(downloadsize), sizeof(long long));

                FtpThread *thread = new FtpThread(i, mHost, mUsername, mPwd, mCurPath, mFile, mDstFileName);
                mThread[i] = thread;
                mThread[i]->setFileAttr(offset, mSize, alreadydowsize, downloadsize);
                cout << offset << endl;
                cout << "downloadsize" << downloadsize << endl;

                //if (i > 5)
                  //  break;
            }

            close(info_fd);
            //return ;
    }

    for (i = 0; i < THREADNUM ;i++)
    {
        connect(mThread[i],SIGNAL(sendData(int,long long )),this,SLOT(receiveDataCallBack(int,long long )));
        connect(mThread[i],SIGNAL(sendFinish()),this,SLOT(receiveFinish()));

        //connect(this,SIGNAL(sendSave()), mThread[i],SLOT(doSaveFile()));
        connect(this,SIGNAL(sendSave()), mThread[i],SLOT(receiveSave()));
        mThread[i]->start();
    }

    this->mThreadNum = i;
    this->mState = RUNNING;
}

void FtpDownload::receiveDataCallBack(int pid,long long len)
{
    this->mTreadhDowloadSize[pid] = len;

    for (int i = 0; i < THREADNUM; i++)
    {
        this->mThreadTotalSize += this->mTreadhDowloadSize[i];
    }
    mProcess = 100 * mThreadTotalSize / this->mFileSize;
    emit progressChang(mProcess);
    mThreadTotalSize = 0;
   // emit sendData(NULL,len);
}

void FtpDownload::doSaveFile()
{

    int fd;
    char buf[BUF_LEN];

    emit sendSave();
    if (!this->mFinishFlag)
    {

        sprintf(buf, "%s.inf", this->mDstFileName);
        fd = open(buf, O_CREAT | O_RDWR, 0666);

        if (fd < 0)
        {
                perror("open()");
               return ;
        }
        for (int i = 0; i < THREADNUM; i++)
        {
            //emit sendSave();
            write(fd, &(this->mThread[i]->mOffset), sizeof(long long));
             write(fd, &(this->mThread[i]->mAlreadyDowSize), sizeof(long long));
            write(fd, &(this->mThread[i]->mDownloadsize), sizeof(long long));
            cout << "save" << this->mThread[i]->mOffset << endl;
            cout << "save" << this->mThread[i]->mAlreadyDowSize << endl;
            cout << "save" << this->mThread[i]->mDownloadsize << endl;
        }

        close(fd);
    }

    cout << "FtpDownload save" << endl;
    /*
    if (!this->mFinishFlag);
    {
       // for (int i = 0; i < THREADNUM; i++)
        {
           //connect(this,SIGNAL(sendSave()), mThread[i],SLOT(receiveSave()));
           emit sendSave();
        }
    }
    cout << "FtpDownload save" << endl;
    */
}

void FtpDownload::receiveFinish()
{
     cout << "FtpDownload save myc" << endl;
    this->mFinish++;
    if ( this->mFinish == THREADNUM)
    {
        this->mFinishFlag = true;
        this->mState = Downloaded;
        stateChange(mState, NULL);
        cout << "Downloaded :  " << mState << endl;
    }

}

void FtpDownload::stop()
{
    mDowloadFlag = false;

    for (int i = 0; i < THREADNUM ;i++)
    {
        mThread[i]->stop();
    }
    this->mState = PASUE;
    stateChange(mState, NULL);
}

void FtpDownload::contin()
{
    mDowloadFlag = true;

    for (int i = 0; i < THREADNUM ;i++)
    {
        mThread[i]->contin();;
    }
    this->mState = RUNNING;
    stateChange(mState, NULL);
}
