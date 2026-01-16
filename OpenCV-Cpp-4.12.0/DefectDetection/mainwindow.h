#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "DefectDetector.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onDetectDefect();

private:
    QLabel *m_normalLabel;
    QLabel *m_defectLabel;
    QLabel *m_resultLabel;
    QPushButton *m_detectButton;
    DefectDetector *m_detector;

    cv::Mat m_normalImage;
    cv::Mat m_defectImage;
};
