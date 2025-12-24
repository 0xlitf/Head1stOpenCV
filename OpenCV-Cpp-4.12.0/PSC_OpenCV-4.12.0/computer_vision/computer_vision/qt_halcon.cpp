#include "qt_halcon.h"

/*
    QImage  : 存在内存对齐，每一行字节数都是4的倍数
    Hobject : 没有字节对齐
*/

HALCON_EXPORT bool dataToHobject(const uchar *pPixelBlock, int width,
                                 int height, int channels, bool isAlignment,
                                 bool isBgr, HObject *phImage) {
    if (pPixelBlock == NULL || width < 1 || height < 1 || phImage == NULL)
        return false;

    try {
        switch (channels) {
        case 1: {
            GenImage1(phImage, "byte", width, height, (Hlong)pPixelBlock);

            if (isAlignment && width % 4) {
                uchar *pDstBlock = NULL;
                GetImagePointer1(*phImage, (HTuple *)&pDstBlock, NULL, NULL, NULL);

                int lineBytes = (width + 3) & (~3);

                pPixelBlock += lineBytes;
                pDstBlock += width;
                for (int i = 1; i < height; ++i) {
                    memcpy(pDstBlock, pPixelBlock, width);
                    pPixelBlock += lineBytes;
                    pDstBlock += width;
                }
            }
        }
            return true;
        case 3: {
            GenImage3(phImage, "byte", width, height, (Hlong)pPixelBlock,
                      (Hlong)pPixelBlock, (Hlong)pPixelBlock);

            uchar *pRed = NULL, *pGreen = NULL, *pBlue = NULL;
            GetImagePointer3(*phImage, (HTuple *)&pRed, (HTuple *)&pGreen,
                             (HTuple *)&pBlue, NULL, NULL, NULL);

            int blanks = 0;

            if (isAlignment) {
                blanks = width * 3 % 4;
                if (blanks != 0)
                    blanks = 4 - blanks;
            }

            if (isBgr) {
                for (int i = 0; i < height; ++i) {
                    for (int j = 0; j < width; ++j) {
                        *pBlue++ = *pPixelBlock++;
                        *pGreen++ = *pPixelBlock++;
                        *pRed++ = *pPixelBlock++;
                    }
                    pPixelBlock += blanks;
                }
            } else {
                for (int i = 0; i < height; ++i) {
                    for (int j = 0; j < width; ++j) {
                        *pRed++ = *pPixelBlock++;
                        *pGreen++ = *pPixelBlock++;
                        *pBlue++ = *pPixelBlock++;
                    }
                    pPixelBlock += blanks;
                }
            }
        }
            return true;
        default:
            break;
        }
    } catch (...) {
        return false;
    }

    return false;
}

HALCON_EXPORT bool dataToHobject(const uchar *pRedBlock,
                                 const uchar *pGreenBlock,
                                 const uchar *pBlueBlock, int width, int height,
                                 bool isAlignment, HObject *phImage) {
    if (pRedBlock == NULL || pGreenBlock == NULL || pBlueBlock == NULL ||
        width < 1 || height < 1 || phImage == NULL)
        return false;

    try {
        GenImage3(phImage, "byte", width, height, (Hlong)pRedBlock,
                  (Hlong)pGreenBlock, (Hlong)pBlueBlock);

        if (isAlignment && width % 4) {
            uchar *pRed = NULL, *pGreen = NULL, *pBlue = NULL;
            GetImagePointer3(*phImage, (HTuple *)&pRed, (HTuple *)&pGreen,
                             (HTuple *)&pBlue, NULL, NULL, NULL);

            int lineBytes = (width + 3) & (~3);

            pRedBlock += lineBytes;
            pGreenBlock += lineBytes;
            pBlueBlock += lineBytes;
            pRed += width;
            pGreen += width;
            pBlue += width;

            for (int i = 1; i < height; ++i) {
                memcpy(pRed, pRedBlock, width);
                memcpy(pGreen, pGreenBlock, width);
                memcpy(pBlue, pBlueBlock, width);

                pRedBlock += lineBytes;
                pGreenBlock += lineBytes;
                pBlueBlock += lineBytes;
                pRed += width;
                pGreen += width;
                pBlue += width;
            }
        }
    } catch (...) {
        return false;
    }

    return true;
}

HALCON_EXPORT bool dataOfHobject(const HObject *phImage, uchar *&pRed,
                                 uchar *&pGreen, uchar *&pBlue, int &width,
                                 int &height, int &channels,
                                 bool &isAlignment) {
    if (phImage == NULL)
        return false;

    char type[128];
    HTuple hWidth;
    HTuple hHeight;
    HTuple hChannels;

    try {
        CountChannels(*phImage, &hChannels);

        if (0 != (int(hChannels != 3))) {
            GetImagePointer1(*phImage, (HTuple *)&pRed, (HTuple *)type,
                             (HTuple *)&hWidth, (HTuple *)&hHeight);
        } else {
            GetImagePointer3(*phImage, (HTuple *)&pRed, (HTuple *)&pGreen,
                             (HTuple *)&pBlue, (HTuple *)type, (HTuple *)&hWidth,
                             (HTuple *)&hHeight);
        }
    } catch (...) {
        return false;
    }

    width = hWidth;
    height = hHeight;
    channels = hChannels;
    isAlignment = false;
    return true;
}

HALCON_EXPORT HObject *createHobject() { return new HObject; }

HALCON_EXPORT void deleteHobject(HObject *&phImage) {
    if (phImage != NULL) {
        delete phImage;
        phImage = NULL;
    }
}

bool matToHobject(cv::Mat &image, HObject &Hobj) {
    // 三通道图像
    if (3 == image.channels()) {
        std::vector<cv::Mat> ichannels;
        cv::Mat imgB;
        cv::Mat imgG;
        cv::Mat imgR;
        split(image, ichannels);
        imgB = ichannels.at(0);
        imgG = ichannels.at(1);
        imgR = ichannels.at(2);
        int height = image.rows;
        int width = image.cols;
        uchar *dataRed = new uchar[width * height];
        uchar *dataGreen = new uchar[width * height];
        uchar *dataBlue = new uchar[width * height];
        for (int i = 0; i < height; i++) {
            memcpy(dataRed + width * i, imgR.ptr() + imgR.step * i, width);
            memcpy(dataGreen + width * i, imgG.ptr() + imgG.step * i, width);
            memcpy(dataBlue + width * i, imgB.ptr() + imgB.step * i, width);
        }
        GenImage3(&Hobj, "byte", width, height, (Hlong)(dataRed),
                  (Hlong)(dataGreen), (Hlong)(dataBlue));
        delete[] dataRed;
        delete[] dataGreen;
        delete[] dataBlue;
    }
    if (1 == image.channels()) {
        int height = image.rows;
        int width = image.cols;
        uchar *dataGray = new uchar[width * height];
        for (int i = 0; i < height; i++) {
            memcpy(dataGray + width * i, image.ptr() + image.step * i, width);
        }
        GenImage1(&Hobj, "byte", width, height, (Hlong)(dataGray));
        delete[] dataGray;
    }
    return true;
}

bool hobjectToMat(HObject &Hobj, cv::Mat &rImage) {
    HTuple htChannels = HTuple();
    // char cType[MAX_STRING];
    int W, H;
    HTuple cType;
    HTuple grayVal = HTuple();
    HTuple width, height;

    ConvertImageType(Hobj, &Hobj, "byte");
    CountChannels(Hobj, &htChannels);

    GetImageSize(Hobj, &width, &height);
    W = (Hlong)width;
    H = (Hlong)height;

    if (htChannels[0].I() == 1) {
        ///////方法1
        // HTuple ptr, lineSize;
        // GetImagePointer1(Hobj, &ptr, &cType, &width, &height);
        //// 检查图像类型
        // if (cType != "byte") {
        //	std::cerr << "错误: 图像不是8位类型，当前类型: " << cType.S() <<
        //std::endl; 	return -1;
        // }

        // rImage.create(H, W, CV_8UC1);

        // memcpy(rImage.data, (uchar*)ptr[0].I(), width[0].I() * height[0].I());
        // //挂在这一步报错
        //

        /// 方法2
        rImage.create(H, W, CV_8UC(1));
        for (int row = 0; row < H; row++) {
            for (int col = 0; col < W; col++) {
                GetGrayval(Hobj, row, col, &grayVal);
                rImage.at<uchar>(row, col) = (uchar)grayVal.I();
            }
        }

        /// 方法3
        // unsigned char* ptr = NULL;

        // GetImagePointer1(Hobj, (HTuple*)&ptr, &cType, (HTuple*)&width,
        // (HTuple*)&height); GetImagePointer1(Hobj, (HTuple*)&ptr, &cType, &width,
        // &height); rImage.create(height, width, CV_8UC(1));

        //      for (int i = 0; i < height; i++)
        //      {
        //          memcpy(rImage.ptr() + rImage.step * i, (uchar*)ptr[0].I() +
        //          width * i, width.I());
        //      }
    }
    if (htChannels[0].I() == 3) {
        ///// 方法0  ok,但超级耗时
        GetImageSize(Hobj, &width, &height);
        rImage.create(height, width, CV_8UC(3));
        for (int row = 0; row < height.I(); row++) {
            for (int col = 0; col < width.I(); col++) {
                GetGrayval(Hobj, row, col, &grayVal);
                rImage.at<uchar>(row, col * 3) = (uchar)grayVal[2].I();
                rImage.at<uchar>(row, col * 3 + 1) = (uchar)grayVal[1].I();
                rImage.at<uchar>(row, col * 3 + 2) = (uchar)grayVal[0].I();
            }
        }

        // 方法1

        // HTuple ptrR, ptrG, ptrB;
        // GetImagePointer3(Hobj, &ptrR, &ptrG, &ptrB, &cType, &width, &height);
        // W = (Hlong)width;
        // H = (Hlong)height;
        // rImage.create(H, W, CV_8UC3);
        // vector<Mat> vecM(3);
        // vecM[2].create(H, W, CV_8UC1);
        // vecM[1].create(H, W, CV_8UC1);
        // vecM[0].create(H, W, CV_8UC1);
        // uchar* pr = (uchar*)ptrR[0].I();
        // uchar* pg = (uchar*)ptrG[0].I();
        // uchar* pb = (uchar*)ptrB[0].I();
        // memcpy(vecM[2].data, pr, W*H);    // 此方法不行，卡在这一步报错
        // memcpy(vecM[1].data, pg, W*H);
        // memcpy(vecM[0].data, pb, W*H);
        // merge(vecM, rImage);

        /// 方法2
        // HTuple ptrR, ptrG, ptrB;
        // GetImagePointer3(Hobj, &ptrR, &ptrG, &ptrB, &cType, &width, &height);
        // int W = (int)width;
        // int H = (int)height;
        // Mat rImage(H, W, CV_8UC3);

        //// 分别为红绿蓝通道创建Mat对象
        // Mat planes[3];
        // planes[0].create(H, W, CV_8UC1);
        // planes[1].create(H, W, CV_8UC1);
        // planes[2].create(H, W, CV_8UC1);

        //// 确保ptrR, ptrG, ptrB是有效的指针
        // if (ptrR.Length() > 0 && ptrG.Length() > 0 && ptrB.Length() > 0) {
        //	int a = ptrR[0].I();
        //	int b = ptrG[0].I();
        //	int c = ptrB[0].I();
        //	uchar* pr = (uchar*)ptrR[0].I();
        //	uchar* pg = (uchar*)ptrG[0].I();
        //	uchar* pb = (uchar*)ptrB[0].I();

        //	// 复制数据到对应的Mat对象
        //	memcpy(planes[2].data, pr, W * H * sizeof(uchar));
        //	memcpy(planes[1].data, pg, W * H * sizeof(uchar));
        //	memcpy(planes[0].data, pb, W * H * sizeof(uchar));

        //	// 将三个通道合并到rImage中
        //	merge(planes, 3, rImage);
        ////}
        // HTuple ptrR, ptrG, ptrB;

        //// 计算 Halcon 图像每行的实际字节数（步长）
        // Hlong halconStride = width[0].I();
        //// OpenCV Mat 的步长（假设是连续的）
        // Hlong opencvStride = W * sizeof(uchar);

        //// 创建 OpenCV Mat
        // cv::Mat rImage(H, W, CV_8UC1);
        ////uchar* pHalconData = (uchar*)ptrR[0].I(); // 获取 Halcon 图像数据指针
        //// 正确提取指针值
        // Hlong ptrValue = ptrR[0].I();  // 获取指针的整数值
        // uchar* pHalconData = reinterpret_cast<uchar*>(ptrValue);  //
        // 将整数值转换为指针 uchar* pOpencvData = rImage.data;

        //// 逐行拷贝，考虑两者的步长可能不同
        // for (Hlong y = 0; y < H; y++) {
        //	memcpy(pOpencvData + y * opencvStride,
        //		pHalconData + y * halconStride,
        //		W); // 每行拷贝 'width' 个字节（实际像素数据）
        // }

        ///// 方法3
        //      unsigned char* ptrRed, * ptrGreen, * ptrBlue;
        //      ptrRed = ptrGreen = ptrBlue = NULL;

        //
        ////GetImagePointer1(Hobj, &ptr, &cType, &width, &height);
        // GetImagePointer3(Hobj, &ptrR, &ptrG, &ptrB, &cType, &width, &height);
        // uchar* pdataR = (uchar*)ptrR[0].I();
        ////GetImagePointer3(Hobj, (HTuple*)&ptrRed, (HTuple*)&ptrGreen,
        ///(HTuple*)&ptrBlue, &cType, &width, &height);   // 卡在这一步报错
        //      rImage.create(height, width, CV_8UC(3));
        //      cv::Mat imgR(height, width, CV_8UC(1));
        //      cv::Mat imgG(height, width, CV_8UC(1));
        //      cv::Mat imgB(height, width, CV_8UC(1));
        //      for (int i = 0; i < height; i++)
        //      {
        //	memcpy(imgR.ptr() + imgR.step * i, pdataR + width * i, width.I());
        //	memcpy(imgG.ptr() + imgG.step * i, ptrGreen + width * i, width.I());
        //	memcpy(imgB.ptr() + imgB.step * i, ptrBlue + width * i, width.I());
        //      }
        //      std::vector<cv::Mat>mgImage;
        //      mgImage.push_back(imgB);
        //      mgImage.push_back(imgG);
        //      mgImage.push_back(imgR);
        //      merge(mgImage, rImage);
    }
    return true;
}
