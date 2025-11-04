import numpy as np
import cv2 as cv
import sys
from PyQt6.QtWidgets import QApplication, QMessageBox
from PyQt6.QtCore import QFile
import matplotlib
from matplotlib import pyplot as plt

print("cv.useOptimized():", cv.useOptimized())
cv.setUseOptimized(False)
print("cv.useOptimized():", cv.useOptimized())
# ROI（Region of Interest，感兴趣区域）是指图像中的一个矩形区域

# 加载两张图片
img1 = cv.imread('../data/messi5.jpg')
img2 = cv.imread('../data/opencv-logo-white.png')
# 我想把logo放在左上角，所以我创建了ROI
rows, cols, channels = img2.shape
roi = img1[0:rows, 0:cols]
# 现在创建logo的掩码，并同时创建其相反掩码
img2gray = cv.cvtColor(img2, cv.COLOR_BGR2GRAY)
ret, mask = cv.threshold(img2gray, 10, 255, cv.THRESH_BINARY)
mask_inv = cv.bitwise_not(mask)
# 现在将ROI中logo的区域涂黑
img1_bg = cv.bitwise_and(roi, roi, mask=mask_inv)
# 仅从logo图像中提取logo区域
img2_fg = cv.bitwise_and(img2, img2, mask=mask)
# 将logo放入ROI并修改主图像
dst = cv.add(img1_bg, img2_fg)
img1[0:rows, 0:cols] = dst
cv.imshow('img2gray', img2gray)
cv.imshow('ret', ret)
cv.imshow('mask', mask)
cv.imshow('res', img1)
cv.waitKey(0)
cv.destroyAllWindows()

img1 = cv.imread('../data/messi5.jpg')
e1 = cv.getTickCount()
for i in range(5, 49, 2):
    img2 = cv.medianBlur(img1, i)
e2 = cv.getTickCount()
t = (e2 - e1) / cv.getTickFrequency()
print(t)
img3 = cv.medianBlur(img1, 5)
cv.imshow('medianBlur', img3)
cv.waitKey(0)
cv.destroyAllWindows()
