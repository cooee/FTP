#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
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


     void onSiteManagerClicked();

     void on_mSiteManager_clicked();


     void on_mBrowse_clicked();

     void on_mComboBox_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
    void actionConnect();
    void initView();
    QComboBox *createComboBox(const QString &text);
    void browse();
    void updateComboBox(QComboBox *comboBox);

    Dialog *mDialog;
    QFileSystemModel *model;
};

#endif // MAINWINDOW_H
