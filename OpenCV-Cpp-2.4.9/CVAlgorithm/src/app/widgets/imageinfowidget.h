#ifndef IMAGEINFOWIDGET_H
#define IMAGEINFOWIDGET_H

#include <QWidget>
#include <QFileInfo>

// 前向声明，避免包含过多头文件
class QLabel;
class QVBoxLayout;

class ImageInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageInfoWidget(QWidget *parent = nullptr);
    // 设置要显示的图片文件信息
    void setFileInfo(const QFileInfo& fileInfo);
    // 单独更新各个信息的接口（按需提供）
    void setFileName(const QString& fileName);
    void setImageSize(const QSize& size);
    void setFileSize(qint64 bytes);

private:
    void setupUI();
    void updateDisplay(); // 统一更新显示

    QLabel* m_nameLabel;
    QLabel* m_infoLabel;
    QVBoxLayout* m_layout;

    // 存储数据
    QString m_fileName;
    QSize m_imageSize;
    qint64 m_fileSizeBytes;
};

#endif // IMAGEINFOWIDGET_H
