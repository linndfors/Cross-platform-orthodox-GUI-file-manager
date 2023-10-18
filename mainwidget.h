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

    void on_lvSource_doubleClicked(const QModelIndex &index);

private:
    Ui::MainWidget *ui;
    QFileSystemModel *model;
};

#endif // MAINWIDGET_H
