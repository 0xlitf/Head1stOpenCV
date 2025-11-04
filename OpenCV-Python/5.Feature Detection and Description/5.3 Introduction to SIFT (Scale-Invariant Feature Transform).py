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

    img = cv.imread('../data/home.jpg')
    gray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
    sift = cv.SIFT_create()
    kp = sift.detect(gray, None)
    img = cv.drawKeypoints(gray, kp, img, flags=cv.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)
    cv.imshow('sift_keypoints.jpg', img)


if __name__ == "__main__":
    app = QApplication(sys.argv)

    function()

    sys.exit(app.exec())
