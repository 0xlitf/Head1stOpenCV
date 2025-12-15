#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QPalette>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 输出完整的版本字符串（例如：4.12.0）
    std::cout << "OpenCV版本: " << CV_VERSION << std::endl;

    // 如果需要分别获取主版本、次版本和修订号，可以使用以下宏
    std::cout << "主版本: " << CV_MAJOR_VERSION << std::endl;
    std::cout << "次版本: " << CV_MINOR_VERSION << std::endl;
    std::cout << "修订号: " << CV_VERSION_REVISION << std::endl;

    // 设置应用程序信息
    a.setApplicationName("GeoMatchQt");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("GeoMatch");
    a.setOrganizationDomain("geomatch.com");

    // 设置样式
    a.setStyle(QStyleFactory::create("Fusion"));

    // 设置调色板
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(240, 240, 240));
    palette.setColor(QPalette::WindowText, QColor(0, 0, 0));
    palette.setColor(QPalette::Base, QColor(255, 255, 255));
    palette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
    palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
    palette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
    palette.setColor(QPalette::Text, QColor(0, 0, 0));
    palette.setColor(QPalette::Button, QColor(240, 240, 240));
    palette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
    palette.setColor(QPalette::BrightText, QColor(255, 0, 0));
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    a.setPalette(palette);

    // 创建主窗口
    MainWindow w;
    w.show();

    return a.exec();
}
