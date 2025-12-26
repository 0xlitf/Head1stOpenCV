import cv2
import numpy as np
from centroidtracker import CentroidTracker

folder_path = 'track_pictures'

# 获取文件夹内所有图片文件
image_files = [f for f in os.listdir(folder_path) if f.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp'))]

# 【重要】排序：必须确保图片是按时间顺序排列的
# 如果文件名是 frame_1.jpg, frame_10.jpg，直接 sort 会乱序 (1, 10, 2)
# 这里假设文件名包含数字，按数字大小排序；如果文件名规范(如 001, 002)，直接 .sort() 即可
try:
    image_files.sort(key=lambda f: int(''.join(filter(str.isdigit, f))))
except:
    image_files.sort()  # 如果没有数字，回退到默认排序

print(f"检测到 {len(image_files)} 张图片，开始处理...")

# 初始化跟踪器和背景减除器
fgbg = cv2.createBackgroundSubtractorMOG2(history=500, varThreshold=50, detectShadows=False)
tracker = CentroidTracker(maxDisappeared=40)

count_line_y = 300
total_counts = 0
trackable_objects = {}

# 改用 for 循环遍历图片列表
for img_name in image_files:
    full_path = os.path.join(folder_path, img_name)
    frame = cv2.imread(full_path)

    if frame is None:
        print(f"无法读取图片: {img_name}")
        continue

    # 调整大小，保持与之前逻辑一致
    frame = cv2.resize(frame, (600, 400))

    # --- 下面的处理逻辑与视频处理完全一致 ---

    # 1. 预处理 & 检测
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    blur = cv2.GaussianBlur(gray, (5, 5), 0)
    mask = fgbg.apply(blur)
    _, mask = cv2.threshold(mask, 200, 255, cv2.THRESH_BINARY)
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    rects = []
    for cnt in contours:
        if cv2.contourArea(cnt) > 500:  # 面积阈值
            x, y, w, h = cv2.boundingRect(cnt)
            rects.append((x, y, x + w, y + h))
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)

    # 2. 跟踪
    objects = tracker.update(rects)

    # 3. 计数 & 绘制
    cv2.line(frame, (0, count_line_y), (600, count_line_y), (0, 0, 255), 2)

    for (objectID, centroid) in objects.items():
        to = trackable_objects.get(objectID, [])

        if len(to) > 0:
            prev_y = to[-1][1]
            curr_y = centroid[1]

            if prev_y < count_line_y and curr_y >= count_line_y:
                total_counts += 1

        if objectID not in trackable_objects:
            trackable_objects[objectID] = []
        trackable_objects[objectID].append(centroid)

        text = "ID {}".format(objectID)
        cv2.putText(frame, text, (centroid[0] - 10, centroid[1] - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
        cv2.circle(frame, (centroid[0], centroid[1]), 4, (0, 255, 0), -1)

    cv2.putText(frame, f"Count: {total_counts}", (10, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)

    cv2.imshow("Frame", frame)

    # 【重要】这里改为等待 100ms (0.1秒)，模拟每秒10帧的播放速度
    # 如果设为 0，则需要按任意键才切换下一张
    key = cv2.waitKey(100) & 0xFF
    if key == 27:  # ESC 退出
        break

cv2.destroyAllWindows()
