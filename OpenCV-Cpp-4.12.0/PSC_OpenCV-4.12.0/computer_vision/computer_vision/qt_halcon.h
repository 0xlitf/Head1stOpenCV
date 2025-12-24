#ifndef QT_HALCON_H
#define QT_HALCON_H

#include "HDevEngineCpp.h"
#include "HalconCpp.h"
#include "cfg.h"

using namespace HalconCpp;
using namespace HDevEngineCpp;

#if defined(TRANSPARENT_PART_LIBRARY)
#define HALCON_EXPORT // Q_DECL_EXPORT
#else
#define HALCON_EXPORT // Q_DECL_IMPORT
#endif

using namespace cv;

bool matToHobject(cv::Mat &image, HObject &hImage);
bool hobjectToMat(HObject &Hobj, cv::Mat &rImage);

/*
    dataToHobject
        提供图片原始数据到Halcon的转换，只支持1和3通道的转换

    pPixelBlock : 待转换的图像的像素数据块
    pRedBlock   : RGB图像拆分后的红色通道数据块
    pGreenBlock : RGB图像拆分的的绿色通道数据块
    pBlueBlock  : RGB图像拆分的的蓝色通道数据块
    width       : 图像数据的宽度
    height      : 图像数据的高度
    channels    : 图像数据的通道数，值={1,3}
    isAlignment : 图像数据是否内存对齐，即每一行像素的字节数都是4的倍数
    isBgr       :
   针对RGB图像使用，表明3个通道像素的存储顺序，true表示0xRRGGBB，false表示0xBBGGRR
*/
HALCON_EXPORT bool dataToHobject(const uchar *pPixelBlock, int width,
                                 int height, int channels, bool isAlignment,
                                 bool isBgr, HObject *phImage);
HALCON_EXPORT bool dataToHobject(const uchar *pRedBlock,
                                 const uchar *pGreenBlock,
                                 const uchar *pBlueBlock, int width, int height,
                                 bool isAlignment, HObject *phImage);

/*
    dataOfHobject
        提取 phImage 中的图像数据指针和其他信息，只支持 byte 类型的1或3通道的
   Hobject 对象

    pRed        : phImage 的红色通道数据块
    pGreen      : phImage 的绿色通道数据块
    pBlue       : phImage 的蓝色通道数据块
    width       : phImage 的图像宽度
    height      : phImage 的图像高度
    channels    : phImage 的图像通道数
    isAlignment : phImage 是否内存对齐，固定输出 false
*/
HALCON_EXPORT bool dataOfHobject(const HObject *phImage, uchar *&pRed,
                                 uchar *&pGreen, uchar *&pBlue, int &width,
                                 int &height, int &channels, bool &isAlignment);

/*
    createHobject deleteHobject
        针对没包含halcon库而有需要使用 Hobject 的情况，提供 new Halcon::Hobject
   和 delete Halcon::Hobject 的功能

    createHobject : return new Halcon::Hobject;
    deleteHobject : if (phImage != NULL) { delete phImage; phImage = NULL; }
*/
HALCON_EXPORT HObject *createHobject();
HALCON_EXPORT void deleteHobject(HObject *&phImage);

#endif // QT_HALCON_H
