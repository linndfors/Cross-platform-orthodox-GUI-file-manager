#include <QStyledItemDelegate>
#include <QListWidgetItem>
#include <QDesktopServices>
#include <QMessageBox>
#include <QInputDialog>
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
    for (auto listView : {ui->dir_list_1, ui->dir_list_2}) {
        connect(listView, &QListView::doubleClicked,
                this, &MainWidget::on_fileList_doubleClicked);
    }

    for (auto treeView : {ui->dir_tree_1, ui->dir_tree_2}) {
        if (treeView == ui->dir_tree_1) {
            connect(treeView, &QTreeView::doubleClicked,
                    this, &MainWidget::on_fileTree_1_doubleClicked);
        } else {
            connect(treeView, &QTreeView::doubleClicked,
                    this, &MainWidget::on_fileTree_2_doubleClicked);
        }

        connect(treeView->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &MainWidget::display_selected_path);
    }

    for (auto listView : {ui->dir_list_1, ui->dir_list_2}) {
        connect(listView->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &MainWidget::display_selected_path);
    }
    connect(ui->new_file, &QPushButton::clicked, this, &MainWidget::prompt_for_filename);
    connect(ui->new_dir, &QPushButton::clicked, this, &MainWidget::prompt_for_folder_name);
    connect(ui->search, &QPushButton::clicked, this, &MainWidget::search_files);
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

    else if (itemView == ui->dir_list_1) {
        ui->path_1->setText(fileInfo.absoluteFilePath());
    } else if (itemView == ui->dir_list_2) {
        ui->path_2->setText(fileInfo.absoluteFilePath());
    }
}

void MainWidget::prompt_for_filename() {
    bool ok;
    QString filename = QInputDialog::getText(this, tr("Enter Filename"),
                                             tr("Filename:"), QLineEdit::Normal,
                                             "default.txt", &ok);
    if (ok && !filename.isEmpty()) {
        QString currentDirPath = model_1->filePath(ui->dir_list_1->rootIndex());

        QFileInfo fi(filename);
        QString baseName = fi.baseName();
        QString extension = fi.completeSuffix();

        QString filePath = QDir(currentDirPath).filePath(filename);

        int counter = 1;
        while (QFile::exists(filePath)) {
            filePath = QDir(currentDirPath).filePath(QString("%1 (%2).%3").arg(baseName).arg(counter).arg(extension));
            counter++;
        }

        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.close();
            qDebug() << "File created:" << filePath;
        } else {
            qDebug() << "Failed to create file:" << filePath;
        }
    }
}


void MainWidget::prompt_for_folder_name() {
    bool ok;
    QString folderName = QInputDialog::getText(this, tr("Enter Folder Name"),
                                               tr("Folder Name:"), QLineEdit::Normal,
                                               "New Folder", &ok);
    if (ok && !folderName.isEmpty()) {
        QString currentDirPath = model_1->filePath(ui->dir_list_1->rootIndex());

        QString folderPath = QDir(currentDirPath).filePath(folderName);

        QDir dir;

        int counter = 1;
        while (dir.exists(folderPath)) {
            folderPath = QDir(currentDirPath).filePath(QString("%1 (%2)").arg(folderName).arg(counter));
            counter++;
        }

        if (dir.mkdir(folderPath)) {
            qDebug() << "Folder created:" << folderPath;
        } else {
            qDebug() << "Failed to create folder:" << folderPath;
        }
    }
}


void MainWidget::search_files() {
    bool ok;
    QString searchTerm = QInputDialog::getText(this, tr("Search"),
                                               tr("Search for:"), QLineEdit::Normal,
                                               "", &ok);
    if (ok && !searchTerm.isEmpty()) {
        QString currentDirPath = model_1->filePath(ui->dir_list_1->rootIndex());

        QDirIterator it(currentDirPath, QDir::AllEntries, QDirIterator::Subdirectories);

        QStringList results;
        while (it.hasNext()) {
            it.next();  // Move the iterator to the next entry.
            QString currentFilePath = it.filePath();
            if (it.fileName().contains(searchTerm, Qt::CaseInsensitive)) {  // Check if the current filename contains the searchTerm.
                results << currentFilePath;
            }
        }

        if (!results.isEmpty()) {
            QDialog dialog(this);
            QVBoxLayout *layout = new QVBoxLayout(&dialog);
            QListWidget *listWidget = new QListWidget(&dialog);
            layout->addWidget(listWidget);

            for (const QString &result : results) {
                listWidget->addItem(result);
            }

            connect(listWidget, &QListWidget::itemDoubleClicked, [this](QListWidgetItem *item){
                QString path = item->text();
                QFileInfo info(path);
                if (info.isFile()) {
                    path = info.absolutePath();  // if it's a file, get the directory containing it
                }
                // Set the directory in dir_list_1
                ui->dir_list_1->setRootIndex(model_1->index(path));
            });

            dialog.exec();
        } else {
            QMessageBox::information(this, "Search Results", "No matching files or directories found.");
        }
    }
}



