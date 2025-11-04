import cv2 as cv
import matplotlib
import numpy as np
import sys
from matplotlib import pyplot as plt
import inspect
from PySide6.QtWidgets import QApplication, QWidget

matplotlib.use("QtAgg", force=True)
plt.ion()


# cdf Cumulative Distribution Function 累积分布函数
def cumulative_distribution_function():
    current_function_name = inspect.currentframe().f_code.co_name
    print("current_function_name:", current_function_name)

    img = cv.imread('../data/wiki.png', 0)
    hist, bins = np.histogram(img.flatten(), 256, [0, 256])
    cdf = hist.cumsum()
    cdf_normalized = cdf * float(hist.max()) / cdf.max()
    plt.plot(cdf_normalized, color='b')
    plt.hist(img.flatten(), 256, [0, 256], color='r')
    plt.xlim([0, 256])
    plt.legend(('cdf', 'histogram'), loc='upper left')
    plt.show()

    cdf_m = np.ma.masked_equal(cdf, 0)
    cdf_m = (cdf_m - cdf_m.min()) * 255 / (cdf_m.max() - cdf_m.min())
    cdf = np.ma.filled(cdf_m, 0).astype('uint8')

    img2 = cdf[img]
    plt.figure()
    plt.plot(cdf, color='b')
    plt.hist(img2.flatten(), 256, [0, 256], color='r')
    plt.xlim([0, 256])
    plt.legend(('cdf', 'histogram'), loc='upper left')
    plt.show()


def equalization():
    img = cv.imread('../data/wiki.png', 0)
    equ = cv.equalizeHist(img)
    print(equ)
    res = np.hstack((img, equ))  # stacking images side-by-side
    cv.imshow('res', res)

    img = cv.imread('../data/tsukuba_l.png', 0)
    # create a CLAHE object (Arguments are optional).
    clahe = cv.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))
    cl1 = clahe.apply(img)
    cv.imwrite('../data/clahe_2.jpg', cl1)

    plt.figure()
    plt.subplot(121)
    plt.imshow(img)
    plt.subplot(122)
    plt.imshow(plt.imread('../data/clahe_2.jpg'))
    plt.show()


def hist2d():
    img = cv.imread('../data/home.jpg')
    hsv = cv.cvtColor(img, cv.COLOR_BGR2HSV)
    hist = cv.calcHist([hsv], [0, 1], None, [180, 256], [0, 180, 0, 256])
    plt.figure()
    plt.subplot(121)
    rgb = cv.cvtColor(img, cv.COLOR_BGR2RGB)
    plt.imshow(rgb)
    plt.subplot(122)
    plt.imshow(hist, interpolation='nearest')
    plt.show()


def back_projection():
    # roi是我们需要找到的对象或对象区域
    roi = cv.imread('rose_red.png')
    hsv = cv.cvtColor(roi, cv.COLOR_BGR2HSV)
    # 目标是我们搜索的图像
    target = cv.imread('rose.png')
    hsvt = cv.cvtColor(target, cv.COLOR_BGR2HSV)
    # 使用calcHist查找直方图。也可以使用np.histogram2d完成
    M = cv.calcHist([hsv], [0, 1], None, [180, 256], [0, 180, 0, 256])
    I = cv.calcHist([hsvt], [0, 1], None, [180, 256], [0, 180, 0, 256])


if __name__ == "__main__":
    app = QApplication(sys.argv)

    cumulative_distribution_function()
    equalization()
    hist2d()
    # back_projection()

    sys.exit(app.exec())
