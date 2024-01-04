#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QFileSystemModel>
#include <QTreeView>
#include <QWidget>
#include <QKeyEvent>
#include <QDir>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>

namespace Ui {
class MainWidget;
}

class MainWidget: public QWidget {
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private slots:
    void setup_views();
    void search_files();
    void setup_models();
    void setup_connections();
    void copy();
    bool copy_file(const QString &sourcePath, const QString &destinationPath);
    bool copy_directory(const QString &sourcePath, const QString &destinationPath);
    void move();
    void mergeDirectories(QDir& sourceDir, QDir& destDir);
    bool askUserForOverwrite(const QString& filePath);
    QFileSystemModel* setup_file_system_model(QDir::Filters filter);
    void setup_tree_view(QTreeView *view, QFileSystemModel *model);
    void setup_view(QAbstractItemView* view, QFileSystemModel* model);
    void display_selected_path(const QModelIndex &index);
    void on_fileList_doubleClicked(const QModelIndex &index);
    void on_fileTree_1_doubleClicked(const QModelIndex &index);
    void on_fileTree_2_doubleClicked(const QModelIndex &index);
    void createNewDirectory();
    void createNewFile();
    void showContextMenu(const QPoint &pos);
    void deleteSelectedItems();
    void renameSelectedItem();
    void compressSelectedItems();
    void copySelectedItems();
    void showSortDialog();
    void compareDirectories();
    void setLightMode();
    void toggleMode();
    void setDarkMode();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;


private:
    Ui::MainWidget *ui;
    QFileSystemModel *model_1;
    QFileSystemModel *model_2;
    QStringList similarFiles;
    QStringList differentFiles;
    QMenu* contextMenu;
    QAction* newFileAction;
    QAction* newDirAction;
    QAction* deleteAction;
    QAction* renameAction;
    QAction* copyAction;
    QAction* sortAction;
    QAbstractItemView* contextMenuView;
    QString determineDestinationPath(QObject *dropTarget, const QPoint &dropPosition);
    void moveItem(QString &sourcePath, QString &destinationPath);


};

#endif // MAINWIDGET_H
