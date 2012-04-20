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
using namespace std;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

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

     void onSiteManagerClicked();

     void on_mSiteManager_clicked();


     void on_mBrowse_clicked();

     void on_mComboBox_textChanged(const QString &arg1);

     void on_mQuickConnection_clicked();

private:
    Ui::MainWindow *ui;
    void actionConnect();
    void initView();
    QComboBox *createComboBox(const QString &text);
    void browse();
    void updateComboBox(QComboBox *comboBox);

    Dialog *mDialog;
    QFileSystemModel *model;

    QFtp *mFtp;
    QString mCurrentPath;
    QHash<QString, bool> mIsDirectory;
};

#endif // MAINWINDOW_H
