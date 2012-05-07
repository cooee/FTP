#include "setting.h"
#include "ui_setting.h"

Setting::Setting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::setting)
{
    ui->setupUi(this);
}

Setting::~Setting()
{
    delete ui;
}

void Setting::on_mDisplaySite_clicked()
{

}

void Setting::on_mIsExit_clicked()
{
    GloabData::mIsExit = true;
}


void Setting::on_mSelectDir_clicked()
{
    ui->mCurrDir->setChecked(false);
    ui->mDownloadDir->setEnabled(true);

}

void Setting::on_mCurrDir_clicked()
{
    ui->mDownloadDir->setEnabled(false);
    ui->mSelectDir->setChecked(false);
}
