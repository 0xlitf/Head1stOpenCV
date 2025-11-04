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

    img = cv.imread('../data/pca_test1.jpg', 0)
    # 初始化FAST检测器
    # 若找不到xfeatures2d，卸载pip uninstall opencv-python，安装pip install opencv-contrib-python
    star = cv.xfeatures2d.StarDetector_create()
    # 初始化BRIEF提取器
    brief = cv.xfeatures2d.BriefDescriptorExtractor_create()
    # 找到STAR的关键点
    kp = star.detect(img, None)
    # 计算BRIEF的描述符
    kp, des = brief.compute(img, kp)
    print(brief.descriptorSize())
    print(des.shape)
    cv.imshow('des', des)
    cv.imshow('img', img)


if __name__ == "__main__":
    app = QApplication(sys.argv)

    function()

    sys.exit(app.exec())
