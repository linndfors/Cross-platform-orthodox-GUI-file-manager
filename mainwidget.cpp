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
#include <QCoreApplication>
#include <QDebug>
#include <zlib.h>
#include <QProcess>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>



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
    renameAction = contextMenu->addAction("Rename");
    copyAction = contextMenu->addAction("Copy");



    connect(newFileAction, &QAction::triggered, this, &MainWidget::createNewFile);
    connect(newDirAction, &QAction::triggered, this, &MainWidget::createNewDirectory);
    connect(deleteAction, &QAction::triggered, this, &MainWidget::deleteSelectedItems);
    connect(renameAction, &QAction::triggered, this, &MainWidget::renameSelectedItem);
    connect(copyAction, &QAction::triggered, this, &MainWidget::copySelectedItems);

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
    model_2 = setup_file_system_model(QDir::Dirs | QDir::Files | QDir::NoDot | QDir::Hidden);
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
    connect(ui->compressButton, &QPushButton::clicked, this, &MainWidget::compressSelectedItems);
    connect(ui->new_file, &QPushButton::clicked, this, &MainWidget::prompt_for_filename);
    connect(ui->new_dir, &QPushButton::clicked, this, &MainWidget::prompt_for_folder_name);
    connect(ui->search, &QPushButton::clicked, this, &MainWidget::search_files);
    connect(ui->copyButton, &QPushButton::clicked, this, &MainWidget::copy);
    connect(ui->moveButton, &QPushButton::clicked, this, &MainWidget::move);
    // Set up the selection mode for the file lists
    ui->dir_list_1->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->dir_list_2->setSelectionMode(QAbstractItemView::ExtendedSelection);

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
        QString newName = baseName + "(1)";
        if (!extension.isEmpty()) {
            newName += "." + extension;
        }
        QString newPath = QDir(dir).absoluteFilePath(newName);

        int copyNumber = 2; // Start with index 2, since we already used index 1
        while (QFile::exists(newPath)) {
            // Construct a new filename with a number in brackets
            newName = baseName + "(" + QString::number(copyNumber++) + ")";
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
        if (reply != QMessageBox::Yes) {
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



bool MainWidget::copy_directory(const QString &sourcePath, const QString &destinationPath) {
    QDir sourceDir(sourcePath);
    if (!sourceDir.exists()) {
        qWarning() << "Source directory does not exist:" << sourcePath;
        return false;
    }

    QString finalDestinationPath = destinationPath;

    QDir destDir(finalDestinationPath);
    if (!destDir.exists()) {
        destDir.mkpath(finalDestinationPath);
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);

    // Create a new folder in the destination path
    QString newFolderName = sourceDir.dirName() + "(1)";
    QString newFolderPath = QDir(finalDestinationPath).absoluteFilePath(newFolderName);
    int copyNumber = 2;
    while (QDir(newFolderPath).exists()) {
        newFolderName = sourceDir.dirName() + "(" + QString::number(copyNumber++) + ")";
        newFolderPath = QDir(finalDestinationPath).absoluteFilePath(newFolderName);
    }
    destDir.mkpath(newFolderPath);

    for (const QFileInfo &fileInfo : fileInfoList) {
        const QString sourceFilePath = fileInfo.absoluteFilePath();
        const QString destFilePath = newFolderPath + QDir::separator() + fileInfo.fileName();

        if (fileInfo.isDir()) {
            // Recursively copy subdirectories
            if (!copy_directory(sourceFilePath, newFolderPath)) {
                return false;
            }
        } else if (fileInfo.isFile()) {
            // Copy files into the new folder
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
        QString defaultDestinationPath = sourcePath;

        QString destinationPath = QInputDialog::getText(this, tr("Copy to"),
                                                        tr("Destination:"), QLineEdit::Normal,
                                                        defaultDestinationPath, &ok);
        if (sourcePath == destinationPath) {
            QMessageBox::warning(this, tr("Error"), tr("Cannot copy to the source."));
            return;
        }
        if (ok && !destinationPath.isEmpty()) {
            QFileInfo sourceInfo(sourcePath);
            QFileInfo destinationInfo(destinationPath);

            if (sourceInfo.isDir()) {
                if (destinationInfo.exists() && !destinationInfo.isDir()) {
                    QMessageBox::warning(this, tr("Error"), tr("Cannot copy a directory to a file."));
                    return;
                }
                qDebug() << "this is directory";
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

void MainWidget::copySelectedItems() {
    if (!contextMenuView) return;

    QAbstractItemView* view = contextMenuView;
    QModelIndexList selectedIndexes = view->selectionModel()->selectedIndexes();

    if (selectedIndexes.isEmpty()) {
        qDebug() << "No selected items.";
        return;
    }

    QFileSystemModel* model = qobject_cast<QFileSystemModel*>(view->model());
    if (!model) {
        qDebug() << "Model is not a QFileSystemModel";
        return;
    }

    for (const QModelIndex& index : selectedIndexes) {
        QFileInfo fileInfo = model->fileInfo(index);
        if (fileInfo.exists()) {
            QString currentDirPath = model->filePath(view->rootIndex());

            // If it's a directory, copy recursively
            if (fileInfo.isDir()) {
                if (!copy_directory(fileInfo.absoluteFilePath(), currentDirPath)) {
                    qWarning() << "Failed to copy directory:" << fileInfo.absoluteFilePath();
                }
            } else if (fileInfo.isFile()) {
                QString destinationPath = QDir(currentDirPath).absoluteFilePath(fileInfo.fileName());
                if (!copy_file(fileInfo.absoluteFilePath(), destinationPath)) {
                    qWarning() << "Failed to copy file:" << fileInfo.absoluteFilePath();
                }
            }
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

void MainWidget::renameSelectedItem() {
    if (!contextMenuView) return;

    QAbstractItemView* view = contextMenuView;
    QModelIndex currentIndex = view->currentIndex();

    if (!currentIndex.isValid()) {
        qDebug() << "No selected item to rename";
        return;
    }

    QFileSystemModel* model = qobject_cast<QFileSystemModel*>(view->model());
    if (!model) return;

    QFileInfo fileInfo = model->fileInfo(currentIndex);
    QString oldFilePath = fileInfo.absoluteFilePath();

    bool ok;
    QString newName = QInputDialog::getText(this, tr("Rename Item"),
                                            tr("New Name:"), QLineEdit::Normal,
                                            fileInfo.fileName(), &ok);
    if (ok && !newName.isEmpty()) {
        QString newFilePath = QDir(fileInfo.absolutePath()).filePath(newName);
        if (!QFile::rename(oldFilePath, newFilePath)) {
            QMessageBox::warning(this, tr("Error"), tr("Failed to rename the item."));
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


void MainWidget::mergeDirectories(QDir& sourceDir, QDir& destDir, bool overwrite) {
    QStringList sourceEntries = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &entry : sourceEntries) {
        QString sourcePath = sourceDir.absoluteFilePath(entry);
        QString destPath = destDir.absoluteFilePath(entry);

        QFileInfo fileInfo(sourcePath);
        if (fileInfo.isDir()) {
            QDir subSourceDir(sourcePath);
            QDir subDestDir(destPath);
            if (!subDestDir.exists()) {
                subDestDir.mkpath(".");
            }
            mergeDirectories(subSourceDir, subDestDir, overwrite);
        } else if (fileInfo.isFile()) {
            if (overwrite || !QFileInfo::exists(destPath)) {
                QFile::remove(destPath);
                QFile::copy(sourcePath, destPath);
            }QFile::remove(sourcePath);
        }
    }

    // Optionally, remove the source directory after merging
     sourceDir.removeRecursively();
}



void MainWidget::handleDirectoryMerge(QDir& sourceDir, QDir& destDir) {
    QStringList sourceFiles = sourceDir.entryList(QDir::Files);
    QStringList identicalFiles;

    for (const QString &file : sourceFiles) {
        if (destDir.exists(file)) {
            identicalFiles.append(file);
        }
    }

    bool overwrite = false;
    if (!identicalFiles.isEmpty()) {
        QMessageBox::StandardButton reply;
        QString question = tr("The directory contains %1 identical files. Do you want to overwrite them?").arg(identicalFiles.size());
        reply = QMessageBox::question(this, tr("Overwrite Files?"), question, QMessageBox::Yes|QMessageBox::No);
        overwrite = (reply == QMessageBox::Yes);
    }

    mergeDirectories(sourceDir, destDir, overwrite);
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
                                                    sourcePath, &ok);
    if (sourcePath == destinationPath) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot move to the source."));
        return;
    }

    if (!ok || destinationPath.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid destination path."));
        return;
    }

    // Get the unique destination name in case the destination exists
    QFileInfo sourceInfo(sourcePath);
    QString baseName = sourceInfo.fileName();

    //    if (QFileInfo::exists(destinationPath + "/" + baseName)) {
    //        destinationPath = getUniqueDestinationName(destinationPath, baseName);
    //    }

    if (sourceInfo.isDir()) {
        QDir sourceDir(sourcePath);
        QString destinationDirPath = destinationPath + "/" + baseName;

        if (!sourceDir.exists()) {
            QMessageBox::warning(this, tr("Error"), tr("Source directory does not exist."));
            return;
        }

        QDir destDir(destinationDirPath);


        if (destDir.exists()) {
            // Handle merging directories
            //            qDebug() << "This is a debug message";
            handleDirectoryMerge(sourceDir, destDir);
            return;
        } else {
            // Normal move operation
//            if (!sourceDir.rename(sourceDir.absolutePath(), destinationPath)) {
//                QMessageBox::warning(this, tr("Error"), tr("Failed to move the directory."));
//                return;
//            }
            if (!copy_directory(sourceDir.absolutePath(), destinationPath)) {
                QMessageBox::warning(this, tr("Error"), tr("Failed to move the directory."));
            } else {
                sourceDir.removeRecursively(); // Remove the original directory after successful copy
            }
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
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("File Exists"),
                                          tr("The file %1 already exists at the destination. Do you want to overwrite it?").arg(baseName),
                                          QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                // User chose to overwrite the file
                // Remove the existing file at the destination
                QFile::remove(destinationPath);
            } else {
                // User chose not to overwrite the file
                // Exit the function, skipping the move operation for this file
                return;
            }
        }

        QFile sourceFile(sourcePath);
        if (!sourceFile.rename(destinationPath)) {
            QMessageBox::warning(this, tr("Error"), tr("Failed to move the file."));
            return;
        }
    }

    QMessageBox::information(this, tr("Success"), tr("Item moved successfully."));
}



class CompressionDialog : public QDialog {
public:
    CompressionDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Compression Settings");

        QVBoxLayout* layout = new QVBoxLayout(this);

        QLabel* formatLabel = new QLabel("Select Archive Format:", this);
        layout->addWidget(formatLabel);

        formatComboBox = new QComboBox(this);
        formatComboBox->addItems(QStringList() << "zip" << "rar" << "tar.xz");
        layout->addWidget(formatComboBox);

        QLabel* baseNameLabel = new QLabel("Base Name for Archive:", this);
        layout->addWidget(baseNameLabel);

        baseNameLineEdit = new QLineEdit(this);
        layout->addWidget(baseNameLineEdit);

        QHBoxLayout* buttonLayout = new QHBoxLayout;

        QPushButton* okButton = new QPushButton("OK", this);
        connect(okButton, &QPushButton::clicked, this, &CompressionDialog::accept);
        buttonLayout->addWidget(okButton);

        QPushButton* cancelButton = new QPushButton("Cancel", this);
        connect(cancelButton, &QPushButton::clicked, this, &CompressionDialog::reject);
        buttonLayout->addWidget(cancelButton);

        layout->addLayout(buttonLayout);
    }

    QComboBox* getFormatComboBox() const {
        return formatComboBox;
    }

    QLineEdit* getBaseNameLineEdit() const {
        return baseNameLineEdit;
    }

private:
    QComboBox* formatComboBox;
    QLineEdit* baseNameLineEdit;
};

void MainWidget::compressSelectedItems() {
    QStringList selectedFiles;

    // Determine the selected items based on the active view
    QAbstractItemView* view = ui->dir_list_1;  // Replace with the correct view variable
    QModelIndexList selectedIndexes = view->selectionModel()->selectedIndexes();

    if (selectedIndexes.isEmpty()) {
        qDebug() << "No selected items.";
        return;
    }

    QFileSystemModel* model = qobject_cast<QFileSystemModel*>(view->model());
    if (!model) {
        qDebug() << "Model is not a QFileSystemModel";
        return;
    }

    // Collect absolute file paths of selected items
    for (const QModelIndex& index : selectedIndexes) {
        QFileInfo fileInfo = model->fileInfo(index);
        QString filePath = fileInfo.filePath();
        if (!fileInfo.isReadable()) {
            qDebug() << "Permission denied: " << filePath;
            QMessageBox::warning(this, "Permission Denied", "You don't have permission to open: " + filePath);
            return;
        }
        if (fileInfo.exists()) {
            selectedFiles << fileInfo.fileName();  // Only add the file name, not the full path
        }
    }

    if (selectedFiles.isEmpty()) {
        qDebug() << "No valid selected files.";
        return;
    }

    // Create and show the compression settings dialog
    CompressionDialog compressionDialog(this);
    if (compressionDialog.exec() != QDialog::Accepted) {
        qDebug() << "User canceled.";
        return;
    }

    // Retrieve values from the dialog
    QString format = compressionDialog.getFormatComboBox()->currentText();
    QString baseName = compressionDialog.getBaseNameLineEdit()->text();

    if (baseName.isEmpty()) {
        qDebug() << "Invalid base name.";
        return;
    }

    QString workingDirectory = model->filePath(view->rootIndex());

    // Determine the unique archive name
    QString archiveName = baseName;
    int archiveNumber = 0;
    QString destinationPath;

    do {
        if (archiveNumber > 0) {
            archiveName = QString("%1(%2)").arg(baseName).arg(archiveNumber);
        }

        destinationPath = workingDirectory + "/" + archiveName + "." + format;
        ++archiveNumber;

    } while (QFile::exists(destinationPath));

    // Use QProcess to run the compression command based on the selected format
    QProcess process;
    process.setWorkingDirectory(workingDirectory);

    if (format == "zip") {
        process.start("zip", QStringList() << "-r" << destinationPath << selectedFiles);
    } else if (format == "rar") {
        process.start("rar", QStringList() << "a" << destinationPath << selectedFiles);
    } else if (format == "tar.xz") {
        process.start("tar", QStringList() << "cJf" << destinationPath << selectedFiles);
    } else {
        qDebug() << "Unsupported archive format.";
        return;
    }

    // Debugging information
    qDebug() << "Executing command:" << process.program() << process.arguments();

    if (process.waitForFinished() && process.exitCode() == 0) {
        QMessageBox::information(this, "Compression Success", "Files compressed successfully.");
    } else {
        qDebug() << "Compression failed. Exit code:" << process.exitCode();
        QMessageBox::warning(this, "Compression Error", "Failed to compress files.");
    }
}









