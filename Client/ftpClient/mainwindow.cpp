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
    ui->mLocalFileList->setRootIndex(model->setRootPath(ui->mComboBox->currentText()));
}
