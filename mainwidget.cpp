#include <QStyledItemDelegate>
#include <QListWidgetItem>
#include <QDesktopServices>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QLineEdit>
#include <QFileInfoList>
#include <QDebug>
#include <QMenu>
#include <QProcess>
#include <QRadioButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>


#include "mainwidget.h"
#include "ui_mainwidget.h"


MainWidget::MainWidget(QWidget* parent)
        : QWidget(parent),
          ui(new Ui::MainWidget) {
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
    sortAction = contextMenu->addAction("Sort by");



    connect(newFileAction, &QAction::triggered, this, &MainWidget::createNewFile);
    connect(newDirAction, &QAction::triggered, this, &MainWidget::createNewDirectory);
    connect(deleteAction, &QAction::triggered, this, &MainWidget::deleteSelectedItems);
    connect(renameAction, &QAction::triggered, this, &MainWidget::renameSelectedItem);
    connect(copyAction, &QAction::triggered, this, &MainWidget::copySelectedItems);
    connect(sortAction, &QAction::triggered, this, &MainWidget::showSortDialog);


    ui->dir_list_1->setDragDropMode(QAbstractItemView::InternalMove);
    ui->dir_list_2->setDragDropMode(QAbstractItemView::InternalMove);
    ui->dir_tree_1->setDragDropMode(QAbstractItemView::InternalMove);
    ui->dir_tree_2->setDragDropMode(QAbstractItemView::InternalMove);

    for (auto listView: {ui->dir_list_1, ui->dir_list_2}) {
        listView->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(listView, &QListView::customContextMenuRequested, this, &MainWidget::showContextMenu);
        listView->setDragEnabled(true);
        listView->setAcceptDrops(true);
        listView->setDropIndicatorShown(true);
        listView->setDragDropMode(QAbstractItemView::InternalMove);
    }

    for (auto treeView: {ui->dir_tree_1, ui->dir_tree_2}) {
        treeView->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(treeView, &QTreeView::customContextMenuRequested, this, &MainWidget::showContextMenu);
        treeView->setDragEnabled(true);
        treeView->setAcceptDrops(true);
        treeView->setDropIndicatorShown(true);
        treeView->setDragDropMode(QAbstractItemView::InternalMove);
    }
    ui->dir_list_1->installEventFilter(this);
    ui->dir_list_2->installEventFilter(this);
    ui->dir_tree_1->installEventFilter(this);
    ui->dir_tree_2->installEventFilter(this);
}


void MainWidget::setup_models() {
    model_1 = setup_file_system_model(QDir::Dirs | QDir::Files | QDir::NoDot);
    model_2 = setup_file_system_model(QDir::Dirs | QDir::Files | QDir::NoDot);
}

QFileSystemModel* MainWidget::setup_file_system_model(QDir::Filters filter) {
    auto model = new QFileSystemModel(this);
    model->setFilter(filter);
    model->sort(0, Qt::AscendingOrder);
    model->setRootPath(QDir::homePath());
    return model;
}

void MainWidget::setup_views() {
    setup_view(ui->dir_list_1, model_1);
    setup_view(ui->dir_list_2, model_2);
    setup_tree_view(ui->dir_tree_1, model_1);
    setup_tree_view(ui->dir_tree_2, model_2);
}

void MainWidget::setup_view(QAbstractItemView* view, QFileSystemModel* model) {
    view->setModel(model);
    view->setRootIndex(model->index(QDir::homePath()));
}


void MainWidget::setup_tree_view(QTreeView* view, QFileSystemModel* model) {
    view->setModel(model);
    view->setSelectionBehavior(QAbstractItemView::SelectItems);

    view->setRootIndex(model->index(QDir::homePath()));

    view->setHeaderHidden(true);

    for (uint8_t i = 1; i <= 3; ++i) {
        view->hideColumn(i);
    }
}

void MainWidget::setLightMode() {
    qApp->setStyleSheet(
        "QWidget {"
        "   background-color: #ffffff;"
        "   color: #000000;"
        "}"
        );
}

void MainWidget::setDarkMode() {
    qApp->setStyleSheet(
        "QWidget {"
        "   background-color: #2b2b2b;"
        "   color: #ffffff;"
        "}"
        );
}

void MainWidget::toggleMode() {
    static bool isDarkMode = false;
    isDarkMode = !isDarkMode;

    if (isDarkMode) {
        setDarkMode();
    } else {
        setLightMode();
    }
}

void MainWidget::setup_connections() {
    for (auto listView: {ui->dir_list_1, ui->dir_list_2}) {
        connect(listView, &QListView::doubleClicked,
                this, &MainWidget::on_fileList_doubleClicked);
    }

    for (auto treeView: {ui->dir_tree_1, ui->dir_tree_2}) {
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

    for (auto listView: {ui->dir_list_1, ui->dir_list_2}) {
        connect(listView->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &MainWidget::display_selected_path);
    }
    connect(ui->compressButton, &QPushButton::clicked, this, &MainWidget::compressSelectedItems);
    connect(ui->new_file, &QPushButton::clicked, this, &MainWidget::createNewFile);
    connect(ui->new_dir, &QPushButton::clicked, this, &MainWidget::createNewDirectory);
    connect(ui->search, &QPushButton::clicked, this, &MainWidget::search_files);
    connect(ui->copyButton, &QPushButton::clicked, this, &MainWidget::copy);
    connect(ui->moveButton, &QPushButton::clicked, this, &MainWidget::move);
    connect(ui->compareButton, &QPushButton::clicked, this, &MainWidget::compareDirectories);
    connect(ui->modeButton, &QPushButton::clicked, this, &MainWidget::toggleMode);
    ui->dir_list_1->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->dir_list_2->setSelectionMode(QAbstractItemView::ExtendedSelection);

}


MainWidget::~MainWidget() {
    delete ui;
}


void MainWidget::on_fileList_doubleClicked(const QModelIndex& index) {
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

void MainWidget::on_fileTree_1_doubleClicked(const QModelIndex& index) {
    QFileInfo fileInfo = model_1->fileInfo(index);
    if (fileInfo.isDir()) {
        ui->dir_list_1->setRootIndex(model_1->index(fileInfo.absoluteFilePath()));
    }
}

void MainWidget::on_fileTree_2_doubleClicked(const QModelIndex& index) {
    QFileInfo fileInfo = model_2->fileInfo(index);
    if (fileInfo.isDir()) {
        ui->dir_list_2->setRootIndex(model_2->index(fileInfo.absoluteFilePath()));
    }
}


void MainWidget::display_selected_path(const QModelIndex& index) {
    QObject* origin = sender();

    QAbstractItemView* itemView;
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
    } else if (itemView == ui->dir_list_1) {
        ui->path_1->setText(fileInfo.absoluteFilePath());
    } else if (itemView == ui->dir_list_2) {
        ui->path_2->setText(fileInfo.absoluteFilePath());
    }
}

class SortDialog : public QDialog {
public:
    SortDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Sort Options");

        QVBoxLayout* layout = new QVBoxLayout(this);

        QGroupBox* groupBox = new QGroupBox("Sort By", this);
        QVBoxLayout* groupBoxLayout = new QVBoxLayout(groupBox);

        nameRadioButton = new QRadioButton("Name", this);
        sizeRadioButton = new QRadioButton("Size", this);
        dateRadioButton = new QRadioButton("Date", this);

        groupBoxLayout->addWidget(nameRadioButton);
        groupBoxLayout->addWidget(sizeRadioButton);
        groupBoxLayout->addWidget(dateRadioButton);

        groupBox->setLayout(groupBoxLayout);

        layout->addWidget(groupBox);

        QHBoxLayout* buttonLayout = new QHBoxLayout;

        QPushButton* okButton = new QPushButton("OK", this);
        connect(okButton, &QPushButton::clicked, this, &SortDialog::accept);
        buttonLayout->addWidget(okButton);

        QPushButton* cancelButton = new QPushButton("Cancel", this);
        connect(cancelButton, &QPushButton::clicked, this, &SortDialog::reject);
        buttonLayout->addWidget(cancelButton);

        layout->addLayout(buttonLayout);
    }

    QRadioButton* getNameRadioButton() const {
        return nameRadioButton;
    }

    QRadioButton* getSizeRadioButton() const {
        return sizeRadioButton;
    }

    QRadioButton* getDateRadioButton() const {
        return dateRadioButton;
    }

    int getSortOption() const {
        if (nameRadioButton->isChecked()) return SortByName;
        if (sizeRadioButton->isChecked()) return SortBySize;
        if (dateRadioButton->isChecked()) return SortByDate;
        return SortByName;  // Default sorting option
    }

private:
    QRadioButton* nameRadioButton;
    QRadioButton* sizeRadioButton;
    QRadioButton* dateRadioButton;

public:
    enum SortOption {
        SortByName,
        SortBySize,
        SortByDate
    };
};
void MainWidget::showSortDialog() {
    // Create an instance of the custom sorting dialog
    SortDialog sortDialog(this);

    // Show the dialog and get the result
    int result = sortDialog.exec();

    // Check if the user clicked OK
    if (result == QDialog::Accepted) {
        // Get the chosen sorting option
        SortDialog::SortOption sortOption = static_cast<SortDialog::SortOption>(sortDialog.getSortOption());

        // Define column indices (consider using constants or enums)
        const int nameColumn = 0;
        const int sizeColumn = 1;
        const int dateColumn = 2;

        // Use the chosen sorting option to sort items
        switch (sortOption) {
        case SortDialog::SortByName:
            model_1->sort(nameColumn, Qt::AscendingOrder);
            model_2->sort(nameColumn, Qt::AscendingOrder);
            break;
        case SortDialog::SortBySize:
            model_1->sort(sizeColumn, Qt::AscendingOrder);
            model_2->sort(sizeColumn, Qt::AscendingOrder);
            break;
        case SortDialog::SortByDate:
            model_1->sort(dateColumn, Qt::AscendingOrder);
            model_2->sort(dateColumn, Qt::AscendingOrder);
            break;
        }
    }
}

void MainWidget::compareDirectories() {
    QString currentDirPath = model_1->filePath(ui->dir_list_1->rootIndex());
    bool ok;
    QString dirPath1 = QInputDialog::getText(this, tr("Enter Source Directory Path"), tr("Source Path:"), QLineEdit::Normal, currentDirPath, &ok);

    if (!ok || dirPath1.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid source path."));
        return;
    }
    QString dirPath2 = QInputDialog::getText(this, tr("Enter Destination Directory Path"), tr("Destination Path:"), QLineEdit::Normal, currentDirPath, &ok);

    if (!ok || dirPath2.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid destination path."));
        return;
    }
    QDir sourceDir(dirPath1);
    QDir destDir(dirPath2);

    if (!sourceDir.exists() || !destDir.exists()) {
        QMessageBox::warning(this, tr("Invalid Directories"), tr("Please enter valid directory paths to compare."));
        return;
    }
    QStringList similarFiles;
    QStringList differentFiles;

    QStringList sourceFiles = sourceDir.entryList(QDir::Files);
    QStringList destFiles = destDir.entryList(QDir::Files);

    for (const QString& file : sourceFiles) {
        if (destFiles.contains(file)) {
            similarFiles.append(file);
        } else {
            differentFiles.append(file);
        }
    }

    for (const QString& file : destFiles) {
        if (!sourceFiles.contains(file)) {
            differentFiles.append(file);
        }
    }
    QString resultMessage = QString("Similar Files:\n%1\n\nDifferent Files:\n%2")
                                .arg(similarFiles.join("\n"))
                                .arg(differentFiles.join("\n"));

    QMessageBox::information(this, tr("Directory Comparison Result"), resultMessage);
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
            QVBoxLayout* layout = new QVBoxLayout(&dialog);
            QListWidget* listWidget = new QListWidget(&dialog);
            layout->addWidget(listWidget);

            for (const QString& result: results) {
                listWidget->addItem(result);
            }

            connect(listWidget, &QListWidget::itemDoubleClicked, [this](QListWidgetItem* item) {
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


bool MainWidget::copy_file(const QString& sourcePath, const QString& destinationPath) {
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

        int copyNumber = 2;
        while (QFile::exists(newPath)) {
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


bool MainWidget::copy_directory(const QString& sourcePath, const QString& destinationPath) {
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

    QString newFolderName = sourceDir.dirName();
    QString newFolderPath = QDir(finalDestinationPath).absoluteFilePath(newFolderName);
    int copyNumber = 1;
    while (QDir(newFolderPath).exists()) {
        newFolderName = sourceDir.dirName() + "(" + QString::number(copyNumber++) + ")";
        newFolderPath = QDir(finalDestinationPath).absoluteFilePath(newFolderName);
    }
    destDir.mkpath(newFolderPath);

    for (const QFileInfo& fileInfo: fileInfoList) {
        const QString sourceFilePath = fileInfo.absoluteFilePath();
        const QString destFilePath = newFolderPath + QDir::separator() + fileInfo.fileName();

        if (fileInfo.isDir()) {
            if (!copy_directory(sourceFilePath, newFolderPath)) {
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

    for (const QModelIndex& index: selectedIndexes) {
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


void MainWidget::showContextMenu(const QPoint& pos) {
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

    int result = QMessageBox::question(this, "Confirm Deletion", "Are you sure you want to delete the selected items?",
                                       QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        QFileSystemModel* model = qobject_cast<QFileSystemModel*>(view->model());
        if (!model) return;

        for (const QModelIndex& index: selectedIndexes) {
            QFileInfo fileInfo = model->fileInfo(index);

            if (fileInfo.isSymLink() && fileInfo.path() == QDir::rootPath()) {
                QString linkTarget = fileInfo.symLinkTarget();

                QMessageBox::warning(
                        this,
                        "Warning",
                        QString("Deletion of the symbolic link to '%1' in the root directory is not allowed.").arg(
                                linkTarget),
                        QMessageBox::Ok
                );
                continue;

            }

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


//void MainWidget::mergeDirectories(QDir& sourceDir, QDir& destDir, bool overwrite) {
//    QStringList sourceEntries = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

//    for (const QString& entry: sourceEntries) {
//        QString sourcePath = sourceDir.absoluteFilePath(entry);
//        QString destPath = destDir.absoluteFilePath(entry);

//        QFileInfo fileInfo(sourcePath);
//        if (fileInfo.isDir()) {
//            QDir subSourceDir(sourcePath);
//            QDir subDestDir(destPath);
//            if (!subDestDir.exists()) {
//                subDestDir.mkpath(".");
//            }
//            mergeDirectories(subSourceDir, subDestDir, overwrite);
//        } else if (fileInfo.isFile()) {
//            if (overwrite || !QFileInfo::exists(destPath)) {
//                QFile::remove(destPath);
//                QFile::copy(sourcePath, destPath);
//            }
//            QFile::remove(sourcePath);
//        }
//    }

//    sourceDir.removeRecursively();
//}


//void MainWidget::handleDirectoryMerge(QDir& sourceDir, QDir& destDir) {
//    QStringList sourceFiles = sourceDir.entryList(QDir::Files);
//    QStringList identicalFiles;

//    for (const QString& file: sourceFiles) {
//        if (destDir.exists(file)) {
//            identicalFiles.append(file);
//        }
//    }

//    bool overwrite = false;
//    if (!identicalFiles.isEmpty()) {
//        QMessageBox::StandardButton reply;
//        QString question = tr("The directory contains %1 identical files. Do you want to overwrite them?").arg(
//                identicalFiles.size());
//        reply = QMessageBox::question(this, tr("Overwrite Files?"), question, QMessageBox::Yes | QMessageBox::No);
//        overwrite = (reply == QMessageBox::Yes);
//    }

//    mergeDirectories(sourceDir, destDir, overwrite);
//}

bool MainWidget::askUserForOverwrite(const QString& filePath) {
    QMessageBox::StandardButton reply;
    QString question = tr("The file %1 already exists. Do you want to overwrite it?").arg(filePath);
    reply = QMessageBox::question(this, tr("Overwrite File?"), question, QMessageBox::Yes | QMessageBox::No);
    return (reply == QMessageBox::Yes);
}

void MainWidget::mergeDirectories(QDir& sourceDir, QDir& destDir) {
    QStringList sourceEntries = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString& entry: sourceEntries) {
        QString sourcePath = sourceDir.absoluteFilePath(entry);
        QString destPath = destDir.absoluteFilePath(entry);

        QFileInfo fileInfo(sourcePath);
        if (fileInfo.isDir()) {
            QDir subSourceDir(sourcePath);
            QDir subDestDir(destPath);
            if (!subDestDir.exists()) {
                subDestDir.mkpath(".");
            }
            mergeDirectories(subSourceDir, subDestDir);
        } else if (fileInfo.isFile()) {
            if (QFileInfo::exists(destPath)) {
                bool overwrite = askUserForOverwrite(destPath); // This function should be implemented to show a dialog box
                if (overwrite) {
                    QFile::remove(destPath);
                    QFile::copy(sourcePath, destPath);
                }
            } else {
                QFile::copy(sourcePath, destPath);
            }
            QFile::remove(sourcePath);
        }
    }

    sourceDir.removeRecursively();
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

    QFileInfo sourceInfo(sourcePath);
    QString baseName = sourceInfo.fileName();

    if (sourceInfo.isDir() && sourceInfo.isWritable()) {
        QDir sourceDir(sourcePath);
        QString destinationDirPath = destinationPath + "/" + baseName;

        if (!sourceDir.exists()) {
            QMessageBox::warning(this, tr("Error"), tr("Source directory does not exist."));
            return;
        }

        QDir destDir(destinationDirPath);


        if (destDir.exists()) {
//            handleDirectoryMerge(sourceDir, destDir);
            mergeDirectories(sourceDir, destDir);
            return;
        } else {
            if (!copy_directory(sourceDir.absolutePath(), destinationPath)) {
                QMessageBox::warning(this, tr("Error"), tr("Failed to move the directory."));
            } else {
                sourceDir.removeRecursively();
            }
        }
    } else if (sourceInfo.isFile() && sourceInfo.isWritable()) {
        QDir destDir(destinationPath);
        if (destDir.exists()) {
            destinationPath = destDir.filePath(baseName);
        }

        if (QFileInfo::exists(destinationPath)) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("File Exists"),
                                          tr("The file %1 already exists at the destination. Do you want to overwrite it?").arg(
                                                  baseName),
                                          QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                QFile::remove(destinationPath);
            } else {
                return;
            }
        }

        QFile sourceFile(sourcePath);
        if (!sourceFile.rename(destinationPath)) {
            QMessageBox::warning(this, tr("Error"), tr("Failed to move the file."));
            return;
        }
    } else {
        return;
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

    QAbstractItemView* view = ui->dir_list_1;
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
    for (const QModelIndex& index: selectedIndexes) {
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



QString MainWidget::determineDestinationPath(QObject *dropTarget, const QPoint &dropPosition) {
    QAbstractItemView *view = qobject_cast<QAbstractItemView*>(dropTarget);
    if (view) {
        QModelIndex index = view->indexAt(dropPosition);
        QFileSystemModel *model = qobject_cast<QFileSystemModel*>(view->model());
        if (model) {
            QString path;
            if (index.isValid()) {
                // If the index is valid (dropped on an item), get the item's path
                path = model->filePath(index);
            } else {
                // If the index is invalid (dropped on empty space), use the root path of the view
                path = model->filePath(view->rootIndex());
            }
            if (QFileInfo(path).isDir()) {
                return path; // Return the directory path
            } else {
                return QFileInfo(path).path(); // Return the parent directory of the file
            }
        }
    }
    return QString(); // Return empty string if destination path cannot be determined
}


void MainWidget::moveItem(QString &sourcePath, QString &destinationPath) {
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

    if (sourcePath.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid source path."));
        return;
    }

    if (sourcePath == destinationPath) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot move to the source."));
        return;
    }

    if (destinationPath.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid destination path."));
        return;
    }

    QFileInfo sourceInfo(sourcePath);
    QString baseName = sourceInfo.fileName();
    QString dirPath = sourceInfo.absolutePath();


    if (dirPath == destinationPath) {
        QMessageBox::warning(this, tr("Error"), tr("Item cannot be moved within the same directory."));
        return;
    }

    qDebug() << baseName;
    qDebug() << dirPath;

//    QFileInfo sourceInfo(sourcePath);
//    QString baseName = sourceInfo.fileName();

    if (sourceInfo.isDir() && sourceInfo.isWritable()) {
        QDir sourceDir(sourcePath);
        QString destinationDirPath = destinationPath + "/" + baseName;

        if (!sourceDir.exists()) {
            QMessageBox::warning(this, tr("Error"), tr("Source directory does not exist."));
            return;
        }

        QDir destDir(destinationDirPath);


        if (destDir.exists()) {
            //            handleDirectoryMerge(sourceDir, destDir);
            mergeDirectories(sourceDir, destDir);
            return;
        } else {
            if (!copy_directory(sourceDir.absolutePath(), destinationPath)) {
                QMessageBox::warning(this, tr("Error"), tr("Failed to move the directory."));
            } else {
                sourceDir.removeRecursively();
            }
        }
    } else if (sourceInfo.isFile() && sourceInfo.isWritable()) {
        QDir destDir(destinationPath);
        if (destDir.exists()) {
            destinationPath = destDir.filePath(baseName);
        }

        if (QFileInfo::exists(destinationPath)) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("File Exists"),
                                          tr("The file %1 already exists at the destination. Do you want to overwrite it?").arg(
                                              baseName),
                                          QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                QFile::remove(destinationPath);
            } else {
                return;
            }
        }

        QFile sourceFile(sourcePath);
        if (!sourceFile.rename(destinationPath)) {
            QMessageBox::warning(this, tr("Error"), tr("Failed to move the file."));
            return;
        }
    } else {
        return;
    }

    QMessageBox::information(this, tr("Success"), tr("Item moved successfully."));
}




bool MainWidget::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::DragEnter) {
        auto *dragEnterEvent = static_cast<QDragEnterEvent*>(event);
        qDebug() << "Drag Enter Event";
        dragEnterEvent->acceptProposedAction();
        return true;
    } else if (event->type() == QEvent::DragMove) {
        auto *dragMoveEvent = static_cast<QDragMoveEvent*>(event);
        qDebug() << "Drag Move Event";
        dragMoveEvent->acceptProposedAction();
        return true;
    } else if (event->type() == QEvent::Drop) {
        auto *dropEvent = static_cast<QDropEvent*>(event);
        qDebug() << "Drop Event";

        const QMimeData *mimeData = dropEvent->mimeData();
        if (mimeData->hasUrls()) {
            QList<QUrl> urls = mimeData->urls();
            if (!urls.isEmpty()) {
                QString sourcePath = urls.first().toLocalFile();
                // Pass the drop position to determine the destination path
                QString destinationPath = determineDestinationPath(obj, dropEvent->pos());

                if (!sourcePath.isEmpty() && !destinationPath.isEmpty()) {
                    moveItem(sourcePath, destinationPath);
                }
            }
        }
        return true;
    }

    return QWidget::eventFilter(obj, event);
}








