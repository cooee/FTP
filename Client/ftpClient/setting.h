#ifndef SETTING_H
#define SETTING_H

#include <QDialog>
#include "gloabdata.h"
namespace Ui {
    class setting;
}

class Setting : public QDialog
{
    Q_OBJECT

public:
    explicit Setting(QWidget *parent = 0);
    ~Setting();

private slots:
    void on_mDisplaySite_clicked();

    void on_mIsExit_clicked();

    void on_mSelectDir_clicked();

    void on_mCurrDir_clicked();

private:
    Ui::setting *ui;
};

#endif // SETTING_H
