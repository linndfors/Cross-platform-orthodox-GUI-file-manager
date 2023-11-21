#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QFileSystemModel>
#include <QTreeView>
#include <QWidget>
#include <QKeyEvent>
#include <QDir>

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
    void prompt_for_filename();
    void prompt_for_folder_name();
    void copy();
    bool copy_file(const QString &sourcePath, const QString &destinationPath);
    bool copy_directory(const QString &sourcePath, const QString &destinationPath, bool isRoot);
    void move();


    QFileSystemModel* setup_file_system_model(QDir::Filters filter);
    void setup_tree_view(QTreeView *view, QFileSystemModel *model);

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


    QString getUniqueDestinationName(const QString &destinationPath, const QString &baseName);

protected:
    void keyPressEvent(QKeyEvent *event) override {
        // Handle Ctrl+C (copy) shortcut
        if (event->matches(QKeySequence::Copy)) {
            copySelectedItems();
        }
        // Handle Ctrl+V (paste) shortcut
        else if (event->matches(QKeySequence::Paste)) {
            // Implement your paste logic here
            // For example, you can call a paste function
            // pasteSelectedItems();
        } else {
            // Call the base class implementation for other key events
            QWidget::keyPressEvent(event);
        }
    }


private:
    Ui::MainWidget *ui;
    QFileSystemModel *model_1;
    QFileSystemModel *model_2;
    QMenu* contextMenu;
    QAction* newFileAction;
    QAction* newDirAction;
    QAction* deleteAction;
    QAction* renameAction;
    QAction* copyAction;
    QAbstractItemView* contextMenuView;


};

#endif // MAINWIDGET_H
