import numpy as np
import cv2 as cv
from matplotlib import pyplot as plt

img = cv.imread('../data/opencv-logo-white.png')
img0 = cv.cvtColor(img, cv.COLOR_BGR2RGB)
kernel = np.ones((5, 5), np.float32) / 25
dst = cv.filter2D(img0, -1, kernel)
plt.subplot(121), plt.imshow(img0), plt.title('Original')
plt.xticks([]), plt.yticks([])
plt.subplot(122), plt.imshow(dst), plt.title('Averaging')
plt.xticks([]), plt.yticks([])
plt.show()

# OpenCV主要提供四种类型的模糊技术
# 1.平均
img1 = cv.cvtColor(img, cv.COLOR_BGR2RGB)
blur = cv.blur(img1, (5, 5))
plt.subplot(121), plt.imshow(img1), plt.title('Original')
plt.xticks([]), plt.yticks([])
plt.subplot(122), plt.imshow(blur), plt.title('Average Blurred')
plt.xticks([]), plt.yticks([])
plt.show()

# 2.高斯模糊
img2 = cv.cvtColor(img, cv.COLOR_BGR2RGB)
blur = cv.GaussianBlur(img2, (5, 5), 0)
plt.subplot(121), plt.imshow(img2), plt.title('Original')
plt.xticks([]), plt.yticks([])
plt.subplot(122), plt.imshow(blur), plt.title('Gaussian Blur')
plt.xticks([]), plt.yticks([])
plt.show()

# 3.中位模糊
img3 = cv.imread('../data/opencv-logo-white-50noise.png')
img3 = cv.cvtColor(img3, cv.COLOR_BGR2RGB)
median = cv.medianBlur(img3, 5)
plt.subplot(121), plt.imshow(img3), plt.title('Original')
plt.xticks([]), plt.yticks([])
plt.subplot(122), plt.imshow(median), plt.title('Median Blur')
plt.xticks([]), plt.yticks([])
plt.show()

# 4.双边滤波
img4 = cv.imread('../data/texture.png')
img4 = cv.cvtColor(img4, cv.COLOR_BGR2RGB)
blur = cv.bilateralFilter(img4, 9, 75, 75)
plt.subplot(121), plt.imshow(img4), plt.title('Original')
plt.xticks([]), plt.yticks([])
plt.subplot(122), plt.imshow(blur), plt.title('Bilateral Blur')
plt.xticks([]), plt.yticks([])
plt.show()
