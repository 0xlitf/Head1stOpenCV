import cv2
import numpy as np
from centroidtracker import CentroidTracker

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
