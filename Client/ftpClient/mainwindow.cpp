#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QSplitter"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    actionConnect();
//    QHBoxLayout *labelLayout = new QHBoxLayout;
//    labelLayout->addWidget(ui->mAbout);
//    QHBoxLayout *rotateSliderLayout = new QHBoxLayout;
//    rotateSliderLayout->addWidget(ui->mReconnect);
//    QGridLayout *topLayout = new QGridLayout;
//    topLayout->addWidget(ui->centralWidget);
//    topLayout->addLayout(labelLayout, 0, 0);
//    topLayout->addLayout(rotateSliderLayout,1, 1);
//    setLayout(topLayout);
    QSplitter *h1Splitter = new QSplitter;
    QSplitter *h2Splitter = new QSplitter;
    QPushButton * button = new QPushButton(this);
    QPushButton * button1 = new QPushButton(this);
    button->setText("sss");
     button1->setText("sss");
     QSplitter *vSplitter = new QSplitter;
     vSplitter->setOrientation(Qt::Horizontal);
     h1Splitter->addWidget(button);
     h2Splitter->addWidget(button1);
     vSplitter->addWidget(h1Splitter);
     vSplitter->addWidget(h2Splitter);
     ui->gridLayout_4->addWidget(vSplitter);

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
//       ui->label->setPixmap(QPixmap(":/images/dir.png"));
//       ui->label->show();
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

