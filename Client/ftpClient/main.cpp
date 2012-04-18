#include <QtGui/QApplication>
#include "mainwindow.h"
#include<QTextCodec>   //Qt的字符编码头文件
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
     QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));//设置中文字体编码
    MainWindow w;

    w.setWindowFlags(w.windowFlags()& ~Qt::WindowMaximizeButtonHint);
     w.show();

   return a.exec();
}
