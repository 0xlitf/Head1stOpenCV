#ifndef IMAGEINFOWIDGET_H
#define IMAGEINFOWIDGET_H

#pragma execution_character_set("utf-8")

#include "controls/controls.h"
#include <QFileInfo>
#include <QWidget>

class QLabel;
class QVBoxLayout;
class QHBoxLayout;

class ImageInfoWidget : public WidgetBase {
    Q_OBJECT
signals:
    void thumbnailClicked();

public:
    explicit ImageInfoWidget(QWidget *parent = nullptr);

    void setFileInfo(const QFileInfo &fileInfo);

    void setThumbnail(const QPixmap &pixmap);

    void setFileName(const QString &fileName);
    void setImageSize(const QSize &size);
    void setFileSize(qint64 bytes);

    // protected:
    //     void mousePressEvent(QMouseEvent *event);

public slots:
    // void onThumbnailClicked();

private:
    QPixmap generateThumbnail(const QString &filePath, const QSize &size);

private:
    void setupUI();
    void updateDisplay();

    QLabel *m_thumbnailLabel;

    QLabel *m_nameLabel;
    QLabel *m_infoLabel;

    QString m_fileName;
    QSize m_imageSize;
    qint64 m_fileSizeBytes;
};

#endif // IMAGEINFOWIDGET_H
