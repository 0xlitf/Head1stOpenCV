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

    img = cv.imread('simple.jpg', 0)
    # 初始化ORB检测器
    orb = cv.ORB_create()
    # 用ORB寻找关键点
    kp = orb.detect(img, None)
    # 用ORB计算描述符
    kp, des = orb.compute(img, kp)
    # 仅绘制关键点的位置，而不绘制大小和方向
    img2 = cv.drawKeypoints(img, kp, None, color=(0, 255, 0), flags=0)
    plt.imshow(img2), plt.show()


if __name__ == "__main__":
    app = QApplication(sys.argv)

    function()

    sys.exit(app.exec())
