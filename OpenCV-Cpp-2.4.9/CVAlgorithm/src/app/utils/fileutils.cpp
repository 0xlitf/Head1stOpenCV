#include "fileutils.h"

#include <QDesktopServices>
#include <QProcess>

FileUtils::FileUtils(QObject *parent) : QObject{parent} {}

QString FileUtils::selectFolderDialog(QWidget *parent) {
    auto defaultWorkDir = (qApp->property("RunEvn") == "exe") ? qApp->applicationDirPath(): QString(PROJECT_DIR);
    QString folder = QFileDialog::getExistingDirectory(
        parent, "选择文件夹", defaultWorkDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!folder.isEmpty()) {
        qDebug() << "选择的文件夹:" << folder;
        return folder;
    } else {
        qDebug() << "用户取消了选择";
        return QString();
    }
}

QString FileUtils::selectFileDialog(const QString &filter, QWidget *parent) {
    auto defaultWorkDir = (qApp->property("RunEvn") == "exe") ? qApp->applicationDirPath(): QString(PROJECT_DIR);
    QString folder = QFileDialog::getOpenFileName(parent, "选择文件夹",
                                                  defaultWorkDir, filter);

    if (!folder.isEmpty()) {
        qDebug() << "选择的文件夹:" << folder;
        return folder;
    } else {
        qDebug() << "用户取消了选择";
        return QString();
    }
}

QPair<int, int> FileUtils::recursiveCopyFolder(const QString &sourceDir,
                                               const QString &destinationDir) {
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

QMap<QString, QString>
FileUtils::gatherCopyFilesTo(const QString &sourceDir,
                             const QString &destinationDir) {
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

QStringList FileUtils::findAllImageFiles(const QString &directory, bool recursive) {
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

    if (recursive) {
        QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (const QString &subDir, subDirs) {
            imageFiles.append(findAllImageFiles(dir.absoluteFilePath(subDir)));
        }
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

QString FileUtils::getImageFileFilter() {
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

void FileUtils::showInFolder(const QString &filePath) {
    QFileInfo info(filePath);
    if (!info.exists()) {
        return;
    }

#if defined(Q_OS_WIN)
    QStringList args;
    args << "/select," << QDir::toNativeSeparators(info.absoluteFilePath());
    QProcess::startDetached("explorer", args);
#elif defined(Q_OS_MAC)
    QStringList args;
    args << "-R" << info.absoluteFilePath();
    QProcess::startDetached("open", args);
#else
    // Linux 或其他类Unix系统
    QDesktopServices::openUrl(QUrl::fromLocalFile(info.absolutePath()));
#endif
}

bool FileUtils::makeFilePath(const QString &path) {
    QFileInfo fileInfo(path);

    QDir dir(fileInfo.absolutePath());
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return false;
}

bool FileUtils::makeFolderPath(const QString &path) {
    QDir dir(path);
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return false;
}

bool FileUtils::removeFolder(const QString &folderPath) {
    QDir dir(folderPath);

    if (!dir.exists()) {
        qDebug() << "文件夹不存在，无需删除:" << folderPath;
        return true;
    }

    if (dir.removeRecursively()) {
        qDebug() << "文件夹删除成功:" << folderPath;
        return true;
    } else {
        qDebug() << "错误：文件夹删除失败:" << folderPath;
        return false;
    }
}
