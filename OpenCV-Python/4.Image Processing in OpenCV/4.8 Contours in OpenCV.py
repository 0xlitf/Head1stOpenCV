import cv2 as cv
import matplotlib
import numpy as np
import sys
from matplotlib import pyplot as plt
import inspect
from PySide6.QtWidgets import QApplication, QWidget

matplotlib.use("QtAgg", force=True)
plt.ion()


def contours_in_OpenCV():
    img = cv.imread('../data/white_rect_in_black.png')
    imgray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
    ret, thresh = cv.threshold(imgray, 127, 255, 0)
    contours, hierarchy = cv.findContours(thresh, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)
    outline = img.copy()
    cv.drawContours(outline, contours, -1, (0, 255, 0), 2)

    # cv.drawContours(img, contours, 3, (0, 255, 0), 3)
    # cnt = contours[4]
    # cv.drawContours(img, [cnt], 0, (0, 255, 0), 3)

    plt.figure(1)
    plt.subplot(121), plt.imshow(cv.cvtColor(img, cv.COLOR_BGR2RGB))
    plt.title('img'), plt.xticks([]), plt.yticks([])

    plt.subplot(122), plt.imshow(cv.cvtColor(outline, cv.COLOR_BGR2RGB))
    plt.title('outline'), plt.xticks([]), plt.yticks([])

    plt.show()


def moments(pic):  # ../data/white_rect_in_black.png #../4.8.0/star_filled@2x.png
    img = cv.imread(pic, 0)
    img_clor = cv.imread(pic, 1)
    ret, thresh = cv.threshold(img, 127, 255, 0)
    contours, hierarchy = cv.findContours(thresh, 1, 2)

    outline = img_clor.copy()
    cv.drawContours(outline, contours, -1, (0, 255, 0), 2)

    cnt = contours[0]
    M = cv.moments(cnt)
    print(M)
    cx = int(M['m10'] / M['m00'])
    cy = int(M['m01'] / M['m00'])
    print(cx)
    print(cy)

    # 2. 轮廓面积
    area = cv.contourArea(cnt)
    print("area: ", area)

    # 3. 轮廓周长
    perimeter = cv.arcLength(cnt, True)
    print("perimeter: ", perimeter)

    # 4. 轮廓近似 根据我们指定的精度，它可以将轮廓形状近似为顶点数量较少的其他形状。它是Douglas-Peucker算法的实现
    epsilon = 0.1 * cv.arcLength(cnt, True)
    approx = cv.approxPolyDP(cnt, epsilon, True)
    print("epsilon: ", epsilon)
    print("approx: ", approx)

    plt.figure(2)
    plt.subplot(331), plt.imshow(cv.cvtColor(img, cv.COLOR_BGR2RGB))
    plt.title('img'), plt.xticks([]), plt.yticks([])

    plt.subplot(332), plt.imshow(cv.cvtColor(thresh, cv.COLOR_BGR2RGB))
    plt.title('thresh'), plt.xticks([]), plt.yticks([])

    plt.subplot(333), plt.imshow(cv.cvtColor(outline, cv.COLOR_BGR2RGB))
    plt.title('outline'), plt.xticks([]), plt.yticks([])

    outline4 = img_clor.copy()
    cv.drawContours(outline4, approx, -1, (0, 255, 0), 20, cv.LINE_8)
    cv.drawContours(outline4, [approx], -1, (0, 255, 0), 2, cv.LINE_8)

    plt.subplot(334), plt.imshow(cv.cvtColor(outline4, cv.COLOR_BGR2RGB))
    plt.title('approx'), plt.xticks([]), plt.yticks([])


# 5. Convex Hull
def convex_hull_draw():
    # 新建512*512的空白图片
    img = np.zeros((512, 512, 3), np.uint8)
    # 平面点集
    pts = np.array([[200, 250], [250, 300], [300, 270], [270, 200], [120, 240]], np.int32)
    pts = pts.reshape((-1, 1, 2))
    # 绘制填充的多边形
    cv.fillPoly(img, [pts], (255, 255, 255))

    gray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
    # 二值化
    ret, thresh = cv.threshold(gray, 127, 255, cv.THRESH_BINARY)
    # 图片轮廓
    contours, hierarchy = cv.findContours(thresh, 2, 1)
    cnt = contours[0]
    # 寻找凸包并绘制凸包（轮廓）
    hull = cv.convexHull(cnt)
    print(hull)

    length = len(hull)
    for i in range(len(hull)):
        cv.line(img, tuple(hull[i][0]), tuple(hull[(i + 1) % length][0]), (0, 255, 0), 2)

    # 显示图片
    cv.imshow('line', img)
    cv.waitKey()


# 5. Convex Hull
def convex_hull(pic):
    # 读取图片并转至灰度模式
    img = cv.imread(pic, 1)
    gray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)

    # 二值化，取阈值为235
    ret, thresh = cv.threshold(gray, 235, 255, cv.THRESH_BINARY)

    # 寻找图像中的轮廓
    contours, hierarchy = cv.findContours(thresh, 1, 2)

    # 寻找物体的凸包并绘制凸包的轮廓
    for cnt in contours:
        hull = cv.convexHull(cnt)
        length = len(hull)
        # 如果凸包点集中的点个数大于5
        if length > 5:
            # 绘制图像凸包的轮廓
            for i in range(length):
                cv.line(img, tuple(hull[i][0]), tuple(hull[(i + 1) % length][0]), (0, 0, 255), 2)

    cv.imshow('finger', img)
    cv.waitKey()


if __name__ == "__main__":
    app = QApplication(sys.argv)

    # contours_in_OpenCV()
    # moments('../4.8.0/electric.jpg')
    # convex_hull('../data/gesture.png')
    convex_hull_draw()

    sys.exit(app.exec())
