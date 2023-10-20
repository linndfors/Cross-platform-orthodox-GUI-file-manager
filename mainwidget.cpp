#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent) :QWidget(parent), ui(new Ui::MainWidget) {
    ui->setupUi(this);
    model_1 = new QFileSystemModel(this);
    model_1->setFilter(QDir::QDir::AllEntries);
    model_1->setRootPath("");
    model_2 = new QFileSystemModel(this);
    model_2->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    model_2->setRootPath("");
    ui->fileTree_1->setModel(model_2);
    ui->fileTree_2->setModel(model_2);
    ui->fileList_1->setModel(model_1);
    ui->fileList_2->setModel(model_1);

    connect(ui->fileList_2, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(on_fileList_1_doubleClicked(QModelIndex)));
}

MainWidget::~MainWidget() {
    delete ui;
}

void contentDifference(QDir &sDir, QDir &dDir, QFileInfoList &diffList) {

}

void recursiveContentList(QDir &dir, QFileInfoList &contentList) {

}

void MainWidget::on_fileList_1_doubleClicked(const QModelIndex &index)
{
        QListView* listView = (QListView*)sender();
        QFileInfo fileInfo = model_1->fileInfo(index);
        if (fileInfo.fileName() == "..") {
            QDir dir = fileInfo.dir();
            dir.cdUp();
            listView->setRootIndex(model_1->index(dir.absolutePath()));
        }
        else if (fileInfo.fileName() == ".") {
            listView->setRootIndex(model_1->index(""));
        }
        else if (fileInfo.isDir()) {
            listView ->setRootIndex(index);
        }
}



void MainWidget::on_fileTree_1_doubleClicked(const QModelIndex &index) {
        QFileInfo fileInfo = model_2->fileInfo(index);
        if (fileInfo.isDir()) {
            model_1->setRootPath(fileInfo.absoluteFilePath());
            ui->fileList_1->setRootIndex(model_1->index(fileInfo.absoluteFilePath()));
        }
}

void MainWidget::on_fileTree_2_doubleClicked(const QModelIndex &index) {
        QFileInfo fileInfo = model_2->fileInfo(index);
        if (fileInfo.isDir()) {
            model_1->setRootPath(fileInfo.absoluteFilePath());
            ui->fileList_2->setRootIndex(model_1->index(fileInfo.absoluteFilePath()));
        }
}



