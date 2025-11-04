import cv2 as cv
import matplotlib
import numpy as np
import sys
from matplotlib import pyplot as plt
import inspect
from PySide6.QtWidgets import QApplication, QWidget

matplotlib.use("QtAgg", force=True)
plt.ion()


def function():
    current_function_name = inspect.currentframe().f_code.co_name
    print("current_function_name:", current_function_name)

    filename = '../data/chessboard.png'
    img = cv.imread(filename)
    gray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
    gray = np.float32(gray)
    dst = cv.cornerHarris(gray, 2, 3, 0.04)
    # result用于标记角点，并不重要
    dst = cv.dilate(dst, None)
    # 最佳值的阈值，它可能因图像而异。
    img[dst > 0.01 * dst.max()] = [0, 0, 255]

    cv.namedWindow("dst", cv.WINDOW_NORMAL)
    cv.resizeWindow("dst", 600, 600)
    cv.imshow('dst', img)
    cv.moveWindow("dst", 100, 100)

    if cv.waitKey(0) & 0xff == 27:
        cv.destroyAllWindows()


def cornerSubPix():
    filename = '../data/chessboard2.jpg'
    img = cv.imread(filename)
    gray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
    # 寻找哈里斯角
    gray = np.float32(gray)
    dst = cv.cornerHarris(gray, 2, 3, 0.04)
    dst = cv.dilate(dst, None)
    ret, dst = cv.threshold(dst, 0.01 * dst.max(), 255, 0)
    dst = np.uint8(dst)
    # 寻找质心
    ret, labels, stats, centroids = cv.connectedComponentsWithStats(dst)
    # 定义停止和完善拐角的条件
    criteria = (cv.TERM_CRITERIA_EPS + cv.TERM_CRITERIA_MAX_ITER, 100, 0.001)
    corners = cv.cornerSubPix(gray, np.float32(centroids), (5, 5), (-1, -1), criteria)
    # 绘制
    res = np.hstack((centroids, corners))
    res = np.int0(res)
    img[res[:, 1], res[:, 0]] = [0, 0, 255]
    img[res[:, 3], res[:, 2]] = [0, 255, 0]
    cv.imwrite('subpixel5.png', img)


if __name__ == "__main__":
    app = QApplication(sys.argv)

    function()

    sys.exit(app.exec())
