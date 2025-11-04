import cv2 as cv
import matplotlib
import numpy as np
import copy
from matplotlib import pyplot as plt

matplotlib.use("QtAgg")

img = cv.imread('../data/messi5.jpg', 0)
A = cv.imread('../data/apple.jpg')
B = cv.imread('../data/orange.jpg')
# 生成A的高斯金字塔
G = A.copy()
gpA = [G]
for i in range(6):
    G = cv.pyrDown(G)
    gpA.append(G)
# 生成B的高斯金字塔
G = B.copy()
gpB = [G]
for i in range(6):
    G = cv.pyrDown(G)
    gpB.append(G)
# 生成A的拉普拉斯金字塔
lpA = [gpA[5]]
for i in range(5, 0, -1):
    GE = cv.pyrUp(gpA[i])
    L = cv.subtract(gpA[i - 1], GE)
    lpA.append(L)
# 生成B的拉普拉斯金字塔
lpB = [gpB[5]]
for i in range(5, 0, -1):
    GE = cv.pyrUp(gpB[i])
    L = cv.subtract(gpB[i - 1], GE)
    lpB.append(L)
# 现在在每个级别中添加左右两半图像 
LS = []
for la, lb in zip(lpA, lpB):
    rows, cols, dpt = la.shape
    col = int(cols / 2)
    ls = np.hstack((la[:, 0:col], lb[:, col:]))
    LS.append(ls)
# 现在重建
ls_ = LS[0]
for i in range(1, 6):
    ls_ = cv.pyrUp(ls_)
    ls_ = cv.add(ls_, LS[i])
# 图像与直接连接的每一半
real = np.hstack((A[:, :col], B[:, col:]))
cv.imwrite('Pyramid_blending2.jpg', ls_)
cv.imwrite('Direct_blending.jpg', real)
## cv.cvtColor(img, cv.COLOR_BGR2RGB)
plt.subplot(121), plt.imshow(cv.cvtColor(real, cv.COLOR_BGR2RGB))
plt.title('real'), plt.xticks([]), plt.yticks([])
plt.subplot(122), plt.imshow(cv.cvtColor(ls_, cv.COLOR_BGR2RGB))
plt.title('ls_'), plt.xticks([]), plt.yticks([])
plt.show()
