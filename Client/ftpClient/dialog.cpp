#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_mAdd_clicked()
{
   QListWidgetItem *item = new QListWidgetItem;
   item->setText(ui->mSiteName->text());
    ui->mListWidget->addItem(item);
}

void Dialog::on_mDelete_clicked()
{
    if(mItem != NULL)
    {
        int row = ui->mListWidget->row(mItem);
        qDebug() << row;
//        ui->mListWidget->removeItemWidget(mItem);
        ui->mListWidget->takeItem(row);
    }
}

void Dialog::on_mListWidget_itemClicked(QListWidgetItem *item)
{
    mItem = item;
}
