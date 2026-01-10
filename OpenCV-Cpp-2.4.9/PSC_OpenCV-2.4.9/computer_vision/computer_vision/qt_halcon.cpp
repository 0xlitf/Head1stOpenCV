#include "qt_halcon.h"

/*
    QImage  : 存在内存对齐，每一行字节数都是4的倍数
    Hobject : 没有字节对齐
*/

HALCON_EXPORT bool dataToHobject(const uchar* pPixelBlock, int width, int height, int channels, bool isAlignment, bool isBgr, Halcon::Hobject* phImage)
{
    if (pPixelBlock == NULL || width < 1 || height < 1 || phImage == NULL)
        return false;

    try
    {
        switch (channels)
        {
        case 1:
        {
            gen_image1(phImage, "byte", width, height, (Hlong)pPixelBlock);

            if (isAlignment && width % 4)
            {
                uchar* pDstBlock = NULL;
                get_image_pointer1(*phImage, (Hlong*)&pDstBlock, NULL, NULL, NULL);

                int lineBytes = (width + 3) & (~3);

                pPixelBlock += lineBytes;
                pDstBlock += width;
                for (int i = 1; i < height; ++i)
                {
                    memcpy(pDstBlock, pPixelBlock, width);
                    pPixelBlock += lineBytes;
                    pDstBlock += width;
                }
            }
        }
        return true;
        case 3:
        {
            gen_image3(phImage, "byte", width, height, (Hlong)pPixelBlock, (Hlong)pPixelBlock, (Hlong)pPixelBlock);

            uchar* pRed = NULL, * pGreen = NULL, * pBlue = NULL;
            get_image_pointer3(*phImage, (Hlong*)&pRed, (Hlong*)&pGreen, (Hlong*)&pBlue, NULL, NULL, NULL);

            int blanks = 0;

            if (isAlignment)
            {
                blanks = width * 3 % 4;
                if (blanks != 0)
                    blanks = 4 - blanks;
            }

            if (isBgr)
            {
                for (int i = 0; i < height; ++i)
                {
                    for (int j = 0; j < width; ++j)
                    {
                        *pBlue++ = *pPixelBlock++;
                        *pGreen++ = *pPixelBlock++;
                        *pRed++ = *pPixelBlock++;
                    }
                    pPixelBlock += blanks;
                }
            }
            else
            {
                for (int i = 0; i < height; ++i)
                {
                    for (int j = 0; j < width; ++j)
                    {
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
    }
    catch (...)
    {
        return false;
    }

    return false;
}

HALCON_EXPORT bool dataToHobject(const uchar* pRedBlock, const uchar* pGreenBlock, const uchar* pBlueBlock, int width, int height, bool isAlignment, Halcon::Hobject* phImage)
{
    if (pRedBlock == NULL || pGreenBlock == NULL || pBlueBlock == NULL || width < 1 || height < 1 || phImage == NULL)
        return false;

    try
    {
        gen_image3(phImage, "byte", width, height, (Hlong)pRedBlock, (Hlong)pGreenBlock, (Hlong)pBlueBlock);

        if (isAlignment && width % 4)
        {
            uchar* pRed = NULL, * pGreen = NULL, * pBlue = NULL;
            get_image_pointer3(*phImage, (Hlong*)&pRed, (Hlong*)&pGreen, (Hlong*)&pBlue, NULL, NULL, NULL);

            int lineBytes = (width + 3) & (~3);

            pRedBlock += lineBytes;
            pGreenBlock += lineBytes;
            pBlueBlock += lineBytes;
            pRed += width;
            pGreen += width;
            pBlue += width;

            for (int i = 1; i < height; ++i)
            {
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
    }
    catch (...)
    {
        return false;
    }

    return true;
}

HALCON_EXPORT bool dataOfHobject(const Halcon::Hobject* phImage, uchar*& pRed, uchar*& pGreen, uchar*& pBlue, int& width, int& height, int& channels, bool& isAlignment)
{
    if (phImage == NULL)
        return false;

    char type[128];
    Hlong hWidth;
    Hlong hHeight;
    Hlong hChannels;

    try
    {
        if (count_channels(*phImage, &hChannels) != H_MSG_TRUE)
            return false;

        switch (hChannels)
        {
        case 1:
            if (get_image_pointer1(*phImage, (Hlong*)&pRed, type, &hWidth, &hHeight) != H_MSG_TRUE)
                return false;

            if (strcmp(type, "byte") != 0)
                return false;

            break;
        case 3:
            if (get_image_pointer3(*phImage, (Hlong*)&pRed, (Hlong*)&pGreen, (Hlong*)&pBlue, type, &hWidth, &hHeight) != H_MSG_TRUE)
                return false;

            if (strcmp(type, "byte") != 0)
                return false;

            break;
        default:
            return false;
        }
    }
    catch (...)
    {
        return false;
    }

    width = hWidth;
    height = hHeight;
    channels = hChannels;
    isAlignment = false;
    return true;
}

HALCON_EXPORT Halcon::Hobject* createHobject()
{
    return new Halcon::Hobject;
}

HALCON_EXPORT void deleteHobject(Halcon::Hobject*& phImage)
{
    if (phImage != NULL)
    {
        delete phImage;
        phImage = NULL;
    }
}


bool matToHobject(cv::Mat& image, Hobject& Hobj)
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
        uchar* dataRed = new uchar[width * height];
        uchar* dataGreen = new uchar[width * height];
        uchar* dataBlue = new uchar[width * height];
        for (int i = 0; i < height; i++)
        {
            memcpy(dataRed + width * i, imgR.ptr() + imgR.step * i, width);
            memcpy(dataGreen + width * i, imgG.ptr() + imgG.step * i, width);
            memcpy(dataBlue + width * i, imgB.ptr() + imgB.step * i, width);
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
        uchar* dataGray = new uchar[width * height];
        for (int i = 0; i < height; i++)
        {
            memcpy(dataGray + width * i, image.ptr() + image.step * i, width);
        }
        gen_image1(&Hobj, "byte", width, height, (Hlong)(dataGray));
        delete[] dataGray;
    }
    return true;
}

bool hobjectToMat(Hobject& Hobj, cv::Mat& rImage)
{
    HTuple htChannels = HTuple();
    char cType[MAX_STRING];
    Hlong width = 0;
    Hlong height = 0;

    convert_image_type(Hobj, &Hobj, "byte");
    count_channels(Hobj, &htChannels);
    if (htChannels[0].I() == 1)
    {
        unsigned char* ptr = NULL;
        get_image_pointer1(Hobj, (Hlong*)&ptr, cType, &width, &height);
        rImage.create(height, width, CV_8UC(1));
        for (int i = 0; i < height; i++)
        {
            memcpy(rImage.ptr() + rImage.step * i, ptr + width * i, width);
        }
    }
    if (htChannels[0].I() == 3)
    {
        unsigned char* ptrRed, * ptrGreen, * ptrBlue;
        ptrRed = ptrGreen = ptrBlue = NULL;
        get_image_pointer3(Hobj, (Hlong*)&ptrRed, (Hlong*)&ptrGreen, (Hlong*)&ptrBlue, cType, &width, &height);
        rImage.create(height, width, CV_8UC(3));
        cv::Mat imgR(height, width, CV_8UC(1));
        cv::Mat imgG(height, width, CV_8UC(1));
        cv::Mat imgB(height, width, CV_8UC(1));
        for (int i = 0; i < height; i++)
        {
            memcpy(imgR.ptr() + imgR.step * i, ptrRed + width * i, width);
            memcpy(imgG.ptr() + imgG.step * i, ptrGreen + width * i, width);
            memcpy(imgB.ptr() + imgB.step * i, ptrBlue + width * i, width);
        }
        std::vector<cv::Mat>mgImage;
        mgImage.push_back(imgB);
        mgImage.push_back(imgG);
        mgImage.push_back(imgR);
        merge(mgImage, rImage);
    }
    return true;
}