import cv2 as cv
import matplotlib
import numpy as np
import sys
from matplotlib import pyplot as plt
import inspect
from PySide6.QtWidgets import QApplication, QWidget

matplotlib.use("QtAgg", force=True)
plt.ion()


def histgrams_in_opencv():
    current_function_name = inspect.currentframe().f_code.co_name
    print("current_function_name:", current_function_name)

    plt.subplot(121)
    img = cv.imread('../data/home.jpg', 0)
    hist = cv.calcHist([img], [0], None, [256], [0, 256])
    hist, bins = np.histogram(img.ravel(), 256, [0, 256])
    plt.hist(img.ravel(), 256, [0, 256])

    img_3_channel = cv.imread('../data/home.jpg', 1)
    color = ('b', 'g', 'r')
    plt.subplot(122)
    for i, col in enumerate(color):
        histr = cv.calcHist([img_3_channel], [i], None, [256], [0, 256])
        plt.plot(histr, color=col)
        plt.xlim([0, 256])
    plt.show()


def mask():
    plt.figure()
    img = cv.imread('../data/home.jpg', 0)
    # create a mask
    mask = np.zeros(img.shape[:2], np.uint8)
    mask[100:300, 100:400] = 255
    masked_img = cv.bitwise_and(img, img, mask=mask)
    # 计算掩码区域和非掩码区域的直方图
    # 检查作为掩码的第三个参数
    hist_full = cv.calcHist([img], [0], None, [256], [0, 256])
    hist_mask = cv.calcHist([img], [0], mask, [256], [0, 256])
    plt.subplot(221), plt.imshow(img, 'gray')
    plt.subplot(222), plt.imshow(mask, 'gray')
    plt.subplot(223), plt.imshow(masked_img, 'gray')
    plt.subplot(224), plt.plot(hist_full), plt.plot(hist_mask)
    plt.xlim([0, 256])
    plt.show()


if __name__ == "__main__":
    app = QApplication(sys.argv)

    histgrams_in_opencv()
    mask()

    sys.exit(app.exec())
