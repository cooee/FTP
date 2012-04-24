#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QSplitter"
#include "view.h"
#include "QFileSystemModel"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initView();
    actionConnect();
    mTaskNum = 0;
//    QSplitter *h1Splitter = new QSplitter;
//    QSplitter *h2Splitter = new QSplitter;
//    QSplitter *vSplitter = new QSplitter;
//    vSplitter->setOrientation(Qt::Horizontal);
////    QPushButton *b1 = new QPushButton;
////    QPushButton *b2 = new QPushButton;
//    vSplitter->addWidget(h1Splitter);
//    vSplitter->addWidget(h2Splitter);
//    h1Splitter->addWidget(ui->widget_2);
//    h2Splitter->addWidget(ui->widget_3);

//    View *view = new View("Top left view");
////    view->view()->setScene(scene);
////    h1Splitter->addWidget(view);
////    h2Splitter->addWidget(view);
//    ui->horizontalLayout_3->addWidget(vSplitter);

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
//         QMdiSubWindow *existing = findMdiChild(fileName);
//         if (existing) {
//             mdiArea->setActiveSubWindow(existing);
//             return;
//         }

//         MdiChild *child = createMdiChild();
//         if (child->loadFile(fileName)) {
//             statusBar()->showMessage(tr("File loaded"), 2000);
//             child->show();
//         } else {
//             child->close();
//         }
     }
 }
void MainWindow::initView()
{
//    ui->mLocalFileList->setEnabled(false);
//    ui->mLocalFileList->setRootIsDecorated(false);
//    ui->mLocalFileList->setHeaderLabels(QStringList() << tr("Name") << tr("Size") << tr("Owner") << tr("Group") << tr("Time"));
//    ui->mLocalFileList->header()->setStretchLastSection(false);

   model = new QFileSystemModel;
   //model->setRootPath("/usr");
//   model->set
   ui->mLocalFileList->setModel(model);
   ui->mLocalFileList->setAnimated(false);
   ui->mLocalFileList->setIndentation(20);
   ui->mLocalFileList->setSortingEnabled(true);
//   ui->mLocalFileList->setRootIndex();
//   ui->mLocalFileList->show();

    ui->mWebFileList->setRootIsDecorated(false);
    ui->mWebFileList->setHeaderLabels(QStringList() << tr("Name") << tr("Size") << tr("Owner") << tr("Group") << tr("Time"));
    ui->mWebFileList->header()->setStretchLastSection(false);

    ui->mDetailed->setRootIsDecorated(false);
    ui->mDetailed->setHeaderLabels(QStringList() << tr("源文件") << tr("大小") << tr("日期") << tr("目标") << tr("进度"));
    ui->mDetailed->header()->setStretchLastSection(false);

    ui->mLog->setText(tr("这里是日志"));
    connect(ui->mSiteManager, SIGNAL(clicked()), this, SLOT(onSiteManagerClicked()));

    createComboBox("/home");
    mDialog = new Dialog;

    ui->mDownload->setEnabled(false);
    ui->mDisconnect->setEnabled(false);

//    ui->mHost->setText("127.0.0.1");
//    ui->mUser->setText("jason");
//    ui->mPassword->setText("maxwit");

    //ui->mHost->setText("ftp.qt.nokia.com");
    loadConfig();
    mCurrentPath.clear();

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
         ui->mLocalFileList->setRootIndex(model->setRootPath(ui->mComboBox->currentText()));
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
//    ui->mLocalFileList->setRootIndex(model->setRootPath(ui->mComboBox->currentText()));
    QString directory = arg1;
    if (!arg1.isEmpty()) {
        if (ui->mComboBox->findText(directory) == -1)
            ui->mComboBox->addItem(directory);
        ui->mComboBox->setCurrentIndex(ui->mComboBox->findText(directory));
        ui->mLocalFileList->setRootIndex(model->setRootPath(ui->mComboBox->currentText()));
    }

}

void MainWindow::ftpCommandFinished(int commandId, bool error)
{
    if (mFtp->currentCommand() == QFtp::ConnectToHost)
    {
        if (error)
        {
            QMessageBox::information(this, tr("FTP"),
                                     tr("Unable to connect to the FTP server ")
                                     .arg(ui->mHost->text()));
            //connectOrDisconnect();
            return;
        }

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

        ui->mDownload->setEnabled(true);
        mFtp->cd("/");
        mFtp->list();
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

    QPixmap pixmap(urlInfo.isDir() ? ":/images/folder.png" : ":/images/star.png");
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
        //ui->mWebFileList->clearFocus();
        mIsDirectory.clear();
        mCurrentPath += "/" + name;
        mFtp->cd(mCurrentPath);
        mFtp->list();

        //cout << (mCurrentPath.toStdString()) << endl;
        return;
    }
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
}


void MainWindow::on_mDownload_clicked()
{
    long mFileSize = ui->mWebFileList->currentItem()->text(1).toLong();
    QString mHost = ui->mHost->text();
    QString mUser = ui->mUser->text();
    QString mPasswd = ui->mPassword->text();
    //QString mFileName = (mCurrentPath  + "/") + ui->mWebFileList->currentItem()->text(0);
    QString mFileName = ui->mWebFileList->currentItem()->text(0);
    //QString mFileName = "test";

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

    FtpDownload *mDowloadFtp = new FtpDownload(mCharHost, mCharUser,mCharPasswd,mCharCurrentPath,mCharFileName, mFileSize);
    this->mFileTask[this->mTaskNum] = mDowloadFtp;
    //this->mFileTask.push_back(mDowloadFtp);

    connect(mFileTask[mTaskNum++],SIGNAL(sendData(char*,int )),this,SLOT(receiveData(char*,int )));
    //this->mDowloadFtp = new Ftp(mCharHost, mCharUser,mCharPasswd,mCharFileName, long mSize);

    //cout << "filename  " <<  (mFileName).toStdString() << endl;
}

void MainWindow::receiveData(char*p,int len)
{
    ui->mWebDir->setText(QString::number(len));
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

//    QXmlStreamAttribute *attr = new QXmlStreamAttribute;
//    attr.isDefault();
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
//                       qDebug() << "name " << xmlreader.name() << "text " << xmlreader.readElementText();
                       ui->mHost->setText(xmlreader.readElementText());
                 } else if (xmlreader.name() == "user") {
//                       qDebug() << "name " << xmlreader.name() << "text " << xmlreader.readElementText();
                      ui->mUser->setText(xmlreader.readElementText());
                 }
                 else if (xmlreader.name() == "password")
                 {
                      ui->mPassword->setText(xmlreader.readElementText());
//                      qDebug() << "name " << xmlreader.name() << "text " << xmlreader.readElementText();
                 }

         }

   }
   tarFile.close();
}

void MainWindow::on_mSetting_clicked()
{
    saveConfig();
}
