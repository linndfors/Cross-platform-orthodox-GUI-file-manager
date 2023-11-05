#include <QStyledItemDelegate>
#include <QListWidgetItem>
#include <QDesktopServices>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QFileInfoList>
#include <QDebug>
#include <stdint.h>
#include <QMenu>

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

    contextMenu = new QMenu(this);
    newFileAction = contextMenu->addAction("New File");
    newDirAction = contextMenu->addAction("New Directory");
    deleteAction = contextMenu->addAction("Delete");


    connect(newFileAction, &QAction::triggered, this, &MainWidget::createNewFile);
    connect(newDirAction, &QAction::triggered, this, &MainWidget::createNewDirectory);
    connect(deleteAction, &QAction::triggered, this, &MainWidget::deleteSelectedItems);

    for (auto listView : {ui->dir_list_1, ui->dir_list_2}) {
        listView->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(listView, &QListView::customContextMenuRequested, this, &MainWidget::showContextMenu);
    }

    for (auto treeView : {ui->dir_tree_1, ui->dir_tree_2}) {
        treeView->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(treeView, &QTreeView::customContextMenuRequested, this, &MainWidget::showContextMenu);
    }
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
    view->setSelectionBehavior(QAbstractItemView::SelectItems);

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
    connect(ui->copyButton, &QPushButton::clicked, this, &MainWidget::copy);
    connect(ui->moveButton, &QPushButton::clicked, this, &MainWidget::move);
}



MainWidget::~MainWidget() {
    delete ui;
}


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
    } else if (fileInfo.isFile()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
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
            it.next();
            QString currentFilePath = it.filePath();
            if (it.fileName().contains(searchTerm, Qt::CaseInsensitive)) {
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
                    path = info.absolutePath();
                }
                ui->dir_list_1->setRootIndex(model_1->index(path));
            });

            dialog.exec();
        } else {
            QMessageBox::information(this, "Search Results", "No matching files or directories found.");
        }
    }
}



bool MainWidget::copy_file(const QString &sourcePath, const QString &destinationPath) {
    QFile sourceFile(sourcePath);

    if (QFileInfo(sourcePath).absoluteFilePath() == QFileInfo(destinationPath).absoluteFilePath()) {
        QString baseName = QFileInfo(sourcePath).completeBaseName();
        QString extension = QFileInfo(sourcePath).suffix();
        QString dir = QFileInfo(sourcePath).absolutePath();
        QString newName = baseName + " - Copy";
        if (!extension.isEmpty()) {
            newName += "." + extension;
        }
        QString newPath = QDir(dir).absoluteFilePath(newName);

        int copyNumber = 1;
        while (QFile::exists(newPath)) {
            // Construct a new filename with a number
            newName = baseName + " - Copy (" + QString::number(copyNumber++) + ")";
            if (!extension.isEmpty()) {
                newName += "." + extension;
            }
            newPath = QDir(dir).absoluteFilePath(newName);
        }

        return QFile::copy(sourcePath, newPath);
    }

    QFile checkDestFile(destinationPath);

    if (checkDestFile.exists()) {
        auto reply = QMessageBox::question(this, "File Exists",
                                           "The file already exists. Do you want to overwrite it?",
                                           QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::No) {
            return false;
        }
    }

    if (!sourceFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open source file:" << sourcePath;
        return false;
    }

    QFile destinationFile(destinationPath);
    if (!destinationFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open destination file:" << destinationPath;
        return false;
    }

    QByteArray fileData = sourceFile.readAll();
    destinationFile.write(fileData);

    sourceFile.close();
    destinationFile.close();

    return true;
}


bool MainWidget::copy_directory(const QString &sourcePath, const QString &destinationPath, bool isRoot=true) {
    QDir sourceDir(sourcePath);
    if (!sourceDir.exists()) {
        qWarning() << "Source directory does not exist:" << sourcePath;
        return false;
    }

    QString finalDestinationPath;
    if (isRoot) {
        finalDestinationPath = destinationPath + QDir::separator() + sourceDir.dirName();
    } else {
        finalDestinationPath = destinationPath;
    }

    QDir destDir(finalDestinationPath);
    if (!destDir.exists()) {
        destDir.mkpath(finalDestinationPath);
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);

    for (const QFileInfo &fileInfo : fileInfoList) {
        const QString sourceFilePath = fileInfo.absoluteFilePath();
        const QString destFilePath = finalDestinationPath + QDir::separator() + fileInfo.fileName();

        if (fileInfo.isDir()) {
            if (!copy_directory(sourceFilePath, destFilePath, false)) {
                return false;
            }
        } else if (fileInfo.isFile()) {
            QFile::copy(sourceFilePath, destFilePath);
        } else {
            qWarning() << "Unsupported file type:" << sourceFilePath;
            return false;
        }
    }

    return true;
}


void MainWidget::copy() {
    QString defaultSourcePath;
    QModelIndex currentIndex1 = ui->dir_list_1->currentIndex();
    QModelIndex currentIndex2 = ui->dir_list_2->currentIndex();

    if (currentIndex1.isValid()) {
        defaultSourcePath = model_1->filePath(currentIndex1);
    } else if (currentIndex2.isValid()) {
        defaultSourcePath = model_2->filePath(currentIndex2);
    } else {
        defaultSourcePath = model_1->filePath(ui->dir_list_1->rootIndex());
    }

    bool ok;
    QString sourcePath = QInputDialog::getText(this, tr("Copy from"),
                                               tr("Source:"), QLineEdit::Normal,
                                               defaultSourcePath, &ok);
    if (ok && !sourcePath.isEmpty()) {
        QString defaultDestinationPath = "";

        QString destinationPath = QInputDialog::getText(this, tr("Copy to"),
                                                        tr("Destination:"), QLineEdit::Normal,
                                                        defaultDestinationPath, &ok);
        if (ok && !destinationPath.isEmpty()) {
            QFileInfo sourceInfo(sourcePath);
            QFileInfo destinationInfo(destinationPath);

            if (sourceInfo.isDir()) {
                if (destinationInfo.exists() && !destinationInfo.isDir()) {
                    QMessageBox::warning(this, tr("Error"), tr("Cannot copy a directory to a file."));
                    return;
                }
                copy_directory(sourcePath, destinationPath);
            } else if (sourceInfo.isFile()) {
                if (destinationInfo.isDir()) {
                    destinationPath = QDir(destinationPath).absoluteFilePath(sourceInfo.fileName());
                }
                copy_file(sourcePath, destinationPath);
            } else {
                QMessageBox::warning(this, tr("Error"), tr("Invalid source path."));
            }
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Invalid destination path."));
        }
    }
}

void MainWidget::showContextMenu(const QPoint &pos) {
    QObject* senderObject = sender();
    if (!senderObject) return;

    // Store the view that triggered the context menu
    contextMenuView = qobject_cast<QAbstractItemView*>(senderObject);

    if (contextMenuView) {
        contextMenu->popup(contextMenuView->viewport()->mapToGlobal(pos));
    }
}

void MainWidget::createNewFile() {
    bool ok;
    QString filename = QInputDialog::getText(this, tr("Enter Filename"),
                                             tr("Filename:"), QLineEdit::Normal,
                                             "newfile.txt", &ok);
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

void MainWidget::createNewDirectory() {
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

void MainWidget::deleteSelectedItems() {
    if (!contextMenuView) return;

    QAbstractItemView* view = contextMenuView;

    QModelIndexList selectedIndexes = view->selectionModel()->selectedIndexes();

    if (selectedIndexes.isEmpty()) {
        qDebug() << "No selected";
        return;
    }

    int result = QMessageBox::question(this, "Confirm Deletion", "Are you sure you want to delete the selected items?", QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        QFileSystemModel* model = qobject_cast<QFileSystemModel*>(view->model());
        if (!model) return;

        for (const QModelIndex &index : selectedIndexes) {
            QFileInfo fileInfo = model->fileInfo(index);
            if (fileInfo.exists()) {
                if (fileInfo.isDir()) {
                    QDir dir(fileInfo.absoluteFilePath());
                    dir.removeRecursively();
                } else if (fileInfo.isFile()) {
                    QFile file(fileInfo.absoluteFilePath());
                    file.remove();
                }
            }
        }
    }
}

QString MainWidget::getUniqueDestinationName(const QString &destinationPath, const QString &baseName) {
    int copyNumber = 0;
    QString uniqueName;
    do {
        uniqueName = destinationPath + "/" + baseName + (copyNumber > 0 ? QString(" - copy(%1)").arg(copyNumber) : "");
        if (QFileInfo::exists(uniqueName)) {
            ++copyNumber;
        } else {
            break;
        }
    } while (true);

    return uniqueName;
}


void MainWidget::move() {
    QString defaultSourcePath;
    QModelIndex currentIndex1 = ui->dir_list_1->currentIndex();
    QModelIndex currentIndex2 = ui->dir_list_2->currentIndex();

    if (currentIndex1.isValid()) {
        defaultSourcePath = model_1->filePath(currentIndex1);
    } else if (currentIndex2.isValid()) {
        defaultSourcePath = model_2->filePath(currentIndex2);
    } else {
        defaultSourcePath = model_1->filePath(ui->dir_list_1->rootIndex());
    }

    bool ok;
    QString sourcePath = QInputDialog::getText(this, tr("Move from"),
                                               tr("Source:"), QLineEdit::Normal,
                                               defaultSourcePath, &ok);
    if (!ok || sourcePath.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid source path."));
        return;
    }

    QString destinationPath = QInputDialog::getText(this, tr("Move to"),
                                                    tr("Destination:"), QLineEdit::Normal,
                                                    "", &ok);
    if (!ok || destinationPath.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid destination path."));
        return;
    }

    // Get the unique destination name in case the destination exists
    QFileInfo sourceInfo(sourcePath);
    QString baseName = sourceInfo.fileName();

    if (QFileInfo::exists(destinationPath + "/" + baseName)) {
        destinationPath = getUniqueDestinationName(destinationPath, baseName);
    }

    if (sourceInfo.isDir()) {
        QDir sourceDir(sourcePath);
        QString destinationDirPath = destinationPath;

        if (!sourceDir.exists()) {
            QMessageBox::warning(this, tr("Error"), tr("Source directory does not exist."));
            return;
        }

        QDir destDir(destinationDirPath);

        // If the destination is a directory, append the source directory name to the destination path
        if (destDir.exists()) {
            destinationDirPath = destDir.filePath(sourceDir.dirName());
        }

        // Check if the destination with the new directory name exists to get a unique name
        if (QFileInfo::exists(destinationDirPath)) {
            destinationDirPath = getUniqueDestinationName(destinationPath, sourceDir.dirName());
        }

        // Now attempt to move the directory
        if (!sourceDir.rename(sourcePath, destinationDirPath)) {
            QMessageBox::warning(this, tr("Error"), tr("Failed to move the directory."));
            return;
        }
    } else if (sourceInfo.isFile()) {
        // Check if destination is a directory to get the full destination file path
        QDir destDir(destinationPath);
        if (destDir.exists()) {
            // If the destination is a directory, append the base name to the destination path
            destinationPath = destDir.filePath(baseName);
        }

        // Check again if the destination file exists to get a unique name
        if (QFileInfo::exists(destinationPath)) {
            destinationPath = getUniqueDestinationName(destDir.absolutePath(), baseName);
        }

        QFile sourceFile(sourcePath);
        if (!sourceFile.rename(destinationPath)) {
            QMessageBox::warning(this, tr("Error"), tr("Failed to move the file."));
            return;
        }
    }

    QMessageBox::information(this, tr("Success"), tr("Item moved successfully."));
}









