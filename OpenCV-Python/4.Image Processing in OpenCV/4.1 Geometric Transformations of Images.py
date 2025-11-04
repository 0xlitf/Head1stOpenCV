import numpy as np
import cv2 as cv
from PyQt6.QtWidgets import QApplication, QMessageBox
from PyQt6.QtCore import QFile
import matplotlib
from matplotlib import pyplot as plt

matplotlib.use('QtAgg')

img = cv.imread('../data/messi5.jpg')
res1 = cv.resize(img, None, fx=2, fy=2, interpolation=cv.INTER_CUBIC)
cv.imshow("res1", res1)
cv.waitKey(0)
cv.destroyAllWindows()
# 或者
height, width = img.shape[:2]
res2 = cv.resize(img, (2 * width, 2 * height), interpolation=cv.INTER_CUBIC)
cv.imshow("res2", res2)
cv.waitKey(0)
cv.destroyAllWindows()

rows, cols = img.shape[:2]
M = np.float32([[1, 0, 100], [0, 1, 50]])
dst = cv.warpAffine(img, M, (cols, rows))
cv.imshow('dst', dst)
cv.waitKey(0)
cv.destroyAllWindows()

rows, cols = img.shape[:2]
# cols-1 和 rows-1 是坐标限制
M = cv.getRotationMatrix2D(((cols - 1) / 2.0, (rows - 1) / 2.0), 90, 1)
rotate = cv.warpAffine(img, M, (cols, rows))
cv.imshow('rotate', rotate)
cv.waitKey(0)
cv.destroyAllWindows()

img = cv.imread('../data/tmpl.png')
rows, cols, _ = img.shape
pts1 = np.float32([[50, 50], [200, 50], [50, 200]])
pts2 = np.float32([[10, 100], [200, 50], [100, 250]])
M = cv.getAffineTransform(pts1, pts2)
dst = cv.warpAffine(img, M, (cols, rows))
plt.subplot(121), plt.imshow(img), plt.title('Input')
plt.subplot(122), plt.imshow(dst), plt.title('Output')
plt.show()

img = cv.imread('../data/sudoku.png')
rows, cols, _ = img.shape
pts1 = np.float32([[56, 65], [368, 52], [28, 387], [389, 390]])
pts2 = np.float32([[0, 0], [300, 0], [0, 300], [300, 300]])
M = cv.getPerspectiveTransform(pts1, pts2)
dst = cv.warpPerspective(img, M, (300, 300))
plt.subplot(121), plt.imshow(img), plt.title('Input')
plt.subplot(122), plt.imshow(dst), plt.title('Output')
plt.show()
