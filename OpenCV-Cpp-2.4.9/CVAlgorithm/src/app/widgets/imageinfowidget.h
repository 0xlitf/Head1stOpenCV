#ifndef IMAGEINFOWIDGET_H
#define IMAGEINFOWIDGET_H

#pragma execution_character_set("utf-8")

#include "controls/controls.h"
#include <QFileInfo>
#include <QWidget>

// 前向声明
class QLabel;
class QVBoxLayout;
class QHBoxLayout;

class ImageInfoWidget : public WidgetBase {
    Q_OBJECT
signals:
    void thumbnailClicked();

public:
    explicit ImageInfoWidget(QWidget *parent = nullptr);

    // 核心方法：设置要显示的文件信息，并生成缩略图
    void setFileInfo(const QFileInfo &fileInfo);

    // 单独设置缩略图（可选）
    void setThumbnail(const QPixmap &pixmap);

    // 单独设置其他信息（可选）
    void setFileName(const QString &fileName);
    void setImageSize(const QSize &size);
    void setFileSize(qint64 bytes);

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    void setupUI();
    void updateDisplay();

    // 缩略图标签
    QLabel *m_thumbnailLabel;

    // 信息显示标签
    QLabel *m_nameLabel;
    QLabel *m_infoLabel;

    // 布局
    QHBoxLayout *m_mainLayout;
    QVBoxLayout *m_infoLayout;

    // 存储数据
    QString m_fileName;
    QSize m_imageSize;
    qint64 m_fileSizeBytes;

    // 生成缩略图的辅助函数
    QPixmap generateThumbnail(const QString &filePath, const QSize &size);

public slots:
    // void onThumbnailClicked();
};

#endif // IMAGEINFOWIDGET_H
