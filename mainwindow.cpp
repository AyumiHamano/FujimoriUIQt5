#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    //connect
    connect(ui->pushButton_cancelROId, SIGNAL(clicked()), this, SLOT(keepLoop()));
    connect(ui->pushButton_cancelROIa, SIGNAL(clicked()), this, SLOT(keepLoop()));
    connect(ui->pushButton_cancelROIs, SIGNAL(clicked()), this, SLOT(keepLoop()));
    connect(ui->pushButton_decideROId, SIGNAL(clicked()), this, SLOT(killLoop()));
    connect(ui->pushButton_decideROIa, SIGNAL(clicked()), this, SLOT(killLoop()));
    connect(ui->pushButton_decideROIs, SIGNAL(clicked()), this, SLOT(killLoop()));

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabCheck(int)));

    //initialization
    this->dialog = new QProgressDialog(this);
    this->dialog->setAutoReset(true);
    this->dialog->setAutoClose(true);

}

MainWindow::~MainWindow()
{
    delete ui;
}
