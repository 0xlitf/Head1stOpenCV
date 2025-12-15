#include "hp_detection_partstrainer_dynamic.h"

HPDetectionPartsTrainer_Dynamic::HPDetectionPartsTrainer_Dynamic()
    : m_pData(NULL)
{
    if (m_pData == NULL)
        m_pData = new TrainData;

    init();
}

HPDetectionPartsTrainer_Dynamic::~HPDetectionPartsTrainer_Dynamic()
{
    uninit();

    if (m_pData != NULL)
    {
        delete m_pData;
        m_pData = NULL;
    }
}

void HPDetectionPartsTrainer_Dynamic::init()
{
    m_pData->bTraining = false;
    try
    {
        pDevEngine = std::make_shared<HDevEngine>();
        pDevEngine->AddProcedurePath("./config");

        pDevProcedure = std::make_shared<HDevProcedure>("generate_shape_model_dynamic");
        pDevProcedureCall = std::make_shared<HDevProcedureCall>(*pDevProcedure);

        int width = 320, height = 240;
        open_window(0, 0, width, height, "root", "buffer", "", &m_pData->WindowHandle);
        set_part(m_pData->WindowHandle, 0, 0, height - 1, width - 1);

        m_pData->strError.clear();
    }
    catch (HDevEngineException &e)
    {
        m_pData->strError = e.Message();
    }
    catch (HException &e)
    {
        m_pData->strError = e.message;
    }
    catch (...)
    {
        m_pData->strError = "init fail";
    }

    if (!m_pData->strError.isEmpty())
        uninit();
}

void HPDetectionPartsTrainer_Dynamic::uninit()
{
    close_window(m_pData->WindowHandle);
}

QString HPDetectionPartsTrainer_Dynamic::project()
{
    return m_pData->strProject;
}

void HPDetectionPartsTrainer_Dynamic::setProject(QString strProject)
{
    m_pData->strProject = strProject;
}


bool HPDetectionPartsTrainer_Dynamic::start(QString strProject, QRect bgRoi)
{
    if (pDevProcedureCall == NULL)
        return false;

    m_pData->ModelHeightAll.Reset();
    m_pData->ModelWidthAll.Reset();

    m_pData->resultList.clear();
    m_pData->resultData.train_count = 0;
    m_pData->bgRoi      = bgRoi;
    m_pData->strProject = strProject;
    m_pData->bTraining  = true;
    return true;
}

void HPDetectionPartsTrainer_Dynamic::stop()
{
    m_pData->bTraining = false;
}

bool HPDetectionPartsTrainer_Dynamic::finish(const QString &strPath, bool bIsHPdetection)
{
    if (pDevProcedureCall == NULL)
        return false;

    if (!m_pData->bTraining)
        return false;

    if (m_pData->resultList.size() == 0)
        return false;

    
    QDir dir;
    if (!dir.exists(strPath))
    {
        if (!dir.mkdir(strPath))
            return false;
    }

    //char *szFile = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<root>";
    QFile file(strPath + "/trainData.xml");
    file.open(QIODevice::WriteOnly);
    QTextStream qOut(&file);
    qOut << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<root>";

    //file.write(szFile);
    try
    {
        for (int i = 0; i < m_pData->resultList.size(); ++i)
        {
            std::cout <<  m_pData->resultList[i].train_count << std::endl;
            if (m_pData->resultList[i].train_count > 2)      // 训练次数超过2的才保存结果
            {
#ifdef USE_OPENCV
                //wstring writePath = qToStdWString(strPath) + L"/train_" + to_wstring(i + 1) + L".jpg";
                //cv::imwrite(writePath, m_pData->resultList[i].image);          // 保存训练图片
#else
                m_pData->resultList[i].image.save(strPath + QString("/train_%1.jpg").arg(QString::number(i + 1)));
#endif // USE_OPENCV
                write_region(m_pData->resultList[i].region, (strPath + QString("/region_%1.reg").arg(QString::number(i + 1))).toLocal8Bit().data());       // 保存所有物料区域 region
                write_shape_model(m_pData->resultList[i].modelID, (strPath + QString("/model_%1.shm").arg(QString::number(i + 1))).toLocal8Bit().data());  // 保存所有匹配模版 modelID
                qOut << "\n<contours>\n<points>1, 1</points>\n<size height = \"" + QString::number(m_pData->resultList[i].modelHeight[0].I());  // 
                qOut << "\" width=\"" + QString::number(m_pData->resultList[i].modelWidth[0].I());  // 
                qOut << "\" area=\"" + QString::number(m_pData->resultList[i].modelArea[0].I()) +"\"/>\n</contours>";
            }
        }
    }
    catch (HException &e)
    {
        m_pData->strError = e.message;
        return false;
    }

    qOut << "\n</root>";
    file.flush();
    file.close();

    if (bIsHPdetection)
    {
        const char *szFile = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<root>\n<contours>\n<points>1,1</points>\n<size height=\"1\" width=\"1\" area=\"1\"/>\n</contours>\n</root>";
        QFile file2(strPath + "/detection.xml");
        file2.open(QIODevice::WriteOnly);
        file2.write(szFile);
        file2.close();
    }

    m_pData->bTraining = false;
    return true;
}

bool HPDetectionPartsTrainer_Dynamic::isTraining()
{
    return m_pData->bTraining;
}

int HPDetectionPartsTrainer_Dynamic::trainTimes()
{
    if (m_pData->bTraining)
        return m_pData->resultList.size();

    return 0;
}

QString HPDetectionPartsTrainer_Dynamic::errorString()
{
    return m_pData->strError;
}

#ifdef USE_OPENCV
bool HPDetectionPartsTrainer_Dynamic::train(cv::Mat &image)
{
    Hobject hImage;
    cv::Mat outImage;
    m_pData->bTraining = true;

    if (!matToHobject(image, hImage))   // 将Mat转为halcon图像
    {
        m_pData->strError = "image error";
        return false;
    }

    try
    {
        pDevProcedureCall->SetInputIconicParamObject("ImageModel", hImage);                      // 设置算子的输入变量 ImageModel
        pDevProcedureCall->SetInputCtrlParamTuple("WindowHandle", m_pData->WindowHandle);        // 设置算子的输入变量 WindowHandle
        pDevProcedureCall->SetInputCtrlParamTuple("ModelWidthAll", m_pData->ModelWidthAll);        // 设置算子的输入变量 ModelWidthAll
        pDevProcedureCall->SetInputCtrlParamTuple("ModelHeightAll", m_pData->ModelHeightAll);      // 设置算子的输入变量 ModelHeightAll
        pDevProcedureCall->Execute();                                                            // 执行算法

        dump_window_image(&hImage, m_pData->WindowHandle);               // 把算子输出图像保存下来，转成QImage outImage
        hobjectToMat(hImage, outImage);

        Hobject hModelRegionsOrigin = pDevProcedureCall->GetOutputIconicParamObject("ModelRegionsOrigin");    // 获取算子的输出变量 ModelRegionsOrigin
        HTuple hModelID = pDevProcedureCall->GetOutputCtrlParamTuple("ModelID");                  // 获取算子的输出变量 ModelID
        HTuple hModelWidth = pDevProcedureCall->GetOutputCtrlParamTuple("ModelWidth");                  // 获取算子的输出变量 ModelWidth
        HTuple hModelHeight = pDevProcedureCall->GetOutputCtrlParamTuple("ModelHeight");                  // 获取算子的输出变量 ModelHeight
        HTuple hModelArea = pDevProcedureCall->GetOutputCtrlParamTuple("ModelArea");                  // 获取算子的输出变量 ModelHeight
        HTuple isTrained = pDevProcedureCall->GetOutputCtrlParamTuple("isTrained");                  // 获取算子的输出变量 ModelID
        if (isTrained[0].I() == 1)
        {
            m_pData->resultData.region = hModelRegionsOrigin;
            m_pData->resultData.modelID = hModelID;
            m_pData->resultData.modelWidth = hModelWidth;
            m_pData->resultData.modelHeight = hModelHeight;
            m_pData->resultData.modelArea = hModelArea;
            tuple_concat(m_pData->ModelWidthAll, hModelWidth, &m_pData->ModelWidthAll);
            tuple_concat(m_pData->ModelHeightAll, hModelHeight, &m_pData->ModelHeightAll);
            m_pData->resultData.image = image;
            m_pData->resultData.train_count = 1;
            saveTrainData();
        }
        else if (isTrained[0].I() == 2)
        {
            int matchID = hModelID[0].I();
            m_pData->resultList[matchID].train_count++;
            // std::cout << m_pData->resultList[matchID].train_count << std::endl;
        }


        if (hModelID.Num() == 0 || hModelID[0].ValType() != LongVal)
        {
            m_pData->strError = "result error";
            return false;
        }
    }
    catch (HDevEngineException &e)
    {
        m_pData->strError = e.Message();
        return false;
    }
    catch (HException &e)
    {
        m_pData->strError = e.message;
        return false;
    }

    return true;
}

bool matToHobject(cv::Mat &image, Hobject &Hobj)
{
    //三通道图像
    if (3 == image.channels())
    {
        std::vector<cv::Mat>ichannels;
        cv::Mat imgB;
        cv::Mat imgG;
        cv::Mat imgR;
        split(image, ichannels);
        imgB = ichannels.at(0);
        imgG = ichannels.at(1);
        imgR = ichannels.at(2);
        int height = image.rows;
        int width = image.cols;
        uchar *dataRed = new uchar[width*height];
        uchar *dataGreen = new uchar[width*height];
        uchar *dataBlue = new uchar[width*height];
        for (int i = 0; i < height; i++)
        {
            memcpy(dataRed + width*i, imgR.ptr() + imgR.step*i, width);
            memcpy(dataGreen + width*i, imgG.ptr() + imgG.step*i, width);
            memcpy(dataBlue + width*i, imgB.ptr() + imgB.step*i, width);
        }
        gen_image3(&Hobj, "byte", width, height, (Hlong)(dataRed), (Hlong)(dataGreen), (Hlong)(dataBlue));
        delete[] dataRed;
        delete[] dataGreen;
        delete[] dataBlue;
    }
    if (1 == image.channels())
    {
        int height = image.rows;
        int width = image.cols;
        uchar *dataGray = new uchar[width*height];
        for (int i = 0; i < height; i++)
        {
            memcpy(dataGray + width*i, image.ptr() + image.step*i, width);
        }
        gen_image1(&Hobj, "byte", width, height, (Hlong)(dataGray));
        delete[] dataGray;
    }
    return true;
}

bool hobjectToMat(Hobject &Hobj, cv::Mat &rImage)
{
    HTuple htChannels = HTuple();
    char cType[MAX_STRING];
    Hlong width = 0;
    Hlong height = 0;

    convert_image_type(Hobj, &Hobj, "byte");
    count_channels(Hobj, &htChannels);
    if (htChannels[0].I() == 1)
    {
        unsigned char *ptr = NULL;
        get_image_pointer1(Hobj, (Hlong*)&ptr, cType, &width, &height);
        rImage.create(height, width, CV_8UC(1));
        for (int i = 0; i < height; i++)
        {
            memcpy(rImage.ptr() + rImage.step*i, ptr + width*i, width);
        }
    }
    if (htChannels[0].I() == 3)
    {
        unsigned char *ptrRed, *ptrGreen, *ptrBlue;
        ptrRed = ptrGreen = ptrBlue = NULL;
        get_image_pointer3(Hobj, (Hlong*)&ptrRed, (Hlong*)&ptrGreen, (Hlong*)&ptrBlue, cType, &width, &height);
        rImage.create(height, width, CV_8UC(3));
        cv::Mat imgR(height, width, CV_8UC(1));
        cv::Mat imgG(height, width, CV_8UC(1));
        cv::Mat imgB(height, width, CV_8UC(1));
        for (int i = 0; i < height; i++)
        {
            memcpy(imgR.ptr() + imgR.step*i, ptrRed + width*i, width);
            memcpy(imgG.ptr() + imgG.step*i, ptrGreen + width*i, width);
            memcpy(imgB.ptr() + imgB.step*i, ptrBlue + width*i, width);
        }
        std::vector<cv::Mat>mgImage;
        mgImage.push_back(imgB);
        mgImage.push_back(imgG);
        mgImage.push_back(imgR);
        merge(mgImage, rImage);
    }
    return true;
}
#else
bool HPDetectionPartsTrainer_Dynamic::train(QImage &image, const QString &strPath, QImage &outImage)
{
    Hobject hImage;

    if (!qimageToHobject(image, &hImage))
    {
        m_pData->strError = "image error";
        return false;
    }

    try
    {
        pDevProcedureCall->SetInputIconicParamObject("ImageModel", hImage);                      // 设置算子的输入变量 ImageModel
        pDevProcedureCall->SetInputCtrlParamTuple("WindowHandle", m_pData->WindowHandle);        // 设置算子的输入变量 WindowHandle
        pDevProcedureCall->SetInputCtrlParamTuple("ModelWidthAll", m_pData->ModelWidthAll);        // 设置算子的输入变量 ModelWidthAll
        pDevProcedureCall->SetInputCtrlParamTuple("ModelHeightAll", m_pData->ModelHeightAll);      // 设置算子的输入变量 ModelHeightAll
        pDevProcedureCall->Execute();                                                            // 执行算法

        dump_window_image(&hImage, m_pData->WindowHandle);               // 把算子输出图像保存下来，转成QImage outImage
        qimageFromHobject(outImage, &hImage);

        Hobject hModelRegionsOrigin = pDevProcedureCall->GetOutputIconicParamObject("ModelRegionsOrigin");    // 获取算子的输出变量 ModelRegionsOrigin
        HTuple hModelID = pDevProcedureCall->GetOutputCtrlParamTuple("ModelID");                  // 获取算子的输出变量 ModelID
        HTuple hModelWidth = pDevProcedureCall->GetOutputCtrlParamTuple("ModelWidth");                  // 获取算子的输出变量 ModelWidth
        HTuple hModelHeight = pDevProcedureCall->GetOutputCtrlParamTuple("ModelHeight");                  // 获取算子的输出变量 ModelHeight
        HTuple hModelArea = pDevProcedureCall->GetOutputCtrlParamTuple("ModelArea");                  // 获取算子的输出变量 ModelHeight
        HTuple isTrained = pDevProcedureCall->GetOutputCtrlParamTuple("isTrained");                  // 获取算子的输出变量 ModelID
        if (isTrained[0].I() == 1)
        {
            m_pData->resultData.region = hModelRegionsOrigin;
            m_pData->resultData.modelID = hModelID;
            m_pData->resultData.modelWidth = hModelWidth;
            m_pData->resultData.modelHeight = hModelHeight;
            m_pData->resultData.modelArea = hModelArea; 
            tuple_concat(m_pData->ModelWidthAll, hModelWidth, &m_pData->ModelWidthAll);
            tuple_concat(m_pData->ModelHeightAll, hModelHeight, &m_pData->ModelHeightAll);
            m_pData->resultData.image = image;
            m_pData->resultData.train_count = 1;
            saveTrainData();
        }
        else if (isTrained[0].I() == 2)
        {
            int matchID = hModelID[0].I();
            m_pData->resultList[matchID].train_count++;
            // std::cout << m_pData->resultList[matchID].train_count << std::endl;
        }


        if (hModelID.Num() == 0 || hModelID[0].ValType() != LongVal)
        {
            m_pData->strError = "result error";
            return false;
        }
    }
    catch (HDevEngineException &e)
    {
        m_pData->strError = e.Message();
        return false;
    }
    catch (HException &e)
    {
        m_pData->strError = e.message;
        return false;
    }
    return true;
}

#endif // USE_OPENCV

void HPDetectionPartsTrainer_Dynamic::saveTrainData()
{
    m_pData->resultList.append(m_pData->resultData);
}

//extern "C" TRANSPARENT_PARTSHARED_EXPORT CTransparentPartsTrainer * createTransparentPartsTrainer()
//{
//    return new CTransparentPartsTrainer;
//}
