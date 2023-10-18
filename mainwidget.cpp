#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent) :QWidget(parent), ui(new Ui::MainWidget) {
    ui->setupUi(this);
    model = new QFileSystemModel(this);
    model->setFilter(QDir::QDir::AllEntries);
    model->setRootPath("");
    ui->lvBackup->setModel(model);
    ui->lvSource->setModel(model);
    connect(ui->lvBackup, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(on_lvSource_doubleClicked(QModelIndex)));
}

MainWidget::~MainWidget() {
    delete ui;
}

void contentDifference(QDir &sDir, QDir &dDir, QFileInfoList &diffList) {

}

void recursiveContentList(QDir &dir, QFileInfoList &contentList) {

}



void MainWidget::on_lvSource_doubleClicked(const QModelIndex &index)
{
    QListView* listView = (QListView*)sender();
    QFileInfo fileInfo = model->fileInfo(index);
    if (fileInfo.fileName() == "..") {
        QDir dir = fileInfo.dir();
        dir.cdUp();
        listView->setRootIndex(model->index(dir.absolutePath()));
    }
    else if (fileInfo.fileName() == ".") {
        listView->setRootIndex(model->index(""));
    }
    else if (fileInfo.isDir()) {
        listView ->setRootIndex(index);
    }
}
