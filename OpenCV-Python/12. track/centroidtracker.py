import cv2
import numpy as np
import math


# --- 1. 定义简单的质心跟踪器 ---
class CentroidTracker:
    def __init__(self, maxDisappeared=50):
        self.nextObjectID = 0
        self.objects = {}  # 存储 ID: Centroid
        self.disappeared = {}  # 存储 ID: 消失帧数
        self.maxDisappeared = maxDisappeared

    def register(self, centroid):
        self.objects[self.nextObjectID] = centroid
        self.disappeared[self.nextObjectID] = 0
        self.nextObjectID += 1

    def deregister(self, objectID):
        del self.objects[objectID]
        del self.disappeared[objectID]

    def update(self, rects):
        if len(rects) == 0:
            for objectID in list(self.disappeared.keys()):
                self.disappeared[objectID] += 1
                if self.disappeared[objectID] > self.maxDisappeared:
                    self.deregister(objectID)
            return self.objects

        inputCentroids = np.zeros((len(rects), 2), dtype="int")
        for (i, (startX, startY, endX, endY)) in enumerate(rects):
            cX = int((startX + endX) / 2.0)
            cY = int((startY + endY) / 2.0)
            inputCentroids[i] = (cX, cY)

        if len(self.objects) == 0:
            for i in range(0, len(inputCentroids)):
                self.register(inputCentroids[i])
        else:
            objectIDs = list(self.objects.keys())
            objectCentroids = list(self.objects.values())

            # 计算当前帧质心与已存质心的距离
            D = []
            for sublist in inputCentroids:
                row = []
                for sublist2 in objectCentroids:
                    dist = math.hypot(sublist2[0] - sublist[0], sublist2[1] - sublist[1])
                    row.append(dist)
                D.append(row)
            D = np.array(D)

            rows = D.min(axis=1).argsort()
            cols = D.argmin(axis=1)[rows]

            usedRows = set()
            usedCols = set()

            for (row, col) in zip(rows, cols):
                if row in usedRows or col in usedCols:
                    continue

                objectID = objectIDs[col]
                self.objects[objectID] = inputCentroids[row]
                self.disappeared[objectID] = 0
                usedRows.add(row)
                usedCols.add(col)

            unusedRows = set(range(0, D.shape[0])).difference(usedRows)
            unusedCols = set(range(0, D.shape[1])).difference(usedCols)

            if D.shape[0] >= D.shape[1]:
                for row in unusedRows:
                    self.register(inputCentroids[row])
            else:
                for col in unusedCols:
                    objectID = objectIDs[col]
                    self.disappeared[objectID] += 1
                    if self.disappeared[objectID] > self.maxDisappeared:
                        self.deregister(objectID)

        return self.objects


# --- 2. 主程序 ---

# 初始化
cap = cv2.VideoCapture('test_video.mp4')  # 替换为你的视频路径或 0 (摄像头)
fgbg = cv2.createBackgroundSubtractorMOG2(history=500, varThreshold=50, detectShadows=False)
tracker = CentroidTracker(maxDisappeared=40)

# 计数线设置
count_line_y = 300
total_counts = 0
trackable_objects = {}  # 存储每个 ID 的历史路径

while True:
    ret, frame = cap.read()
    if not ret: break

    frame = cv2.resize(frame, (600, 400))

    # 1. 预处理 & 检测
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    blur = cv2.GaussianBlur(gray, (5, 5), 0)
    mask = fgbg.apply(blur)
    _, mask = cv2.threshold(mask, 200, 255, cv2.THRESH_BINARY)
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    rects = []
    for cnt in contours:
        if cv2.contourArea(cnt) > 500:  # 过滤噪点
            x, y, w, h = cv2.boundingRect(cnt)
            rects.append((x, y, x + w, y + h))
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)

    # 2. 跟踪
    objects = tracker.update(rects)

    # 3. 计数逻辑
    cv2.line(frame, (0, count_line_y), (600, count_line_y), (0, 0, 255), 2)

    for (objectID, centroid) in objects.items():
        # 获取该物体之前的质心位置（如果有）
        to = trackable_objects.get(objectID, [])

        if len(to) > 0:
            prev_y = to[-1][1]
            curr_y = centroid[1]

            # 判断是否越线 (假设物体向下移动: 上一帧 < 线, 当前帧 >= 线)
            # 如果物体向上移动，逻辑需反过来
            if prev_y < count_line_y and curr_y >= count_line_y:
                total_counts += 1
                cv2.line(frame, (0, count_line_y), (600, count_line_y), (0, 255, 255), 3)  # 越线变色

        # 更新历史轨迹
        if objectID not in trackable_objects:
            trackable_objects[objectID] = []
        trackable_objects[objectID].append(centroid)

        # 绘制 ID 和中心点
        text = "ID {}".format(objectID)
        cv2.putText(frame, text, (centroid[0] - 10, centroid[1] - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
        cv2.circle(frame, (centroid[0], centroid[1]), 4, (0, 255, 0), -1)

    cv2.putText(frame, f"Count: {total_counts}", (10, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
    cv2.imshow("Frame", frame)
    # cv2.imshow("Mask", mask) # 调试用

    if cv2.waitKey(30) & 0xFF == 27:  # ESC 退出
        break

cap.release()
cv2.destroyAllWindows()