#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFileSystemModel>
#include <QDir>
#include <QThread>

namespace Ui {
class Widget;
}

class BackupWorker : public QObject
{
    Q_OBJECT
public:
    explicit BackupWorker(QObject *parent = nullptr);
public slots:
    void runBackup(QString sPath, QString dPath);
signals:
void backupFinished();
private:
    void contentDifference(QDir &sDir, QDir &dDir, QFileInfoList &diffList);
    void recursiveContentList(QDir &dir, QFileInfoList &contentList);
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_lvSourse_doubleClicked(const QModelIndex &index);

    void on_pushButton_clicked();

    void readyToStart();

signals:
    void startOperation(QString sPath, QString dPath);

private:
    Ui::Widget *ui;
    QFileSystemModel *model;
    BackupWorker *worker;
    QThread *thread;
};



#endif // WIDGET_H
