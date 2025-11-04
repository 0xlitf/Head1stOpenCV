import numpy as np
import cv2 as cv
import sys
from PyQt6.QtWidgets import QApplication, QMessageBox
from PyQt6.QtCore import QFile
import matplotlib
from matplotlib import pyplot as plt

matplotlib.use('QtAgg')

if __name__ == '__main__':
    x = np.uint8([250])
    y = np.uint8([10])
    print(cv.add(x, y))
    print(x + y)

    img1 = cv.imread('../data/ml.png')
    img2 = cv.imread('../data/opencv-logo.png')


    def nothing(x):
        current_function_name = inspect.currentframe().f_code.co_name


    print("current_function_name:", current_function_name)

    cv.namedWindow('image')
    # 创建颜色变化的轨迹栏
    cv.createTrackbar('slider', 'image', 0, 100, nothing)

    min_size = min(img1.size, img2.size)

    if img1.size < img2.size:
        min_shape = img1.shape
    else:
        min_shape = img2.shape

    img1 = img1[0:min_shape[0], 0:min_shape[1]]
    img2 = img2[0:min_shape[0], 0:min_shape[1]]

    while 1:
        k = cv.waitKey(1) & 0xFF
        if k == 27:
            break

        s = cv.getTrackbarPos("slider", 'image')

        dst = cv.addWeighted(img1, s / 100, img2, 1 - s / 100, 0)
        cv.imshow('image', dst)
    cv.destroyAllWindows()
