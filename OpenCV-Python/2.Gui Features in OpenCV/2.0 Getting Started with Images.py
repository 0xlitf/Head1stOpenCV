import numpy as np
import cv2 as cv
import sys
from PyQt6.QtWidgets import QApplication, QMessageBox
from PyQt6.QtCore import QFile
import matplotlib
from matplotlib import pyplot as plt

matplotlib.use('QtAgg')


def check_file(file_path):
    return QFile.exists(file_path)


def show_error_message():
    app = QApplication(sys.argv)
    msg_box = QMessageBox()
    msg_box.setIcon(QMessageBox.Icon.Critical)
    msg_box.setWindowTitle("错误")
    msg_box.setText("文件不存在！")
    msg_box.exec()


if __name__ == '__main__':
    img_name = "../image/github_icon_color.jpg"

    if not check_file(img_name):
        show_error_message()
        exit(1)

    # cv.IMREAD_COLOR == 1： 加载彩色图像。任何图像的透明度都会被忽视。它是默认标志。
    # cv.IMREAD_GRAYSCALE == 0：以灰度模式加载图像
    # cv.IMREAD_UNCHANGED == -1：加载图像，包括alpha通道
    # 注意 除了这三个标志，你可以分别简单地传递整数1、0或-1。

    cv.imshow("1", cv.imread(img_name, 1))
    cv.imshow("0", cv.imread(img_name, 0))
    cv.imshow("-1", cv.imread(img_name, -1))

    img = cv.imread(img_name, 0)
    print("plt.get_backend(): ", plt.get_backend())
    plt.imshow(img, cmap='gray', interpolation='bicubic')
    # plt.xticks([]), plt.yticks([])  # 隐藏 x 轴和 y 轴上的刻度值
    plt.show()

    # 如果使用的是64位计算机，则必须按如下所示修改行：k = cv.waitKey(0) & 0xFF
    k = cv.waitKey(0) & 0xFF
    if k == 27:  # 等待ESC退出
        cv.destroyAllWindows()
    elif k == ord('s'):  # 等待关键字，保存和退出
        cv.imwrite('imwrite_example.png', cv.imread(img_name, 0))
        cv.destroyAllWindows()
