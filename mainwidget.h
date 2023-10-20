#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QFileSystemModel>
#include <QDir>

namespace Ui {
class MainWidget;
}


class MainWidget: public QWidget {
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();
private slots:


    void on_fileList_1_doubleClicked(const QModelIndex &index);


    void on_fileTree_1_doubleClicked(const QModelIndex &index);
    void on_fileTree_2_doubleClicked(const QModelIndex &index);


private:
    Ui::MainWidget *ui;
    QFileSystemModel *model_1;
    QFileSystemModel *model_2;
};

#endif // MAINWIDGET_H
