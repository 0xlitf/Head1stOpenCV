#include "fileutils.h"

FileUtils::FileUtils(QObject *parent)
    : QObject{parent} {}

QString FileUtils::selectFolderDialog(QWidget *parent) {
    QString folder = QFileDialog::getExistingDirectory(parent,
                                                       "选择文件夹",
                                                       "",
                                                       QFileDialog::ShowDirsOnly
                                                           | QFileDialog::DontResolveSymlinks);

    if (!folder.isEmpty()) {
        qDebug() << "选择的文件夹:" << folder;
        return folder;
    } else {
        qDebug() << "用户取消了选择";
        return QString();
    }
}

QString FileUtils::selectFileDialog(const QString& filter, QWidget *parent) {
    QString folder = QFileDialog::getOpenFileName(parent,
                                                       "选择文件夹",
                                                       "",
                                                       filter);

    if (!folder.isEmpty()) {
        qDebug() << "选择的文件夹:" << folder;
        return folder;
    } else {
        qDebug() << "用户取消了选择";
        return QString();
    }
}

QPair<int, int> FileUtils::recursiveCopyFolder(const QString &sourceDir, const QString &destinationDir) {
    qDebug() << "sourceDir: " << sourceDir;
    qDebug() << "destinationDir: " << destinationDir;
    QStringList copiedFiles;
    QStringList errorFiles;

    QStringList allImageFiles = findAllImageFiles(sourceDir);
    qDebug() << "allImageFiles.size: " << allImageFiles.size();

    int successCount = 0;
    int failCount = 0;

    foreach (const QString &sourceFile, allImageFiles) {
        QFileInfo fileInfo(sourceFile);
        QString relativePath = QDir(sourceDir).relativeFilePath(sourceFile);
        QString destFile = QDir(destinationDir).absoluteFilePath(relativePath);

        qDebug() << "relativePath" << relativePath;
        qDebug() << "destFile" << destFile;

        QDir destDir = QFileInfo(destFile).absoluteDir();
        if (!destDir.exists()) {
            destDir.mkpath(".");
        }

        if (QFile::copy(sourceFile, destFile)) {
            copiedFiles << sourceFile;
            successCount++;
        } else {
            errorFiles << sourceFile;
            failCount++;
        }

        QCoreApplication::processEvents();
    }
    return QPair<int, int>(successCount, failCount);
}

QMap<QString, QString> FileUtils::gatherCopyFilesTo(const QString &sourceDir, const QString &destinationDir) {
    QStringList copiedFiles;
    QStringList errorFiles;

    QMap<QString, QString> map;

    QStringList allImageFiles = findAllImageFiles(sourceDir);

    int successCount = 0;
    int failCount = 0;

    QDir destDir = QDir(destinationDir);

    if (!destDir.removeRecursively()) {
        qDebug() << "删除目录失败:" << destinationDir;
    }

    if (!destDir.exists()) {
        destDir.mkpath(".");
    }

    foreach (const QString &sourceFile, allImageFiles) {
        QFileInfo fileInfo(sourceFile);
        QString filename = fileInfo.fileName();
        QString destFile = QDir(destinationDir).absoluteFilePath(filename);

        if (QFile::copy(sourceFile, destFile)) {
            copiedFiles << sourceFile;
            successCount++;

            map.insert(sourceFile, destFile);
        } else {
            errorFiles << sourceFile;
            failCount++;
            qDebug() << "拷贝文件失败:" << destDir;
        }

        QCoreApplication::processEvents();
    }
    return map;
}

QStringList FileUtils::findAllImageFiles(const QString &directory) {
    QStringList imageFiles;
    QDir dir(directory);

    if (!dir.exists()) {
        return imageFiles;
    }

    QStringList filters;
    foreach (const QString &format, QImageReader::supportedImageFormats()) {
        filters << "*." + format;
    }

    imageFiles.append(dir.entryList(filters, QDir::Files));
    for (int i = 0; i < imageFiles.size(); ++i) {
        imageFiles[i] = dir.absoluteFilePath(imageFiles[i]);
    }

    QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const QString &subDir, subDirs) {
        imageFiles.append(findAllImageFiles(dir.absoluteFilePath(subDir)));
    }

    return imageFiles;
}

QStringList FileUtils::findDepth1Folder(const QString &directory) {
    QStringList result;
    QDir dir(directory);

    if (!dir.exists()) {
        qDebug() << "目录不存在:" << directory;
        return result;
    }

    // 设置过滤器：只获取目录，排除 . 和 ..
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    // 获取所有直接子目录
    result = dir.entryList();

    return result;
}
