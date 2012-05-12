#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QSplitter"
#include "QFileSystemModel"
#include "setting.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initView();
    actionConnect();
    mTaskNum = 0;
    mSelectTask = 0;
    mLogList.clear();
    mLogStr = "";
    mLocalPath = "/tmp";
    mFtp = NULL;

    connect(ui->mDetailed, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
                this, SLOT(operateFile(QTreeWidgetItem*,int)));

    connect(ui->mLocalFileList, SIGNAL(clicked(QModelIndex)),
                this, SLOT(localItemChange(QModelIndex)));

    connect(ui->mLocalFileList, SIGNAL(doubleClicked(QModelIndex)),
                this, SLOT(cdLocalDir(QModelIndex)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::actionConnect()
{
       ui->actionOpen->setShortcut(QKeySequence("Ctrl+o"));
       ui->actionOpen->setStatusTip("open a file");//设置状态栏...
       connect( ui->actionOpen, SIGNAL(activated()) , this, SLOT( open() ) );
}
void MainWindow::open()
 {
     QString fileName = QFileDialog::getOpenFileName(this);
     if (!fileName.isEmpty()) {

     }
 }
void MainWindow::initView()
{

    mModel = new QFileSystemModel;

    ui->mLocalFileList->setModel(mModel);
    ui->mLocalFileList->setAnimated(false);
    ui->mLocalFileList->setIndentation(20);
    ui->mLocalFileList->setSortingEnabled(true);


    ui->mWebFileList->setRootIsDecorated(false);
    ui->mWebFileList->setHeaderLabels(QStringList() << tr("Name") << tr("Size") << tr("Owner") << tr("Group") << tr("Time"));
    ui->mWebFileList->header()->setStretchLastSection(false);

    ui->mDetailed->setRootIsDecorated(false);

    ui->mDetailed->setHeaderLabels(QStringList() << tr("任务") << tr("源文件") << tr("大小") << tr("日期") << tr("目标") << tr("进度"));
    ui->mDetailed->header()->setStretchLastSection(false);
//    ui->mDetailed->header()->setResizeMode(QHeaderView::ResizeToContents );
    ui->mLog->setText(tr("这里是日志"));
    connect(ui->mSiteManager, SIGNAL(clicked()), this, SLOT(onSiteManagerClicked()));

    createComboBox("/home");
    this->mLocalPath = "/home";
    mDialog = new Dialog;

    ui->mDownload->setEnabled(false);
    ui->mDisconnect->setEnabled(false);
    ui->mstart->setEnabled(false);
    ui->mPause->setEnabled(false);
    ui->mUpload->setEnabled(false);
    ui->mReconnect->setEnabled(false);
    ui->mUpDir->setEnabled(false);

    ui->mHost->setText("ftp.qt.nokia.com");
    ui->mUser->setText("anonymous");
    ui->mPassword->setText("123");
    loadConfig();
    mCurrentPath = "/";

}

void MainWindow::onSiteManagerClicked()
{
    ui->mLog->setText(tr("这里是日志2"));
    mDialog->show();
}

void MainWindow::on_mSiteManager_clicked()
{

}

QComboBox *MainWindow::createComboBox(const QString &text)
{
    ui->mComboBox->setEditable(true);
    ui->mComboBox->addItem(text);
    ui->mComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return ui->mComboBox;
}

void MainWindow::browse()
 {
     QString directory = QFileDialog::getExistingDirectory(this,
                                tr("Find Files"), QDir::currentPath());

     if (!directory.isEmpty()) {
         if (ui->mComboBox->findText(directory) == -1)
             ui->mComboBox->addItem(directory);
         ui->mComboBox->setCurrentIndex(ui->mComboBox->findText(directory));
         ui->mLocalFileList->setRootIndex(mModel->setRootPath(ui->mComboBox->currentText()));
     }
 }

void MainWindow::updateComboBox(QComboBox *comboBox)
 {
     if (comboBox->findText(comboBox->currentText()) == -1)
         comboBox->addItem(comboBox->currentText());
 }

void MainWindow::on_mBrowse_clicked()
{
    browse();
}

void MainWindow::on_mComboBox_textChanged(const QString &arg1)
{
    QString directory = arg1;
    if (!arg1.isEmpty()) {
        if (ui->mComboBox->findText(directory) == -1)
            ui->mComboBox->addItem(directory);
        ui->mComboBox->setCurrentIndex(ui->mComboBox->findText(directory));
        ui->mLocalFileList->setRootIndex(mModel->setRootPath(ui->mComboBox->currentText()));
    }

}

void MainWindow::ftpCommandFinished(int commandId, bool error)
{
    if (mFtp->currentCommand() == QFtp::ConnectToHost)
    {
        if (error)
        {
            mLogStr += "connect " + ui->mHost->text() + "error \n";
            ui->mLog->setText(mLogStr);
            QMessageBox::information(this, tr("FTP"),
                                     tr("Unable to connect to the FTP server ")
                                     .arg(ui->mHost->text()));
            return;
        }

        mLogStr += "connect " + ui->mHost->text() + "success \n";
        ui->mLog->setText(mLogStr);
        if (ui->mUser->text() == "" && ui->mPassword->text() == "")
            mFtp->login();
        else
        {
            if (ui->mPassword->text() == "")
            {
                QMessageBox::information(this, tr("FTP"),
                                        tr("Please input passwd"));
                            //connectOrDisconnect();
                return ;
            }
            mFtp->login(ui->mUser->text(), ui->mPassword->text());
        }

        return;
    }

    if (mFtp->currentCommand() == QFtp::Login)
    {
        if ( error != 0)
        {
            QMessageBox::information(this, tr("FTP"),
                                     tr("User or Passwd error"));
            mFtp->abort();
            mFtp->deleteLater();
            return ;
        }

        mLogStr += ui->mUser->text() + "      Login \n";
        ui->mLog->setText(mLogStr);
        ui->mDownload->setEnabled(true);
        mFtp->cd("/");
        ui->mWebDir->setText("/");
        mFtp->list();
    }

    if (mFtp->currentCommand() == QFtp::Put)
    {
         if ( error != 0)
         {
             mLogStr += this->mLocalPath + "  upload failed\n";
         }
         else
         {
             mLogStr += this->mLocalPath + "  upload success\n";
         }
         ui->mLog->setText(mLogStr);
    }
}

void MainWindow::addToFileList(const QUrlInfo &urlInfo)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, urlInfo.name());
    item->setText(1, QString::number(urlInfo.size()));
    item->setText(2, urlInfo.owner());
    item->setText(3, urlInfo.group());
    item->setText(4, urlInfo.lastModified().toString("MMM dd yyyy"));

    QPixmap pixmap(urlInfo.isDir() ? ":/images/folder.png" : ":/images/file.png");
    item->setIcon(0, pixmap);

    mIsDirectory[urlInfo.name()] = urlInfo.isDir();
    ui->mWebFileList->addTopLevelItem(item);
    if (!ui->mWebFileList->currentItem()) {
        ui->mWebFileList->setCurrentItem(ui->mWebFileList->topLevelItem(0));
        ui->mWebFileList->setEnabled(true);
    }
}

void MainWindow::cdToDirectory(QTreeWidgetItem *item, int /*column*/)
{

    QString name = item->text(0);
    //cout << name.toStdString() << endl;

    if (mIsDirectory.value(name)) {
        ui->mWebFileList->clear();
        mIsDirectory.clear();
        mCurrentPath += "/" + name;
        this->mLogStr += "cd " + name + "\n";
        ui->mLog->setText(mLogStr);

        mFtp->cd(mCurrentPath);
        mFtp->list();
        ui->mUpDir->setEnabled(true);
        ui->mWebDir->setText(mCurrentPath);
        return;
    }
}

void MainWindow::cdLocalDir(QModelIndex item)
{
    QModelIndex index = ui->mLocalFileList->currentIndex();
    QString path = mModel->filePath(index);
    cout << path.toStdString() << endl;
}

void MainWindow::cdToServerParent()
{
    ui->mWebFileList->clear();
    this->mIsDirectory.clear();
    this->mCurrentPath =  mCurrentPath.left(mCurrentPath.lastIndexOf('/'));
    if (mCurrentPath.isEmpty()) {
        ui->mUpDir->setEnabled(false);
        this->mFtp->cd("/");
        ui->mWebDir->setText("/");
    } else {
        mFtp->cd(mCurrentPath);
        ui->mWebDir->setText(mCurrentPath);
    }
    mFtp->list();
}

void MainWindow::on_mQuickConnection_clicked()
{
    mFtp = new QFtp(this);

    connect(mFtp, SIGNAL(commandFinished(int,bool)),
            this, SLOT(ftpCommandFinished(int,bool)));
    connect(mFtp, SIGNAL(listInfo(QUrlInfo)),
            this, SLOT(addToFileList(QUrlInfo)));
    connect(ui->mWebFileList, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(cdToDirectory(QTreeWidgetItem*,int)));

    mFtp->connectToHost(ui->mHost->text(), ui->mPort->text().toInt());

    ui->mWebFileList->clear();
    this->mCurrentPath = "";
    this->saveConfig();
    ui->mDisconnect->setEnabled(true);
    ui->mReconnect->setEnabled(true);
}


void MainWindow::on_mDownload_clicked()
{
    QMessageBox msgBox;
    QString downFile, tmpDownFile;
    QModelIndex index = ui->mLocalFileList->currentIndex();
    mLocalPath = mModel->filePath(index);

    mLocalPath = "/tmp";

    long mFileSize = ui->mWebFileList->currentItem()->text(1).toLongLong();
    QString mHost = ui->mHost->text();
    QString mUser = ui->mUser->text();
    QString mPasswd = ui->mPassword->text();
    QString mFileName = ui->mWebFileList->currentItem()->text(0);
    downFile = mLocalPath + "/" + mFileName;
    tmpDownFile = downFile + ".inf";

    if (mLocalPath == "" || QFileInfo(mLocalPath).isFile())
    {
        QMessageBox::information(this, tr("FTP"), tr("Select Dowload Dir"));
        return ;
    }

    if (QFile::exists(tmpDownFile)) {
         msgBox.setText("The File : " + downFile + "  has been download not finish.");
         msgBox.setInformativeText("What are you want to do?");
         QPushButton *continueButton = msgBox.addButton(tr("Continue"), QMessageBox::ActionRole);
         QPushButton *reDowloadButton = msgBox.addButton(tr("reDowload"), QMessageBox::ActionRole);
         QPushButton *CancleButton = msgBox.addButton(tr("Cancle"), QMessageBox::ActionRole);

         msgBox.exec();
         if (msgBox.clickedButton() == continueButton)
         {
              cout << "continue" << endl;
         }
         else if (msgBox.clickedButton() == reDowloadButton)
         {
             QFile::remove(tmpDownFile);
              cout << "reDowLoad" << endl;
         }
         else if (msgBox.clickedButton() == CancleButton)
         {
              cout << "cancle" << endl;
              return ;
         }
    }
    else if (QFile::exists(downFile)) {
        msgBox.setText("The File : " + downFile + "  is exists");
        msgBox.setInformativeText("What are you want to do?");
        QPushButton *reDowloadButton = msgBox.addButton(tr("reDowload"), QMessageBox::ActionRole);
        QPushButton *CancleButton = msgBox.addButton(tr("Cancle"), QMessageBox::ActionRole);

        msgBox.exec();
        if (msgBox.clickedButton() == CancleButton)
        {
             cout << "Cancle" << endl;
             return ;
        }
    }

    QByteArray mQbHost = mHost.toLatin1();
    char *mCharHost = mQbHost.data();

    QByteArray mQbUser = mUser.toLatin1();
    char *mCharUser = mQbUser.data();

    QByteArray mQbPasswd = mPasswd.toLatin1();
    char *mCharPasswd = mQbPasswd.data();

    QByteArray mQbFileName = mFileName.toLatin1();
    char *mCharFileName = mQbFileName.data();

    QByteArray mQbCurrent = this->mCurrentPath.toLatin1();
    char *mCharCurrentPath = mQbCurrent.data();

   QByteArray mQbDownFile = downFile.toLatin1();
    char *mCharDowFIle = mQbDownFile.data();

    FtpDownload *mDowloadFtp = new FtpDownload(mCharHost, mCharUser,mCharPasswd,mCharCurrentPath,mCharFileName, mCharDowFIle, mFileSize);
    this->mFileTask[this->mTaskNum] = mDowloadFtp;

    connect(mFileTask[mTaskNum],SIGNAL(progressChang(int)),this,SLOT(onProgressChang(int)));
    connect(mFileTask[mTaskNum],SIGNAL(stateChange(int, void*)),this,SLOT(onStateChange(int, void *)));
    connect(this,SIGNAL(sendSaveFile()),mFileTask[mTaskNum++],SLOT(doSaveFile()));

    QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间
    QString str = time.toString("yyyy-MM-dd hh:mm:ss");
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, QString::number(mTaskNum));
    item->setText(1, mFileName);
    item->setText(2, QString::number(mFileSize));
    item->setText(3, str);
    item->setText(4, mFileName);
    item->setText(5, "0%");
    ui->mDetailed->addTopLevelItem(item);

}

void MainWindow::onStateChange(int state, void *object)
{
    cout << "Test  state :  " << state << endl;
}

void MainWindow::onProgressChang(int progress)
{
    int i = 0;
    QTreeWidgetItemIterator item(ui->mDetailed);

    while (*item)
    {

        if (mFileTask[i]->mDowloadFlag)
        {
             (*item)->setText(5, QString::number(mFileTask[i]->mProcess) + "%");
        }
        else
        {

            (*item)->setText(5, "pause");
        }

            ++item;
            i++;
    }


    return ;

}

void MainWindow::operateFile(QTreeWidgetItem *item, int column)
{

    mSelectTask = item->text(0).toInt();

    if (this->mFileTask[mSelectTask - 1]->mDowloadFlag)
    {
        ui->mPause->setEnabled(true);
        ui->mstart->setEnabled(false);
    }
    else
    {
        ui->mPause->setEnabled(false);
        ui->mstart->setEnabled(true);
    }
}

void MainWindow::localItemChange(QModelIndex item)
{
    QModelIndex index = ui->mLocalFileList->currentIndex();
    mLocalPath = mModel->filePath(index);
    if (!this->mFtp)
        return;

     QString mFileName = ui->mWebFileList->currentItem()->text(0);

    ui->mUpload->setEnabled(false);
    if (QFileInfo(mLocalPath).isFile() && mFtp != NULL && this->mIsDirectory.value(mFileName))
    {
        ui->mUpload->setEnabled(true);
    }
}

void MainWindow::saveConfig()
{
    QFile tarFile("config.xml");
    if (!tarFile.open(QFile::WriteOnly | QFile::Text | QIODevice::Truncate))
    {
       QMessageBox::warning(this, tr("Warning/ Convert"),
       tr("Cannot open target file %1:\n%2.").arg("config.ini")
         .arg(tarFile.errorString()));
         return;
    }
    QXmlStreamWriter writer(&tarFile);

    writer.setAutoFormatting(true);

    writer.writeStartElement("config");
    writer.writeTextElement("host",ui->mHost->text());
    writer.writeTextElement("user",ui->mUser->text());
    writer.writeTextElement("password",ui->mPassword->text());
    writer.writeEndElement();
    writer.writeEndDocument();
    tarFile.close();
}

void MainWindow::loadConfig()
{
    QFile tarFile("config.xml");
    if (!tarFile.open(QFile::ReadOnly | QFile::Text))
    {
       QMessageBox::warning(this, tr("Warning/ Convert"),
       tr("Cannot open target file %1:\n%2.").arg("config.xml")
         .arg(tarFile.errorString()));
         return;
    }
   QXmlStreamReader xmlreader(&tarFile);
   while (!xmlreader.atEnd()) {
         xmlreader.readNext();
         {
              if (xmlreader.name() == "config")
                    xmlreader.readNext();
                 if(xmlreader.name() == "host")
                 {
                       ui->mHost->setText(xmlreader.readElementText());
                 } else if (xmlreader.name() == "user") {
                      ui->mUser->setText(xmlreader.readElementText());
                 }
                 else if (xmlreader.name() == "password")
                 {
                      ui->mPassword->setText(xmlreader.readElementText());
                 }

         }

   }
   tarFile.close();
}

void MainWindow::on_mSetting_clicked()
{
      Setting *mSetting = new Setting;
      mSetting->show();
}

void MainWindow::on_mstart_clicked()
{
    ui->mPause->setEnabled(true);
    ui->mstart->setEnabled(false);
    this->mFileTask[mSelectTask - 1]->mDowloadFlag = true;

    if (mSelectTask)
        this->mFileTask[mSelectTask - 1]->contin();

}

void MainWindow::on_mPause_clicked()
{
    int i = 0;

    ui->mPause->setEnabled(false);
    ui->mstart->setEnabled(true);

    this->mFileTask[mSelectTask - 1]->mDowloadFlag = false;

    if (mSelectTask)
        this->mFileTask[mSelectTask - 1]->stop();


}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(GloabData::mIsExit == true) {
        QMessageBox::information(this, tr("FTP"),
                                tr("is exit?"));
    }


    for (int i = 0; i < this->mTaskNum; i++)
    {
        if (!mFileTask[i]->mFinishFlag)
        {
            mFileTask[i]->stop();
            emit sendSaveFile();
        }
    }
    cout << "save temfile" << endl;
}


void MainWindow::on_mUpload_clicked()
{

  QFile *file = new QFile( mLocalPath );
  QByteArray mTmp = mLocalPath.toLatin1();
  char *mCharHost = mTmp.data();
  char *p = strrchr(mCharHost, '/');
  QString fileName(p);
  QString serverFileName = mCurrentPath + ui->mWebFileList->currentItem()->text(0) + fileName;

  if ( !file->open(QIODevice::ReadOnly) )
  {
      QMessageBox::critical( this, tr("Upload error"),
      tr("Can't open file '%1' for reading.").arg(mLocalPath) );
      delete file;
      return;
  }

  mLogStr += mLocalPath + "  upload to  " + mCurrentPath + ui->mWebFileList->currentItem()->text(0) + " \n";
  ui->mLog->setText(mLogStr);

  mFtp->put(file, serverFileName);
}

void MainWindow::on_mAbout_clicked()
{
      QMessageBox msgBox;
      msgBox.setText(tr("作者：莫玉成，周均鹏"));
      msgBox.exec();
}

void MainWindow::on_mUpDir_clicked()
{
    this->cdToServerParent();
}

void MainWindow::on_mDisconnect_clicked()
{
    ui->mWebFileList->clear();
    ui->mDisconnect->setEnabled(false);
}

void MainWindow::on_mReconnect_clicked()
{
    mFtp = new QFtp(this);

    connect(mFtp, SIGNAL(commandFinished(int,bool)),
            this, SLOT(ftpCommandFinished(int,bool)));
    connect(mFtp, SIGNAL(listInfo(QUrlInfo)),
            this, SLOT(addToFileList(QUrlInfo)));
    connect(ui->mWebFileList, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(cdToDirectory(QTreeWidgetItem*,int)));

    mFtp->connectToHost(ui->mHost->text(), ui->mPort->text().toInt());

    ui->mWebFileList->clear();
    this->mCurrentPath = "";
    this->saveConfig();
    ui->mDisconnect->setEnabled(true);
    ui->mReconnect->setEnabled(true);
}
