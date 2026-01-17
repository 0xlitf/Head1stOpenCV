#ifndef RECTIFICATION_H
#define RECTIFICATION_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
// #include <opencv2/xfeatures2d.hpp>

class Rectification : public QObject {
    Q_OBJECT
public:
    explicit Rectification(QObject *parent = nullptr);

    // 使用ORB特征进行图像配准
    cv::Mat alignImageWithORB(const cv::Mat& standardImg, const cv::Mat& defectImg,
                              cv::Mat& alignedDefect, bool showMatches = false) {
        // 1. 转换为灰度图（如果输入是彩色图）
        cv::Mat grayStandard, grayDefect;
        if (standardImg.channels() == 3) {
            cv::cvtColor(standardImg, grayStandard, cv::COLOR_BGR2GRAY);
            cv::cvtColor(defectImg, grayDefect, cv::COLOR_BGR2GRAY);
        } else {
            grayStandard = standardImg.clone();
            grayDefect = defectImg.clone();
        }

        // 2. 初始化ORB检测器
        cv::Ptr<cv::ORB> orb = cv::ORB::create(1000); // 设置特征点数量

        // 3. 检测关键点和计算描述符
        std::vector<cv::KeyPoint> keypoints1, keypoints2;
        cv::Mat descriptors1, descriptors2;

        orb->detectAndCompute(grayStandard, cv::noArray(), keypoints1, descriptors1);
        orb->detectAndCompute(grayDefect, cv::noArray(), keypoints2, descriptors2);

        std::cout << "标准图检测到 " << keypoints1.size() << " 个特征点" << std::endl;
        std::cout << "缺陷图检测到 " << keypoints2.size() << " 个特征点" << std::endl;

        // 4. 特征点匹配（使用FLANN或暴力匹配）
        cv::Ptr<cv::DescriptorMatcher> matcher;
        if (descriptors1.type() == CV_8U) {
            // ORB描述符是二进制描述符，使用汉明距离
            matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::BRUTEFORCE_HAMMING);
        } else {
            // SIFT/SURF描述符是浮点型，使用FLANN
            matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
        }

        std::vector<std::vector<cv::DMatch>> knnMatches;
        matcher->knnMatch(descriptors1, descriptors2, knnMatches, 2);

        // 5. Lowe's ratio test筛选优质匹配
        std::vector<cv::DMatch> goodMatches;
        const float ratio_thresh = 0.75f; // Lowe's ratio阈值

        for (size_t i = 0; i < knnMatches.size(); i++) {
            if (knnMatches[i].size() == 2) {
                if (knnMatches[i][0].distance < ratio_thresh * knnMatches[i][1].distance) {
                    goodMatches.push_back(knnMatches[i][0]);
                }
            }
        }

        std::cout << "筛选后优质匹配点数量: " << goodMatches.size() << std::endl;

        // 6. 显示匹配结果（可选）
        if (showMatches && goodMatches.size() > 10) {
            cv::Mat matchImg;
            cv::drawMatches(standardImg, keypoints1, defectImg, keypoints2,
                            goodMatches, matchImg, cv::Scalar::all(-1),
                            cv::Scalar::all(-1), std::vector<char>(),
                            cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
            cv::imshow("特征点匹配结果", matchImg);
            cv::waitKey(1);
        }

        // 7. 提取匹配点的坐标
        std::vector<cv::Point2f> srcPoints, dstPoints;
        for (size_t i = 0; i < goodMatches.size(); i++) {
            srcPoints.push_back(keypoints1[goodMatches[i].queryIdx].pt);
            dstPoints.push_back(keypoints2[goodMatches[i].trainIdx].pt);
        }

        // 8. 计算单应性矩阵（使用RANSAC去除异常值）
        cv::Mat H;
        if (goodMatches.size() >= 4) {
            H = cv::findHomography(dstPoints, srcPoints, cv::RANSAC, 3.0);
            std::cout << "单应性矩阵 H:\n" << H << std::endl;

            // 9. 应用透视变换矫正缺陷图像
            cv::warpPerspective(defectImg, alignedDefect, H, standardImg.size(),
                                cv::INTER_LINEAR + cv::WARP_INVERSE_MAP);

            // 10. 计算重投影误差评估配准质量
            double reprojectionError = 0;
            for (size_t i = 0; i < srcPoints.size(); i++) {
                cv::Mat pt(3, 1, CV_64F);
                pt.at<double>(0) = dstPoints[i].x;
                pt.at<double>(1) = dstPoints[i].y;
                pt.at<double>(2) = 1.0;

                cv::Mat transformedPt = H * pt;
                transformedPt /= transformedPt.at<double>(2);

                double dx = transformedPt.at<double>(0) - srcPoints[i].x;
                double dy = transformedPt.at<double>(1) - srcPoints[i].y;
                reprojectionError += sqrt(dx*dx + dy*dy);
            }
            reprojectionError /= srcPoints.size();
            std::cout << "平均重投影误差: " << reprojectionError << " 像素" << std::endl;

        } else {
            std::cerr << "匹配点数量不足，无法计算单应性矩阵（需要至少4个匹配点）" << std::endl;
            H = cv::Mat::eye(3, 3, CV_64F); // 返回单位矩阵
            alignedDefect = defectImg.clone();
        }

        return H;
    }

};

#endif // RECTIFICATION_H
