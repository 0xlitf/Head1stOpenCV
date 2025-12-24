/*****************************************************************************
name: partstrainer.cpp
date:  2016.8.23(start)
******************************************************************************/
#include "partstrainer.h"
#include "cfg.h"
using namespace cv;

PartsTrainer::PartsTrainer() {
    bool bOk = readSysParamFromXml(CONFIG_PATH);
    // bool bOk = readSysParamFromXml("E:/VC++
    // Project/Counter_halcon/20230609/ok2");
    m_isDynamic = false;
    m_useHalcon = 0;
    m_openElementSize = 7;
    m_roiLTX = 10;
    m_roiLTY = 10;
    m_roiWidth = 100;
    m_roiHeight = 100;
    ptr_HPDetectionPartsTrainer = std::make_shared<HPDetectionPartsTrainer>();

    m_modelID = 0;
    m_trainNum = 3;
    resultList.clear();
    resultData.train_count = 0;
    ModelWidthAll.clear();
    ModelHeightAll.clear();
    ModelIDs.clear();
    ClearAllShapeModels();
}

PartsTrainer::~PartsTrainer() {
    resultList.clear();
    ModelWidthAll.clear();
    ModelHeightAll.clear();
    ModelIDs.clear();
    ClearAllShapeModels();
}

bool PartsTrainer::updateParam_train() {
    // 读取系统参数
    bool bOk = readSysParamFromXml(CONFIG_PATH);
    // bool bOk = readSysParamFromXml("E:/VC++
    // Project/Counter_halcon/20230609/2");
    ptr_HPDetectionPartsTrainer = std::make_shared<HPDetectionPartsTrainer>();
    if (bOk) {
        // 更新参数
        if (!ptr_HPDetectionPartsTrainer->updateParam(mySysParam.threshValue,
                                                      mySysParam.wdiffValue))
            return false;
    } else {

        return false;
    }

    return true;
}

bool PartsTrainer::HPDetection_start(std::string strProject,
                                     std::string roiPath) {
    return ptr_HPDetectionPartsTrainer->start(strProject, roiPath);
}

bool PartsTrainer::HPDetection_train(cv::Mat &image, cv::Mat &trainResult) {
    return ptr_HPDetectionPartsTrainer->train(image, trainResult);
}

bool PartsTrainer::HPDetection_finish() {
    return ptr_HPDetectionPartsTrainer->finish();
}

int PartsTrainer::getDetectObjNum() {
    if (m_useHalcon) {
        return ptr_HPDetectionPartsTrainer->getDetectObjNum();
    } else {
        return resultList.size();
    }
}

int PartsTrainer::getTrainedObjNum() {
    if (m_useHalcon) {
        return ptr_HPDetectionPartsTrainer->getTrainedObjNum();
    } else {
        int cnt = 0;
        for (int i = 0; i < resultList.size(); i++) {
            if ((m_isDynamic == false) || (resultList[i].train_count >= m_trainNum)) {
                cnt++;
            }
        }
        return cnt;
    }
}

cv::Mat PartsTrainer::getTrainedImage() {
    if (m_useHalcon) {
        return ptr_HPDetectionPartsTrainer->getTrainedImage();
    } else {
        cv::Mat outImg;
        for (int i = 0; i < resultList.size(); ++i) {
            int model_id = resultList[i].modelID;
            if (m_modelID == model_id)
                outImg = resultList[i].image.clone();
        }
        return outImg;
    }
}

cv::Size PartsTrainer::getTrainedSize() {
    if (m_useHalcon) {
        return ptr_HPDetectionPartsTrainer->getTrainedSize();
    } else {
        int width, height;
        for (int i = 0; i < resultList.size(); ++i) {
            int model_id = resultList[i].modelID;
            if (m_modelID == model_id) {
                width = resultList[i].modelWidth;
                height = resultList[i].modelHeight;
            }
        }
        return cv::Size(width, height);
    }
}

double PartsTrainer::getCalibFactor(cv::Mat &image) {
    return ptr_HPDetectionPartsTrainer->getCalibFactor(image);
}

bool PartsTrainer::training(cv::Mat *trainImg, Mat *trainResult, int thresh) {
    // 该段代码注释见partscounter.cpp
    CV_Assert(trainImg != NULL);

    isTrained = 0;
    Mat outThreshold;
    Mat _trainImg;
    if (3 == trainImg->channels())
        cvtColor(*trainImg, _trainImg, CV_BGR2GRAY);
    else
        _trainImg = *trainImg;

    trainResult->create(_trainImg.size(), CV_8UC3);
    Mat RoiImage =
        _trainImg(Range(m_roiLTY, min(_trainImg.rows, m_roiLTY + m_roiHeight)),
                             Range(m_roiLTX, min(_trainImg.cols, m_roiLTX + m_roiWidth)));

    threshold(RoiImage, outThreshold, thresh, 255, THRESH_BINARY_INV);
    // 填充中间空洞
    // Mat fill = outThreshold.clone();
    // for (int y = 1; y < fill.rows - 1; y++)
    //{
    //	if (!fill.at<uchar>(y, 90))
    //	{
    //		floodFill(fill, Point(90, y), Scalar(255));
    //		break;
    //	}
    // }
    // Mat invertFill = inverseColor6(fill);   	//图像反转
    // bitwise_or(outThreshold, invertFill, outThreshold);
    // Mat element1 = getStructuringElement(MORPH_ELLIPSE, Size(m_openElementSize,
    // m_openElementSize));    //MORPH_ELLIPSE morphologyEx(outThreshold,
    // outThreshold, MORPH_CLOSE, element1);  //MORPH_CLOSE、MORPH_OPEN

    Mat element1 = getStructuringElement(
        MORPH_RECT, Size(m_openElementSize, m_openElementSize)); // MORPH_ELLIPSE
    // Mat element2 = getStructuringElement(MORPH_RECT, Size(1,
    // m_openElementSize*0.5));
    morphologyEx(outThreshold, outThreshold, MORPH_CLOSE,
                 element1); // MORPH_CLOSE、MORPH_OPEN
    // morphologyEx(outThreshold, outThreshold, MORPH_OPEN, element2);
    // //MORPH_CLOSE、MORPH_OPEN

    // outThreshold.row(0) = uchar(0);
    // outThreshold.row(1) = uchar(0);
    // outThreshold.row(outThreshold.rows - 1) = uchar(0);
    // outThreshold.row(outThreshold.rows - 2) = uchar(0);
    // outThreshold.col(0) = uchar(0);
    // outThreshold.col(1) = uchar(0);
    // outThreshold.col(outThreshold.cols - 1) = uchar(0);
    // outThreshold.col(outThreshold.cols - 2) = uchar(0);

    // 设置边界为0
    outThreshold.row(0).setTo(cv::Scalar(0));
    outThreshold.row(1).setTo(cv::Scalar(0));
    outThreshold.row(outThreshold.rows - 1).setTo(cv::Scalar(0));
    outThreshold.row(outThreshold.rows - 2).setTo(cv::Scalar(0));
    outThreshold.col(0).setTo(cv::Scalar(0));
    outThreshold.col(1).setTo(cv::Scalar(0));
    outThreshold.col(outThreshold.cols - 1).setTo(cv::Scalar(0));
    outThreshold.col(outThreshold.cols - 2).setTo(cv::Scalar(0));

    Canny(outThreshold, outThreshold, 150, 255, 5);

    Mat element = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
    dilate(outThreshold, outThreshold, element);

    if (0 != m_tempContours.size()) {
        m_tempContours.clear();
    }
    std::vector<Vec4i> hierarchy;
    findContours(outThreshold, m_tempContours, hierarchy, CV_RETR_EXTERNAL,
                 CV_CHAIN_APPROX_NONE);
    std::vector<std::vector<Point>>::const_iterator itContours =
        m_tempContours.begin();
    std::vector<Vec4i>::const_iterator itHierarchy = hierarchy.begin();
    while (itContours != m_tempContours.end()) {
        double conArea = contourArea(*itContours);

        if (((conArea > m_roiWidth * m_roiHeight) || (conArea < 100))) {
            itContours = m_tempContours.erase(itContours);
            itHierarchy = hierarchy.erase(itHierarchy);
        } else {
            ++itContours;
            ++itHierarchy;
        }
    }
    // unsigned int num = unsigned int(m_tempContours.size());
    unsigned int num = static_cast<unsigned int>(m_tempContours.size());

    // 转换到BGR格式下，才能绘制彩色轮廓
    cvtColor(_trainImg, *trainResult, CV_GRAY2BGR);
    // 绘制ROI边框
    line(*trainResult, Point(m_roiLTX, m_roiLTY),
         Point((m_roiLTX + m_roiWidth), m_roiLTY), Scalar(0, 255, 0));
    line(*trainResult, Point((m_roiLTX + m_roiWidth), m_roiLTY),
         Point((m_roiLTX + m_roiWidth), m_roiLTY + m_roiHeight),
         Scalar(0, 255, 0));
    line(*trainResult, Point((m_roiLTX + m_roiWidth), m_roiLTY + m_roiHeight),
         Point(m_roiLTX, m_roiLTY + m_roiHeight), Scalar(0, 255, 0));
    line(*trainResult, Point(m_roiLTX, m_roiLTY + m_roiHeight),
         Point(m_roiLTX, m_roiLTY), Scalar(0, 255, 0));
    line(*trainResult, Point(m_roiLTX + m_roiWidth / 2, 0),
         Point(m_roiLTX + m_roiWidth / 2, mySysParam.imgHeight),
         Scalar(0, 0, 255)); // ROI中轴线-竖线
    line(*trainResult, Point(0, m_roiLTY + (m_roiHeight / 2)),
         Point(mySysParam.imgWidth, m_roiLTY + (m_roiHeight / 2)),
         Scalar(0, 0, 255)); // ROI中轴线-横线

    if (m_isOutBoundry > 0)
        m_isOutBoundry--;

    if (0 == num) {
        return false;
    } else {
        // 对物料进行排序，取最左边物料进行判断
        if (m_tempContours.size() > 1)
            sort(m_tempContours.begin(), m_tempContours.end(), compContours);

        m_objArea = fabs(contourArea(m_tempContours[0]));

        RotatedRect r0 = minAreaRect(m_tempContours[0]);
        if (r0.size.width > r0.size.height) {
            m_objHeight = int(r0.size.height);
            m_objWidth = int(r0.size.width);
        } else {
            m_objHeight = int(r0.size.width);
            m_objWidth = int(r0.size.height);
        }

        // r1 为目标轮廓的水平外接矩形
        Rect r1 = boundingRect(m_tempContours[0]);
        int Left = r1.x + r1.width + m_roiLTX;

        if ((Left > m_roiLTX + m_roiWidth - 5) || (m_objArea < 30)) {
            return false;
        } else if (r1.x < 15 * _trainImg.cols / 320) {
            // 如果出视野结果为3
            isTrained = 3;
            if ((Left < m_roiLTX + 15 * _trainImg.cols / 320) &&
                (m_useCountDebug == true))
                trainCountAdd(); // 调试时代替光纤计数
            return false;
        }

        if (m_isDynamic == false) {
            // 只有一个训练物料时，训练正确
            if (1 == num) {
                isTrained = 1;
                m_modelID = ModelWidthAll.size() + 1; // 默认用新ID
            }
            // 检测到超过一个训练物料时，训练错误，多料
            else if (num > 1) {
                // 红色
                Scalar color(0, 0, 255);
                drawContours(*trainResult, m_tempContours, -1, color, 1, 8, hierarchy,
                             INT_MAX, Point(m_roiLTX, m_roiLTY));
            }
        } else {
            // 多个料评估最前（左）面的料是否训练过
            estimateTrainObj();
        }

        if (isTrained == 1) {
            // 绿色
            Scalar color(0, 255, 30);
            drawContours(*trainResult, m_tempContours, -1, color, 2, 8, hierarchy,
                         INT_MAX, Point(m_roiLTX, m_roiLTY));
            trainResult->copyTo(resultData.image);
            // if (m_isDynamic == true)
            saveData();
            return true;
        } else {
            // 洋红色
            Scalar color(255, 0, 255); /// 248, 170, 221
            drawContours(*trainResult, m_tempContours, -1, color, 2, 8, hierarchy,
                         INT_MAX, Point(m_roiLTX, m_roiLTY));
        }
    }
    return false;
}

void PartsTrainer::estimateTrainObj() {
    // m_trainedWidth 已训练目标
    // m_tempContours   正在检测目标
    float min_aDiffValue = 1.0;
    isTrained = 1;                        // 未训练料
    m_modelID = ModelWidthAll.size() + 1; // 默认用新ID
    for (int i = 0; i < ModelWidthAll.size(); ++i) {
        if ((abs(m_objHeight - ModelHeightAll[i]) < mySysParam.wdiffValue) &&
            (abs(m_objWidth - ModelWidthAll[i]) < mySysParam.wdiffValue)) {
            // 计算模板面积=
            // 训练面积+（（目标高-模板高）*模板宽）+（（目标宽-模板宽）*模板高）
            // 设计该计算方式的意义？ Question-01
            // A：解决不同相机高度可能存在放大倍数误差问题，起到补偿作用
            float tempArea =
                m_objArea + ((ModelHeightAll[i] - m_objHeight - 1) / 2 * m_objWidth) +
                             ((ModelWidthAll[i] - m_objWidth - 1) / 2 * m_objHeight);
            // 如果目标面积范围在 模板面积+/-公差面积（现为0.15）以内，则视为合格料
            // if ((((tempArea - m_objArea[i]< m_allowContourArea1*2)) && ((tempArea +
            // m_objArea[i]) > m_allowContourArea1 *2)))
            float aDiffValue = fabs(tempArea - ModelAreaAll[i]) / tempArea;
            if (aDiffValue < mySysParam.adiffValue) {
                // 将合格的轮廓加入m_objContour
                isTrained = 2; // 已训练料
                m_modelID = i + 1;
                break;
            }
            if (min_aDiffValue > aDiffValue)
                min_aDiffValue = aDiffValue;
        }
    }
    if (isTrained == 1) {
        int a = 1;
    }
}

void PartsTrainer::saveData() {
    if (m_tempContours.size() > 0) {
        resultData.modelWidth = m_objWidth;
        resultData.modelHeight = m_objHeight;
        resultData.modelArea = m_objArea;
        resultData.modelID = m_modelID;
        resultData.train_count = 0; // 重置光纤计数器，掉下才计数，刚创建不计数

        ModelWidthAll.push_back(m_objWidth);
        ModelHeightAll.push_back(m_objHeight);
        ModelAreaAll.push_back(int(m_objArea)); // 保存训练对象的面积
        ModelIDs.push_back(m_modelID);
        m_trainContours.push_back(m_tempContours[0]); // 保存训练对象的轮廓

        resultList.push_back(resultData);
    } else {
        std::cout << "There is no data to save !!" << std::endl;
    }
}

/*
void PartsTrainer::saveData()
{
        if (m_tempContours.size() > 0)
        {
                m_trainContours.push_back(m_tempContours[0]);
//$)A!@??????????(.??????
                m_objArea.push_back(int(fabs(contourArea(m_tempContours[0]))));
//$)A!@??????????(.?????? RotatedRect r0 = minAreaRect(m_tempContours[0]);
//$)A!@??????????(.???!h?(* if (r0.size.width > r0.size.height)
                {
                        m_objHeight.push_back(int(r0.size.height));
                        m_objWidth.push_back(int(r0.size.width));
                }
                else
                {
                        m_objHeight.push_back(int(r0.size.width));
                        m_objWidth.push_back(int(r0.size.height));
                }
        }
        else{
                std::cout << "There is no data to save !!" << std::endl;
        }
}*/

void PartsTrainer::dynamicTrainModeOpen() {
    m_isDynamic = true;
    ptr_HPDetectionPartsTrainer->HD_dynamicTrainMode(m_isDynamic);
}

void PartsTrainer::dynamicTrainModeClose() {
    m_isDynamic = false;
    ptr_HPDetectionPartsTrainer->HD_dynamicTrainMode(m_isDynamic);
}

void PartsTrainer::setCountDebug() { m_useCountDebug = true; }

void PartsTrainer::setHalonMode(bool flag) { m_useHalcon = flag; }

void PartsTrainer::trainCountAdd() {
    if (m_useHalcon) {
        ptr_HPDetectionPartsTrainer->trainCountAdd();
    } else {
        // 如果视觉调试 或者 料不在出视野边界时光纤计数 都有效
        if (m_isOutBoundry == 0) {
            for (int i = 0; i < resultList.size(); ++i) {
                int model_id = resultList[i].modelID;
                if (m_modelID == model_id) {
                    m_isOutBoundry = 4;
                    resultList[i].train_count++;
                    std::cout << "matchID: " << model_id
                              << "  train_count:" << resultList[i].train_count
                              << std::endl;
                }
            }
        }
    }
}

void PartsTrainer::setTrainTarget(int num) {
    m_trainNum = num;
    ptr_HPDetectionPartsTrainer->setTrainTarget(num);
}

bool PartsTrainer::writeTrainDataToXml() {
    _TrainData trainResults;

    for (int i = 0; i < resultList.size(); ++i) {
        std::cout << "trainingID: " << i
                  << "         accumulate times:" << resultList[i].train_count
                  << std::endl;
        if ((m_isDynamic == false) || (resultList[i].train_count >=
                                       m_trainNum)) // 训练次数超过3次的就保存结果
        {
            trainResults.trainContours.push_back(m_trainContours[i]);
            trainResults.trainArea.push_back(ModelAreaAll[i]);
            trainResults.trainWidth.push_back(ModelWidthAll[i]);
            trainResults.trainHeight.push_back(ModelHeightAll[i]);
        }
    }
    saveTrainDataXML(trainResults, m_strProjectPath);

    return true;
}

void PartsTrainer::cleanData() {
    m_trainContours.clear();
    ModelAreaAll.clear();
    ModelWidthAll.clear();
    ModelHeightAll.clear();
    resultList.clear();
}

void PartsTrainer::reset(std::string strProjectPath, std::string strRoiPath) {
    m_strProjectPath = strProjectPath;
    // vector<string> fileLists, regionFiles, modelFiles;
    // getFiles(strProjectPath, fileLists);

    ////设置文件过滤格式,将过滤后的文件名称存入到列表中
    // regionFiles = fileFilter(fileLists, ".reg");
    // modelFiles = fileFilter(fileLists, ".shm");

    // for (auto& regionPath : regionFiles)
    //{
    //	std::string path = m_strProjectPath + "/" + regionPath;
    // }
    // for (auto& modelPath : modelFiles)
    //{
    //	std::string path = m_strProjectPath + "/" + modelPath;
    // }

    RoiData roiData;
    if (readRoiDataFromXml(strRoiPath, &roiData)) {
        m_roiLTX = roiData.roiLTX;
        m_roiLTY = roiData.roiLTY;
        m_roiWidth = roiData.roiWidth;
        m_roiHeight = roiData.roiHeight;
    }

    cleanData();
}

bool PartsTrainer::writeImage(cv::Mat *img) {
    // 保存图片
    string strImagePath =
        m_strProjectPath + "/train_" + to_string(m_trainContours.size()) + ".jpg";
    vector<unsigned char> inImage;
    cv::imencode(".jpg", *img, inImage);
    return cv::imwrite(strImagePath, *img);
}

void PartsTrainer::run(cv::Mat *img, std::string strProjectPath,
                       std::string strRoiPath) {
    reset(strProjectPath, strRoiPath);
    Mat trainResult;
    training(img, &trainResult, mySysParam.threshValue);
    cv::imshow("trainResult", trainResult);
    cv::waitKey(0);

    saveData();
    writeTrainDataToXml();
    cleanData();
}
