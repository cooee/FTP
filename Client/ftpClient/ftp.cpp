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

FtpThread::FtpThread(char *mHost, char *mUsername, char *mPwd, char *mCurPath, char *mFile)
{
    printf("%s\n", mHost);
    printf("%s\n", mUsername);
    printf("%s\n", mPwd);
    printf("%s\n\n", mFile);

    strncpy(this->mUser, mUsername, strlen(mUsername) + 1);
    strncpy(this->mPasswd, mPwd, strlen(mPwd) + 1);
    strncpy(mCurrentPath, mCurPath, strlen(mCurPath) + 1);
    strncpy(this->mFileName, mFile, strlen(mFile) + 1);

    printf("%s\n", mUser);
    printf("%s\n", mPasswd);
    printf("%s\n", mFileName);

    mServerAddr.sin_family = AF_INET;
    if (inet_aton (mHost, &mServerAddr.sin_addr))
    {
        memcpy(mIp, mHost, strlen(mHost));
    }
    else
      {
        struct hostent *hp = gethostbyname (mHost);
        if (!hp)
          return ;

        mServerAddr.sin_family = hp->h_addrtype;
        if (hp->h_length > (int) sizeof (mServerAddr.sin_addr))
          hp->h_length = sizeof (mServerAddr.sin_addr);

        memcpy (&mServerAddr.sin_addr, hp->h_addr, hp->h_length);
      }
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

    for (i = 0; cmd[i]; i++)
    {
            buf[i] = cmd[i];
    }


    if (arg !=NULL)
    {
            buf[i++] = ' ';
            int j;
            for (j = 0; arg[j]; j++)
            {
                    buf[i++] = arg[j];
            }
    }

    buf[i++] = '\r';
    buf[i++] = '\n';
    buf[i] = '\0';

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

    printf("%s\n", buf);
    printf("tid = %lu, %s\n", pthread_self(), buf);

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

    ret = ftpSendCommand("USER", this->mUser);
    ret = ftpSendCommand("PASS", this->mPasswd);
    //ret = ftpSendCommand(mSockFd, "SYST", NULL);
    //ret = ftpSendCommand(mSockFd, "TYPE", "I");

    return mSockFd;
}

void FtpThread::run()
{
    int ret;
    char buf[BUF_LEN];
    char FileNamePath[BUF_LEN];

    int num = 0;
    while (0)
    {
        cout << "down run()" << num++ << endl;
        sleep(2);
    }

    mSockFd = ftpLogin();
    mDataPort = getDataPort();

    mServerAddr.sin_port = htons(mDataPort);
    //cout << mDataPort << endl;
    mDataFd = connect_port(mServerAddr);

    sprintf(buf, "%d", mOffset);
    sprintf(FileNamePath, "%s/%s", this->mCurrentPath, this->mFileName);
    ret = ftpSendCommand("REST", buf);
    //ret = ftpSendCommand("RETR", mFileName);
    //ret = ftpSendCommand("RETR", "/srv/ftp/test");
    ret = ftpSendCommand("RETR", FileNamePath);

    //cout << "aaaaaaa" << FileNamePath << endl;
    // pthread_mutex_lock(&mFattr[i].lock);
    mFileFd = open(mFileName, O_CREAT | O_RDWR, 0666);
    lseek(mFileFd, mOffset, SEEK_SET);

    int len;
    size_t curr_size = 0, size;
    size = mDownloadsize;

    while (curr_size <= size)
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
            curr_size += ret;

            emit sendData(NULL,curr_size);
            cout << 100 * curr_size  / size << endl;
            // printf("==current size = %lu size = %lu, %d%%\r==", curr_size, size, 100 * (curr_size * 1.0 / size));
            //printf("%d%%\t\r", 100 * curr_size  / size);
    }
    printf("\n");
    close(mFileFd);
    // pthread_mutex_unlock(&mFattr[i].lock);

    ret = recv(mSockFd, buf, BUF_LEN, 0);
    buf[ret] = '\0';
    cout << buf << endl;

    // ftp_logout(mSockFd);
    close(mDataFd);
    close(mSockFd);
}

int FtpThread::setFileAttr(int offset, size_t size, size_t downloadsize)
{
    this->mOffset = offset;
    this->mSize = size;
    this->mDownloadsize = downloadsize;
}

#if 0
void sig_act(int signum)
{
	int i;
	char buf[BUF_LEN];
	int mFileFd;

	sprintf(buf, "%s.inf", fattr[0].filename);

	mFileFd = open(buf, O_CREAT | O_RDWR, 0666);

	if (mFileFd < 0)
	{
		perror("open()");
		exit(0);
	}

	for (i = 0; i < MAX_THREAD_NUM; i++)
	{
		printf("tid = %lu, offset = %d, download = %d\n", fattr[i].tid, fattr[i].offset, fattr->download_size);
		write(mFileFd, &(fattr[i].offset), sizeof(int));
		write(mFileFd, &(fattr[i].download_size), sizeof(int));
	}

	close(mFileFd);
	exit(1);
}
#endif




