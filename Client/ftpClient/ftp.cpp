#include "ftp.h"

static int connect_port(struct sockaddr_in server_addr)
{
        int mSockFd;
        int ret;

        mSockFd = socket(AF_INET, SOCK_STREAM, 0);
        if (mSockFd == -1)
        {
                return mSockFd;
        }

        //server_addr.sin_family = AF_INET;
        //server_addr.sin_port = htons(port);
        //server_addr.sin_addr.s_addr = inet_addr(ip);

        ret = connect(mSockFd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
        if (ret == -1)
        {
                perror("connect()");
                close(mSockFd);

                return ret;
        }

        return mSockFd;
}

FtpThread::FtpThread()
{
}

FtpThread::FtpThread(int pid, char *mHost, char *mUsername, char *mPwd, char *mCurPath, char *mFile, char *mDowFileName)
{
    /*printf("%s\n", mHost);
    printf("%s\n", mUsername);
    printf("%s\n", mPwd);
    printf("%s\n\n", mFile);*/

    mAlive = true;
    mFinish = false;
    mPid = pid;

    strncpy(this->mUser, mUsername, strlen(mUsername) + 1);
    strncpy(this->mPasswd, mPwd, strlen(mPwd) + 1);
    strncpy(mCurrentPath, mCurPath, strlen(mCurPath) + 1);
    strncpy(this->mFileName, mFile, strlen(mFile) + 1);
    strncpy(this->mDownFileName, mDowFileName, strlen(mDowFileName) + 1);
    strncpy(this->mIp, mHost, strlen(mHost) + 1);

    cout << "mUse      " << mUser << endl;
    cout << "mPasswd   " << mPasswd << endl;
    cout << "mFileName " << mFileName << endl;
}
/*
int FtpThread::connect_port(unsigned short port, const char *ip)
{
	int mSockFd, ret;
	struct sockaddr_in server_addr;

	mSockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (mSockFd == -1)
        {
		return mSockFd;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);

	ret = connect(mSockFd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret == -1)
	{
		perror("connect()");
		close(mSockFd);

		return ret;
	}

	return mSockFd;
}*/

int FtpThread::ftpSendCommand(const char *cmd, const char *arg)
{
    int ret = 0, i = 0;
    char buf[BUF_LEN];

    if (arg != NULL)
            snprintf(buf, BUF_LEN, "%s %s\r\n", cmd, arg);
    else
            sprintf(buf, "%s\r\n", cmd);

    ret = send(mSockFd, buf, strlen(buf), 0);
    if (ret < 0)
    {
            perror("send()");
            return ret;
    }

    ret = recv(mSockFd, buf, BUF_LEN - 1, 0);
    if (ret < 0)
    {
            perror("recv()");
            return ret;
    }
    buf[ret] = '\0';

    if (!strncmp(buf, USER_PSWD_ERROR, 3))
    {
        ret = -1;
    }
    else if (!strncmp(buf, NO_SUCH_FILE, 3))
    {
        ret = -1;
    }

    printf("%s\n", buf);
    //printf("tid = %lu, %s\n", pthread_self(), buf);

    return ret;
}

int FtpThread::getSize(int mSockFd, const char *filename)
{
    int size = 0;
    int ret = 0;
    char buf[BUF_LEN];
    char *p;

    sprintf(buf, "SIZE %s\r\n", filename);

    // ret = ftpSendCommand(mSockFd, "SIZE", filename);
    ret = send(mSockFd, buf, strlen(buf), 0);
    if (ret < 0)
    {
            perror("send()");
            return ret;
    }

    ret = recv(mSockFd, buf, BUF_LEN - 1, 0);
    if (ret < 0)
    {
            perror("recv()");
            return ret;
    }
    buf[ret] = '\0';

    p = strrchr(buf, ' ');
    size = atoi(p + 1);

    return size;
}

unsigned short FtpThread::getDataPort()
{
    unsigned short mDataPort;
    int ret;
    char buf[BUF_LEN];
    char *p;

    ret = send(mSockFd, "PASV\r\n", 6, 0);
    if (ret < 0)
    {
            perror("send()");
            return ret;
    }
    ret = recv(mSockFd, buf, BUF_LEN - 1, 0);
    buf[ret] = '\0';

    cout << buf << endl;

    p = strrchr(buf, ',');
    mDataPort = atoi(p + 1);
    *p = '\0';

    p = strrchr(buf, ',');
    mDataPort += atoi(p + 1) << 8;

    return mDataPort;
}

int FtpThread::ftpLogin()
{
    int ret;
    char buf[BUF_LEN];

    mServerAddr.sin_port = htons(FTP_PORT);

    mSockFd = connect_port(mServerAddr);
    if (mSockFd < 0)
    {
        return mSockFd;
    }

    ret = recv(mSockFd, buf, BUF_LEN, 0);
    buf[ret] = '\0';
    cout <<  buf << endl;

    if (this->mUser[0] != 0 && this->mPasswd[0] != '0')
    {
        ret = ftpSendCommand("USER", this->mUser);
        ret = ftpSendCommand("PASS", this->mPasswd);
        ret = ftpSendCommand("SYST", NULL);
        ret = ftpSendCommand("TYPE", "I");
    }
    else
    {
        ret = ftpSendCommand("USER", "anonymous");
        ret = ftpSendCommand("PASS", "123");
        ret = ftpSendCommand("SYST", NULL);
        ret = ftpSendCommand("TYPE", "I");
    }

    return ret;
}

void FtpThread::run()
{
    int ret;
    char buf[BUF_LEN];
    char fileNamePath[BUF_LEN];

    mServerAddr.sin_family = AF_INET;
    if (inet_aton (mIp, &mServerAddr.sin_addr))
    {
        cout << "Ip is error :"  << mIp << endl;
        //return ;
    }
    else
    {
        struct hostent *hp = gethostbyname (mIp);
        if (!hp)
        {
             cout << "gethostbyname error" << endl;
          return ;
        }

        mServerAddr.sin_family = hp->h_addrtype;
        if (hp->h_length > (int) sizeof (mServerAddr.sin_addr))
            hp->h_length = sizeof (mServerAddr.sin_addr);

        memcpy (&mServerAddr.sin_addr, hp->h_addr, hp->h_length);
      }

    ret = ftpLogin();
    if (ret < 0)
    {
        cout << "login failed" << endl;
        return;
    }
    mDataPort = getDataPort();

    mServerAddr.sin_port = htons(mDataPort);
    //cout << mDataPort << endl;
    mDataFd = connect_port(mServerAddr);

    sprintf(buf, "%d", mOffset);
    sprintf(fileNamePath, "%s/%s", this->mCurrentPath, this->mFileName);
    ret = ftpSendCommand("REST", buf);
    //ret = ftpSendCommand("RETR", mFileName);
    //ret = ftpSendCommand("RETR", "/srv/ftp/test");
    ret = ftpSendCommand("RETR", fileNamePath);

    //cout << "aaaaaaa" << FileNamePath << endl;
    // pthread_mutex_lock(&mFattr[i].lock);
    //sprintf(dowFileName, "/tmp/%s", mFileName);
    mFileFd = open(this->mDownFileName, O_CREAT | O_RDWR, 0666);
    lseek(mFileFd, mOffset, SEEK_SET);

    int len;
    long long size;
    mCurrSize = 0;
    size = mDownloadsize;

    while (mCurrSize <= size)
    {
        if (mAlive)
        {
            len = mDownloadsize > BUF_LEN ? BUF_LEN : mDownloadsize;
            ret = recv(mDataFd, buf, len, 0);
            if (ret < 0)
            {
                    perror("recv()");
                    return ;
            }
            else if (ret == 0)
            {
                    break;
            }

            write(mFileFd, buf, ret);

            mOffset += ret;
            mDownloadsize -= ret;
            mCurrSize += ret;
            //mAlreadyDowSize += curr_size;
            //emit sendData(mPid,mOffset);
            emit sendData(mPid,mCurrSize + mAlreadyDowSize);
            // printf("==current size = %lu size = %lu, %d%%\r==", curr_size, size, 100 * (curr_size * 1.0 / size));
            //printf("%d%%\t\r", 100 * curr_size  / size);
        }
    }
    close(mFileFd);
    // pthread_mutex_unlock(&mFattr[i].lock);

    ret = recv(mSockFd, buf, BUF_LEN, 0);
    buf[ret] = '\0';
    cout << buf << endl;

    // ftp_logout(mSockFd);
    close(mDataFd);
    close(mSockFd);

    mFinish = true;
    emit sendFinish();
}

int FtpThread::setFileAttr(long long offset, long long size, long long alreadydowsize, long long downloadsize)
{
    this->mOffset = offset;
    this->mSize = size;
    this->mAlreadyDowSize = alreadydowsize;
    this->mDownloadsize = downloadsize;
}

void FtpThread::stop()
{
    this->mAlive = false;
}

void FtpThread::contin()
{
    this->mAlive = true;
}


void FtpThread::receiveSave()
{

    this->mAlreadyDowSize += this->mCurrSize;
   /* int fd;
    char buf[BUF_LEN];

    if (!this->mFinish)
    {
        cout << "FtpThread save" << endl;

        sprintf(buf, "%s.inf", this->mDownFileName);

        fd = open(buf, O_CREAT | O_RDWR, 0666);

        if (fd < 0)
        {
                perror("open()");
               return ;
        }
        write(fd, &(this->mOffset), sizeof(int));
        write(fd, &(this->mDownloadsize), sizeof(int));
        cout << "save" << this->mOffset << endl;
        cout << "save" << this->mDownloadsize << endl;

        close(fd);
    }*/
}
