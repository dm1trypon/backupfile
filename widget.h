#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFileSystemModel>
#include <QDir>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_lvSourse_doubleClicked(const QModelIndex &index);

    void on_pushButton_clicked();

private:
    Ui::Widget *ui;
    QFileSystemModel *model;
};

void contentDifference(QDir &sDir, QDir &dDir, QFileInfoList &diffList);

void recursiveContentList(QDir &dir, QFileInfoList &contentList);

#endif // WIDGET_H
