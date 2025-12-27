#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>
#include <QImageReader>
#include <QFileDialog>

#pragma execution_character_set("utf-8")

class FileUtils : public QObject {
    Q_OBJECT
public:
    explicit FileUtils(QObject *parent = nullptr);

    static QString selectFolderDialog(QWidget* parent = nullptr);

    static QString selectFileDialog(const QString& filter = getImageFileFilter(), QWidget* parent = nullptr);

    static QPair<int, int> recursiveCopyFolder(const QString& sourceDir, const QString& destinationDir);

    static QMap<QString, QString> gatherCopyFilesTo(const QString& sourceDir, const QString& destinationDir);

    static QStringList findAllImageFiles(const QString& directory);

    static QStringList findDepth1Folder(const QString& directory);

    static QString getRelativePath(const QString& absolutePath, const QString& basePath) {
        QDir baseDir(basePath);
        return baseDir.relativeFilePath(absolutePath);
    }

    static QString getImageFileFilter() {
        QList<QByteArray> formats = QImageReader::supportedImageFormats();

        QStringList filterList;

        for (const QByteArray &format : formats) {
            filterList.append(QString("*.%1").arg(QString(format)));
        }

        QString filter = QString("Images (%1)").arg(filterList.join(" "));

        // 您还可以添加一个"所有文件"的选项
        filter += ";;All Files (*)";

        return filter;
    }

signals:
};

#endif // FILEUTILS_H
