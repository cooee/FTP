#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QDebug>
namespace Ui {
    class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private:
    Ui::Dialog *ui;
    QListWidgetItem *mItem;
private slots:
    void on_mAdd_clicked();
    void on_mDelete_clicked();
    void on_mListWidget_itemClicked(QListWidgetItem *item);
    };

#endif // DIALOG_H
