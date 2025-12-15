#ifndef CIMAGEWINDOW_H
#define CIMAGEWINDOW_H
#include "roi_global.h"

#include <QFrame>

namespace cv{ class Mat; }
namespace Halcon{ class Hobject; }

class CRoiList;
class QScrollBar;
struct CImageWindowPrivate;


typedef bool (*FnCvMatToQImage)(const cv::Mat &, QImage &);
typedef bool (*FnHojectToQImage)(const Halcon::Hobject &, QImage &);


class ROISHARED_EXPORT CImageWindow : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool pointPixel READ pointPixel WRITE setPointPixel)

public:
    static void setFunctionImageFromCvMat(FnCvMatToQImage fn);
    static void setFunctionImageFromHobject(FnHojectToQImage fn);

    explicit CImageWindow(QWidget *parent = 0);
    ~CImageWindow();

    void showImage(const QString strPath);
    void showImage(const QImage &image);
    void showImage(const cv::Mat &mat);
    void showImage(const Halcon::Hobject &hImage);
    void showImage(const uchar *pR, const uchar *pG, const uchar *pB, int width, int height);
    void clearImage();

    QRect imageRect();
    QRect viewRect();
    const QImage &image();

    bool pointPixel();
    void setPointPixel(bool bEmit);

    CRoiList *roiList();
    void setRoiList(CRoiList *pRoiList);

    void moveImage(const QPointF &ptOffset);

    void scaleImage(const QPointF &ptCenter, double scale);

    void setImageRatio(double ratio);

    void adjustWindow();

signals:
    void signal_point_pixel(QPoint point, unsigned int rgba);

public slots:
    void slot_zoom_in() { scaleImage(QPointF(0.0, 0.0), 0.5); }
    void slot_zoom_out() { scaleImage(QPointF(0.0, 0.0), -0.5); }
    void slot_adjust_size() { adjustWindow(); }
    void slot_xd5_size() { setImageRatio(0.5); }
    void slot_x1_size() { setImageRatio(1.0); }
    void slot_x2_size() { setImageRatio(2.0); }
    void slot_x4_size() { setImageRatio(4.0); }
    void slot_save_image();

protected slots:
    void slot_hscroll_action_triggered(int action);
    void slot_vscroll_action_triggered(int action);

    void slot_insert_roi(int index, void *pRoiData);
    void slot_remove_roi(int index);
    void slot_roi_move(int index, int to);
    void slot_roi_adjust_window(int index);
    void slot_roi_set_color(int index, const QColor &color);
    void slot_roi_set_visible(int index, bool visible);
    void slot_roi_set_enable(int index, bool enable);
    void slot_roi_set_data(int index, void *pRoiData, QObject *pSender);
    void slot_roi_set_value(int index, void *pRoiData, QObject *pSender);
    void slot_current_index_changed(int index);

protected:
    virtual bool eventFilter(QObject *object, QEvent *event);

private:
    void imagePaintEvent(QPaintEvent *event);
    void imageResizeEvent(QResizeEvent *event);
    void imageWheelEvent(QWheelEvent *event);
    void imageMousePressEvent(QMouseEvent *event);
    void imageMouseReleaseEvent(QMouseEvent *event);
    void imageMouseMoveEvent(QMouseEvent *event);
    void imageContextMenuEvent(QContextMenuEvent *event);
    void clearRoiWidget();

private:
    int scrollMove(QScrollBar *pBar, int action, int &oldValue, bool &bOk);
    void updatePosition();
    void updatePointPixel(const QPoint &pt);

private:
    CImageWindowPrivate *d;
};

#endif // CIMAGEWINDOW_H
