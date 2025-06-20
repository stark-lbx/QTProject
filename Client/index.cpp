#include "index.h"
#include "ui_index.h"

Index::Index(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Index)
{
    ui->setupUi(this);
    ui->stackedWidget->setHidden(true);
}

Index &Index::getInstance()
{
    static Index instance;
    return instance;
}

Index::~Index()
{
    delete ui;
}

Friend *Index::getFriend()
{
    return ui->friendPage;
}
File *Index::getFile(){
    return ui->filePage;
}


void Index::on_friend_PB_clicked()
{
    ui->stackedWidget->setHidden(false);
    ui->stackedWidget->setCurrentIndex(1);
}


void Index::on_file_PB_clicked()
{
    ui->stackedWidget->setHidden(false);
    ui->stackedWidget->setCurrentIndex(0);
}

