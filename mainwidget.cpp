#include <QStyledItemDelegate>
#include <stdint.h>

#include "mainwidget.h"
#include "ui_mainwidget.h"


MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    setup_models();
    setup_views();
    setup_connections();
}


void MainWidget::setup_models() {
    model_1 = setup_file_system_model(QDir::Dirs | QDir::Files | QDir::NoDot);
    model_2 = setup_file_system_model(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden);
}


QFileSystemModel* MainWidget::setup_file_system_model(QDir::Filters filter) {
    auto model = new QFileSystemModel(this);
    model->setFilter(filter);
    model->setRootPath(QDir::homePath());
    return model;
}


void MainWidget::setup_views() {
    ui->dir_list_1->setModel(model_1);
    ui->dir_list_2->setModel(model_2);

    setup_tree_view(ui->dir_tree_1, model_1);
    setup_tree_view(ui->dir_tree_2, model_2);
}


void MainWidget::setup_tree_view(QTreeView *view, QFileSystemModel *model) {
    view->setModel(model);

    view->setRootIndex(model->index(QDir::homePath()));

    view->setHeaderHidden(true);

    for (uint8_t i = 1; i <= 3; ++i) {
        view->hideColumn(i);
    }
}


void MainWidget::setup_connections() {
    connect(ui->dir_list_1, &QListView::doubleClicked,
            this, &MainWidget::on_fileList_doubleClicked);
    connect(ui->dir_list_2, &QListView::doubleClicked,
            this, &MainWidget::on_fileList_doubleClicked);
    connect(ui->dir_tree_1, &QTreeView::doubleClicked,
            this, &MainWidget::on_fileTree_1_doubleClicked);
    connect(ui->dir_tree_2, &QTreeView::doubleClicked,
            this, &MainWidget::on_fileTree_2_doubleClicked);

    connect(ui->dir_tree_1->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWidget::display_selected_path);
    connect(ui->dir_tree_2->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWidget::display_selected_path);

    connect(ui->dir_list_1->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWidget::display_selected_path);
    connect(ui->dir_list_2->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWidget::display_selected_path);
}


MainWidget::~MainWidget() {
    delete ui;
}

//void contentDifference(QDir &sDir, QDir &dDir, QFileInfoList &diffList) {

//}

//void recursiveContentList(QDir &dir, QFileInfoList &contentList) {

//}


void MainWidget::on_fileList_doubleClicked(const QModelIndex &index)
{
    QListView* listView = qobject_cast<QListView*>(sender());
    if (!listView) return;

    QFileSystemModel* model = qobject_cast<QFileSystemModel*>(listView->model());
    if (!model) return;

    QFileInfo fileInfo = model->fileInfo(index);
    if (fileInfo.fileName() == "..") {
        QDir dir = fileInfo.dir();
        dir.cdUp();
        listView->setRootIndex(model->index(dir.absolutePath()));
    } else if (fileInfo.isDir()) {
        listView->setRootIndex(index);
    }
}

void MainWidget::on_fileTree_1_doubleClicked(const QModelIndex &index) {
    QFileInfo fileInfo = model_1->fileInfo(index);
    if (fileInfo.isDir()) {
        ui->dir_list_1->setRootIndex(model_1->index(fileInfo.absoluteFilePath()));
    }
}

void MainWidget::on_fileTree_2_doubleClicked(const QModelIndex &index) {
    QFileInfo fileInfo = model_2->fileInfo(index);
    if (fileInfo.isDir()) {
        ui->dir_list_2->setRootIndex(model_2->index(fileInfo.absoluteFilePath()));
    }
}


void MainWidget::display_selected_path(const QModelIndex &index) {
    QObject* origin = sender();

    QAbstractItemView* itemView = nullptr;
    if (auto selectionModel = qobject_cast<QItemSelectionModel*>(origin)) {
        itemView = qobject_cast<QAbstractItemView*>(selectionModel->parent());
    } else {
        itemView = qobject_cast<QAbstractItemView*>(origin);
    }

    if (!itemView) return;

    QFileSystemModel* model = qobject_cast<QFileSystemModel*>(itemView->model());
    if (!model) return;

    QFileInfo fileInfo = model->fileInfo(index);

    // For Tree Views
    if (itemView == ui->dir_tree_1) {
        ui->path_1->setText(fileInfo.absoluteFilePath());
    } else if (itemView == ui->dir_tree_2) {
        ui->path_2->setText(fileInfo.absoluteFilePath());
    }
    // For List Views
    else if (itemView == ui->dir_list_1) {
        ui->path_1->setText(fileInfo.absoluteFilePath());
    } else if (itemView == ui->dir_list_2) {
        ui->path_2->setText(fileInfo.absoluteFilePath());
    }
}

