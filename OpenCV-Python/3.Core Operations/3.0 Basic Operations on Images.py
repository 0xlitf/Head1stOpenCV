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

    # cv.IMREAD_COLOR： 加载彩色图像。任何图像的透明度都会被忽视。它是默认标志。
    # cv.IMREAD_GRAYSCALE：以灰度模式加载图像
    # cv.IMREAD_UNCHANGED：加载图像，包括alpha通道
    # 注意 除了这三个标志，你可以分别简单地传递整数1、0或-1。

    img = cv.imread(img_name)
    px = img[100, 100]
    print(px)
    blue = img[100, 100, 0]
    print(blue)
    img[100, 100] = [255, 255, 255]

    px_red = img.item(10, 10, 2)
    print(px_red)
    img.itemset((10, 10, 2), 100)
    print(img.item(10, 10, 2))

    print("img.shape:", img.shape)
    print("img.size:", img.size)
    print("img.dtype:", img.dtype)

    ball = img[280:340, 330:390]
    img[206:266, 206:266] = ball
    cv.imshow('img', img)
    cv.waitKey(0)

    b, g, r = cv.split(img)
    img = cv.merge((b, g, r))
    b = img[:, :, 0]
    img[:, :, 2] = 0

    BLUE = [255, 0, 0]
    img1 = cv.imread('../data/opencv-logo.png')
    replicate = cv.copyMakeBorder(img1, 10, 10, 10, 10, cv.BORDER_REPLICATE)
    reflect = cv.copyMakeBorder(img1, 10, 10, 10, 10, cv.BORDER_REFLECT)
    reflect101 = cv.copyMakeBorder(img1, 10, 10, 10, 10, cv.BORDER_REFLECT_101)
    wrap = cv.copyMakeBorder(img1, 10, 10, 10, 10, cv.BORDER_WRAP)
    constant = cv.copyMakeBorder(img1, 10, 10, 10, 10, cv.BORDER_CONSTANT, value=BLUE)
    plt.subplot(231), plt.imshow(cv.cvtColor(img1, cv.COLOR_BGR2RGB), 'gray'), plt.title('ORIGINAL')
    plt.subplot(232), plt.imshow(cv.cvtColor(replicate, cv.COLOR_BGR2RGB), 'gray'), plt.title('REPLICATE')
    plt.subplot(233), plt.imshow(cv.cvtColor(reflect, cv.COLOR_BGR2RGB), 'gray'), plt.title('REFLECT')
    plt.subplot(234), plt.imshow(cv.cvtColor(reflect101, cv.COLOR_BGR2RGB), 'gray'), plt.title('REFLECT_101')
    plt.subplot(235), plt.imshow(cv.cvtColor(wrap, cv.COLOR_BGR2RGB), 'gray'), plt.title('WRAP')
    plt.subplot(236), plt.imshow(cv.cvtColor(constant, cv.COLOR_BGR2RGB), 'gray'), plt.title('CONSTANT')
    plt.show()
