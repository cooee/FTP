#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QFtp>
#include <QUrlInfo>
#include <QHash>
#include <QTreeWidgetItem>
#include <QPixmap>
#include <QMessageBox>
#include"QComboBox"
#include <iostream>
#include "dialog.h"
#include "ftp.h"
#include "ftpdownload.h"
#include "QXmlStreamWriter"
#include "QDebug"
#include "setting.h"
using namespace std;

#define TASK_MAX 20

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
//     void newFile();
     void open();
//     void save();
//     void saveAs();
//     void cut();
//     void copy();
//     void paste();
//     void about();
//     void updateMenus();
//     void updateWindowMenu();
     void ftpCommandFinished(int commandId, bool error);
     void addToFileList(const QUrlInfo &urlInfo);
     void cdToDirectory(QTreeWidgetItem *item, int column);
     void cdLocalDir(QModelIndex item);
     void cdToServerParent();

     void onProgressChang(int progress); //下载进度改变
     void onStateChange(int state, void *object);
     void operateFile(QTreeWidgetItem*,int);
     void localItemChange(QModelIndex);

     void onSiteManagerClicked();

     void on_mSiteManager_clicked();


     void on_mBrowse_clicked();

     void on_mComboBox_textChanged(const QString &arg1);

     void on_mQuickConnection_clicked();

     void on_mDownload_clicked();

     void on_mSetting_clicked();

     void on_mstart_clicked();

     void on_mPause_clicked();

     void on_mUpload_clicked();

     void on_mAbout_clicked();

     void on_mUpDir_clicked();

signals:
    void sendSaveFile();

private:
    Ui::MainWindow *ui;
    void actionConnect();
    void initView();
    QComboBox *createComboBox(const QString &text);
    void browse();
    void updateComboBox(QComboBox *comboBox);

    Dialog *mDialog;
    QFileSystemModel *mModel;
    QDateTime mDatatime;

    int mTaskNum;
    QFtp *mFtp;
    QString mCurrentPath;
    QString mLocalPath;
    QHash<QString, bool> mIsDirectory;
    //list<FtpDownload > mFileTask;
    FtpDownload* mFileTask[TASK_MAX];
    int mSelectTask;
    list<QString> mLogList;
    QString mLogStr;

     void saveConfig();
     void loadConfig();
};

#endif // MAINWINDOW_H
