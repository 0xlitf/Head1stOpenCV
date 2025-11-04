# 不同的形态学操作，例如侵蚀，膨胀，开运算，闭运算
# https://homepages.inf.ed.ac.uk/rbf/HIPR2/morops.htm
import cv2 as cv
import matplotlib
import numpy as np
from matplotlib import pyplot as plt

matplotlib.use("QtAgg")

img = cv.imread('../data/j.png', 0)
kernel = np.ones((5, 5), np.uint8)
erosion = cv.erode(img, kernel, iterations=1)
dilation = cv.dilate(img, kernel, iterations=1)

opening_img = cv.imread('../data/opening.png', 0)
opening = cv.morphologyEx(img, cv.MORPH_OPEN, kernel)

closing_img = cv.imread('../data/closing.png', 0)
closing = cv.morphologyEx(img, cv.MORPH_CLOSE, kernel)

# 5. 形态学梯度
gradient = cv.morphologyEx(img, cv.MORPH_GRADIENT, kernel)

kernel_9x9 = np.ones((9, 9), np.uint8)

# 6. 顶帽
tophat = cv.morphologyEx(img, cv.MORPH_TOPHAT, kernel_9x9)  # 改变内核大小将改变输出形状

# 7. 黑帽
blackhat = cv.morphologyEx(img, cv.MORPH_BLACKHAT, kernel_9x9)

# cv.imshow('j', img)
# cv.waitKey(0)
plt.subplot(341), plt.imshow(img), plt.title('Original')
plt.subplot(342), plt.imshow(erosion), plt.title('erosion')
plt.subplot(343), plt.imshow(opening_img), plt.title('Original')
plt.subplot(344), plt.imshow(opening), plt.title('opening')
plt.subplot(345), plt.imshow(closing_img), plt.title('Original')
plt.subplot(346), plt.imshow(closing), plt.title('closing')
plt.subplot(347), plt.imshow(img), plt.title('Original')
plt.subplot(348), plt.imshow(gradient), plt.title('gradient')
plt.subplot(349), plt.imshow(img), plt.title('Original')
plt.subplot(3, 4, 10), plt.imshow(tophat), plt.title('tophat')
plt.subplot(3, 4, 11), plt.imshow(img), plt.title('Original')
plt.subplot(3, 4, 12), plt.imshow(blackhat), plt.title('blackhat')
plt.show()

# 矩形内核
print(cv.getStructuringElement(cv.MORPH_RECT, (5, 5)))
# array([[1, 1, 1, 1, 1],
#        [1, 1, 1, 1, 1],
#        [1, 1, 1, 1, 1],
#        [1, 1, 1, 1, 1],
#        [1, 1, 1, 1, 1]], dtype=uint8)
# 椭圆内核
print(cv.getStructuringElement(cv.MORPH_ELLIPSE, (5, 5)))
# array([[0, 0, 1, 0, 0],
#        [1, 1, 1, 1, 1],
#        [1, 1, 1, 1, 1],
#        [1, 1, 1, 1, 1],
#        [0, 0, 1, 0, 0]], dtype=uint8)
# 十字内核
print(cv.getStructuringElement(cv.MORPH_CROSS, (5, 5)))
# array([[0, 0, 1, 0, 0],
#        [0, 0, 1, 0, 0],
#        [1, 1, 1, 1, 1],
#        [0, 0, 1, 0, 0],
#        [0, 0, 1, 0, 0]], dtype=uint8)
