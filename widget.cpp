#include "widget.h"
#include "ui_widget.h"
#include <QDateTime>
#include <QFileIconProvider>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    model = new QFileSystemModel(this);
    model->setFilter(QDir::QDir::AllEntries);
    model->setRootPath("");
    ui->lvBackup->setModel(model);
    ui->lvSourse->setModel(model);

    thread = new QThread(this);
    connect(this, SIGNAL(destroyed()), thread, SLOT(quit()));

    connect(ui->lvBackup, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(on_lvSourse_doubleClicked(QModelIndex)));
    worker = new BackupWorker();
    connect(this, SIGNAL(startOperation(QString, QString)), worker, SLOT(runBackup(QString, QString)));

    worker->moveToThread(thread);
    thread->start();

    connect(worker, SIGNAL(backupFinished()),
            this, SLOT(readyToStart()));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_lvSourse_doubleClicked(const QModelIndex &index)
{
    QListView *listView = (QListView*)sender();
    QFileInfo fileInfo = model->fileInfo(index);
    if (fileInfo.fileName() == ".."){
        QDir dir = fileInfo.dir();
        dir.cdUp();
        listView->setRootIndex(model->index(dir.absolutePath()));
    }
    else if (fileInfo.fileName() == "."){
        listView->setRootIndex(model->index(""));
    }
    else if (fileInfo.isDir()){
        listView->setRootIndex(index);
    }
}

void BackupWorker::recursiveContentList(QDir &dir, QFileInfoList &contentList)
{
    foreach(QFileInfo info, dir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst)){
        contentList.append(info);
        if (info.isDir() && dir.cd(info.fileName())){
            recursiveContentList(dir, contentList);
            dir.cdUp();
        }
    }
}

void BackupWorker::contentDifference(QDir &sDir, QDir &dDir, QFileInfoList &diffList)
{
    foreach(QFileInfo sInfo, sDir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst)){
       bool fileExists = false;
       foreach(QFileInfo dInfo, dDir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst)){
           if (sInfo.fileName() == dInfo.fileName()){
               if (sInfo.isDir() || sInfo.lastModified() <= dInfo.lastModified())
                   fileExists = true;
               break;
           }
       }
       if (!fileExists)
           diffList.append(sInfo);
       if (sInfo.isFile())
           continue;
       if (fileExists){
           sDir.cd(sInfo.fileName());
           dDir.cd(sInfo.fileName());
           contentDifference(sDir, dDir, diffList);
           sDir.cdUp();
           dDir.cdUp();
       }
       else {
           sDir.cd(sInfo.fileName());
           recursiveContentList(sDir, diffList);
           sDir.cdUp();
       }
    }
}

void Widget::on_pushButton_clicked()
{
    QString sDir = model->filePath(ui->lvSourse->rootIndex());
    QString dDir = model->filePath(ui->lvBackup->rootIndex());

    ui->pushButton->setEnabled(false);
    emit startOperation(sDir, dDir);
}

void Widget::readyToStart()
{
    ui->pushButton->setEnabled(true);
}

BackupWorker::BackupWorker(QObject *parent)
{

}

void BackupWorker::runBackup(QString sPath, QString dPath)
{
    QDir sDir = QDir(sPath);
    QDir dDir = QDir(dPath);

    QFileInfoList diffList = QFileInfoList();
    contentDifference(sDir, dDir, diffList);

    foreach(QFileInfo diffInfo, diffList){
        QString backupPath = diffInfo.filePath().replace(sDir.absolutePath(), dDir.absolutePath());
        if (diffInfo.isFile()){
            QFile::remove(backupPath);
            QFile::copy(diffInfo.absoluteFilePath(), backupPath);
        }
        if (diffInfo.isDir()){
            dDir.mkdir(backupPath);
        }
    }
}
