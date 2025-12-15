// XmlC++RW.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "tinyxml.h"//使用TinyXML只需要将6个文件拷贝到项目中就可以直接使用了，这六个文件是：tinyxml.h、tinystr.h、tinystr.cpp、tinyxml.cpp、tinyxmlerror.cpp、tinyxmlparser.cpp
#include <iostream>
#include <vector>
#include <stdio.h>
#include <windows.h>
#include "cfg.h"
using namespace std;

string  UrlUTF8(char* str);
void GB2312ToUTF_8(string& pOut, char* pText, int pLen);
void Gb2312ToUnicode(WCHAR* pOut, char* gbBuffer);
void UnicodeToUTF_8(char* pOut, WCHAR* pText);
string UrlUTF8(char* str)
{
    string tt;
    string dd;
    GB2312ToUTF_8(tt, str, strlen(str));
    int len = tt.length();
    for (int i = 0; i < len; i++)
    {
        if (isalnum((BYTE)tt.at(i))) //判断字符中是否有数组或者英文
        {
            char tempbuff[2] = { 0 };
            sprintf_s(tempbuff, "%c", (BYTE)tt.at(i));
            dd.append(tempbuff);
        }
        else if (isspace((BYTE)tt.at(i)))
        {
            dd.append("+");
        }
        else
        {
            char tempbuff[4];
            sprintf_s(tempbuff, "%%%X%X", ((BYTE)tt.at(i)) >> 4, ((BYTE)tt.at(i)) % 16);
            dd.append(tempbuff);
        }

    }
    return dd;
}
void GB2312ToUTF_8(string& pOut, char* pText, int pLen)
{
    char buf[4];
    memset(buf, 0, 4);

    pOut.clear();

    int i = 0;
    while (i < pLen)
    {
        //如果是英文直接复制就可以
        if (pText[i] >= 0)
        {
            char asciistr[2] = { 0 };
            asciistr[0] = (pText[i++]);
            pOut.append(asciistr);
        }
        else
        {
            WCHAR pbuffer;
            Gb2312ToUnicode(&pbuffer, pText + i);

            UnicodeToUTF_8(buf, &pbuffer);

            pOut.append(buf);

            i += 2;
        }
    }

    return;
}
void Gb2312ToUnicode(WCHAR* pOut, char* gbBuffer)
{
    ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, gbBuffer, 2, pOut, 1);
    return;
}
void UTF_8ToUnicode(WCHAR* pOut, char* pText)
{
    char* uchar = (char*)pOut;

    uchar[1] = ((pText[0] & 0x0F) << 4) + ((pText[1] >> 2) & 0x0F);
    uchar[0] = ((pText[1] & 0x03) << 6) + (pText[2] & 0x3F);

    return;
}
void  UnicodeToUTF_8(char* pOut, WCHAR* pText)
{
    // 注意 WCHAR高低字的顺序,低字节在前，高字节在后
    char* pchar = (char*)pText;

    pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));
    pOut[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[0] & 0xC0) >> 6);
    pOut[2] = (0x80 | (pchar[0] & 0x3F));

    return;
}

bool readRoiDataFromXml(const string& strPath, RoiData* myRoiData)
{//读roi.xml文件
    string strFileName = strPath + "roi.xml";

    TiXmlDocument doc;
    if (!doc.LoadFile(strFileName.c_str()))
    {
        cerr << doc.ErrorDesc() << endl;
        return false;
    }

    TiXmlElement* root = doc.FirstChildElement();
    if (root == NULL)
    {
        cerr << "Failed to load file: No root element." << endl;
        doc.Clear();
        return false;
    }

    TiXmlElement* roiElem = root->FirstChildElement("roi");

    const char* attr_name;
    const char* attr_rgb;
    const char* attr_enable;
    const char* attr_type;
    const char* attr_visible;
    attr_name = roiElem->Attribute("name");
    attr_rgb = roiElem->Attribute("rgb");
    attr_enable = roiElem->Attribute("enable");
    attr_type = roiElem->Attribute("type");
    attr_visible = roiElem->Attribute("visible");
    cout << "roi para:" << endl;
    cout << "    name:" << attr_name << endl;
    cout << "    rgb:" << attr_rgb << endl;
    cout << "    enable:" << attr_enable << endl;
    cout << "    type:" << attr_type << endl;
    cout << "    visible:" << attr_visible << endl;

    string strW;
    GB2312ToUTF_8(strW, (char*)("宽度"), 4);
    const char* pW = strW.data();
    string strH;
    GB2312ToUTF_8(strH, (char*)("高度"), 4);
    const char* pH = strH.data();
    for (TiXmlElement* valueElem = roiElem->FirstChildElement(); valueElem != NULL; valueElem = valueElem->NextSiblingElement())
    {
       const char* attr_Value;
       attr_Value = valueElem->Attribute("name");
       if (strcmp(attr_Value, "x") == 0)
       {
           TiXmlNode* eValue = valueElem->FirstChild();
           myRoiData->roiLTX = int(eValue->ToText()->Value());
           cout << "    x:" << myRoiData->roiLTX << endl;
       }
       if (strcmp(attr_Value, "y") == 0)
       {
           TiXmlNode* eValue = valueElem->FirstChild();
           myRoiData->roiLTY = int(eValue->ToText()->Value());
           cout << "    y:" << myRoiData->roiLTY << endl;
       }
       if (strcmp(attr_Value, pW) == 0)
       {
           TiXmlNode* eValue = valueElem->FirstChild();
           myRoiData->roiWidth = int(eValue->ToText()->Value());
           cout << "    宽度:" << myRoiData->roiWidth << endl;
       }
       if (strcmp(attr_Value, pH) == 0)
       {
           TiXmlNode* eValue = valueElem->FirstChild();
           myRoiData->roiHeight = int(eValue->ToText()->Value());
           cout << "    高度:" << myRoiData->roiHeight << endl;
       }
    }

    doc.Clear();
    return true;
}

bool readSysParamFromXml(const string& strPath, SysParam* mySysParam)
{//读Sysparam.xml文件
    string strFileName = strPath + "/Sysparam.xml";

    TiXmlDocument doc;
    if (!doc.LoadFile(strFileName.c_str()))
    {
        cerr << doc.ErrorDesc() << endl;
        return false;
    }

    TiXmlElement* root = doc.FirstChildElement();
    if (root == NULL)
    {
        cerr << "Failed to load file: No root element." << endl;
        doc.Clear();
        return false;
    }

    TiXmlElement* sysElem = root->FirstChildElement("Sysparam");
    TiXmlElement* itemElem;
    TiXmlNode* itemElemValue;

    cout << "Sysparam:" << endl;
    itemElem = sysElem->FirstChildElement("threshValue");
    itemElemValue = itemElem->FirstChild();
    mySysParam->threshValue = int(itemElemValue->ToText()->Value());
    cout << "    threshValue:" << mySysParam->threshValue << endl;
    itemElem = sysElem->FirstChildElement("xCompensate");
    itemElemValue = itemElem->FirstChild();
    mySysParam->xCompensate = int(itemElemValue->ToText()->Value());
    cout << "    xCompensate:" << mySysParam->xCompensate << endl;
    itemElem = sysElem->FirstChildElement("openElementSize");
    itemElemValue = itemElem->FirstChild();
    mySysParam->openElementSize = int(itemElemValue->ToText()->Value());
    cout << "    openElementSize:" << mySysParam->openElementSize << endl;
    itemElem = sysElem->FirstChildElement("matchMethod");
    itemElemValue = itemElem->FirstChild();
    mySysParam->matchMethod = int(itemElemValue->ToText()->Value());
    cout << "    matchMethod:" << itemElemValue->ToText()->Value() << endl;
    itemElem = sysElem->FirstChildElement("similarity");
    itemElemValue = itemElem->FirstChild();
    mySysParam->similarity = strtod(itemElemValue->ToText()->Value(), NULL);
    cout << "    similarity:" << itemElemValue->ToText()->Value() << endl;
    itemElem = sysElem->FirstChildElement("Lcoefficient");
    itemElemValue = itemElem->FirstChild();
    mySysParam->lcoefficient = strtod(itemElemValue->ToText()->Value(), NULL);
    cout << "    Lcoefficient:" << itemElemValue->ToText()->Value() << endl;
    itemElem = sysElem->FirstChildElement("Hcoefficient");
    itemElemValue = itemElem->FirstChild();
    mySysParam->hcoefficient = strtod(itemElemValue->ToText()->Value(), NULL);
    cout << "    Hcoefficient:" << itemElemValue->ToText()->Value() << endl;
    itemElem = sysElem->FirstChildElement("aDiffValue");
    itemElemValue = itemElem->FirstChild();
    mySysParam->adiffValue = strtod(itemElemValue->ToText()->Value(), NULL);
    cout << "    aDiffValue:" << itemElemValue->ToText()->Value() << endl;
    itemElem = sysElem->FirstChildElement("wDiffValue");
    itemElemValue = itemElem->FirstChild();
    mySysParam->wdiffValue = int(itemElemValue->ToText()->Value());
    cout << "    wDiffValue:" << itemElemValue->ToText()->Value() << endl;
    itemElem = sysElem->FirstChildElement("hDiffValue");
    itemElemValue = itemElem->FirstChild();
    mySysParam->hdiffValue = int(itemElemValue->ToText()->Value());
    cout << "    hDiffValue:" << itemElemValue->ToText()->Value() << endl;
    itemElem = sysElem->FirstChildElement("roiLineLimitC");
    itemElemValue = itemElem->FirstChild();
    mySysParam->roiLineLimitC = strtod(itemElemValue->ToText()->Value(), NULL);
    cout << "    roiLineLimitC:" << itemElemValue->ToText()->Value() << endl;
    itemElem = sysElem->FirstChildElement("allowContourArea0");
    itemElemValue = itemElem->FirstChild();
    mySysParam->allowContourArea0 = int(itemElemValue->ToText()->Value());
    cout << "    allowContourArea0:" << itemElemValue->ToText()->Value() << endl;
    itemElem = sysElem->FirstChildElement("allowContourArea1");
    itemElemValue = itemElem->FirstChild();
    mySysParam->allowContourArea1 = int(itemElemValue->ToText()->Value());
    cout << "    allowContourArea1:" << itemElemValue->ToText()->Value() << endl;

    doc.Clear();
    return true;
}

bool readTrainDataFromXml(const string& strPath, _TrainData* trainData)
{//读trainData.xml文件
    string strFileName = strPath + "/trainData.xml";

    TiXmlDocument doc;
    if (!doc.LoadFile(strFileName.c_str()))
    {
        cerr << doc.ErrorDesc() << endl;
        return false;
    }

    TiXmlElement* root = doc.FirstChildElement();
    if (root == NULL)
    {
        cerr << "Failed to load file: No root element." << endl;
        doc.Clear();
        return false;
    }

    TiXmlElement* itemElem;
    TiXmlNode* itemElemValue;

    cout << "TrainData:" << endl;
    for (TiXmlElement* contoursElem = root->FirstChildElement("contours"); contoursElem != NULL; contoursElem = contoursElem->NextSiblingElement())
    {
        itemElem = contoursElem->FirstChildElement("points");
        itemElemValue = itemElem->FirstChild();
        string pointStrList = itemElemValue->ToText()->Value();
        cout << "    points:" << pointStrList << endl;

        vector<string> numberList = split(pointStrList, ",");
        int ptCount = numberList.size() / 2;
        if (ptCount <= 0) 
            continue;
       
        trainData->trainContours.push_back(std::vector<cv::Point>());
        std::vector<cv::Point>& ptList = trainData->trainContours.at(trainData->trainContours.size() - 1);
        for (int i = 0; i < ptCount; ++i)
        {
            ptList.push_back(cv::Point(int(numberList[i * 2]), int(numberList[i * 2 + 1])));
        }

        itemElem = contoursElem->FirstChildElement("size");
        int attr_area = int(itemElem->Attribute("area"));
        int attr_width = int(itemElem->Attribute("width"));
        int attr_height = int(itemElem->Attribute("height"));
        trainData->trainArea.push_back(attr_area);
        trainData->trainWidth.push_back(attr_width);
        trainData->trainHeight.push_back(attr_height);
        cout << "    area:" << attr_area << endl;
        cout << "    height:" << attr_height << endl;
        cout << "    width:" << attr_width << endl;
    }

    doc.Clear();
    return true;
}

bool saveTrainDataXML()
{//写trainData.xml文件
    TiXmlDocument doc;
    doc.LinkEndChild(new TiXmlDeclaration("1.0", "utf-8", ""));// 添加XML声明

    TiXmlElement* root = new TiXmlElement("root");
    doc.LinkEndChild(root);

    vector<string> strPoints;
    vector<string> strArea;
    vector<string> strHeight;
    vector<string> strWidth;

    strPoints.push_back("138,80,137,81,136,82,135,83,135,84,134,85,134,86,134,87,134,88,134,89,134,90,134,91,135,92,136,92,137,93,137,94,137,95,137,96,137,97,138,98,138,99,139,100,140,100,141,100,142,101,143,101,144,101,145,101,146,101,147,101,148,101,149,101,150,100,150,99,151,98,152,97,153,97,154,96,155,96,156,96,157,95,157,94,158,93,158,92,158,91,158,90,159,89,159,88,159,87,159,86,158,85,157,85,156,84,155,84,154,84,153,83,152,83,151,83,150,83,149,82,148,82,147,82,146,82,145,81,144,81,143,81,142,81,141,81,140,80,139,80");
    strArea.push_back("10"); 
    strHeight.push_back("20");
    strWidth.push_back("30");

    strPoints.push_back("157,76,156,77,155");
    strArea.push_back("11");
    strHeight.push_back("21");
    strWidth.push_back("31");

    strPoints.push_back("125,72,124,73,123");
    strArea.push_back("12");
    strHeight.push_back("22");
    strWidth.push_back("32");

    strPoints.push_back("135,80,134,81,133");
    strArea.push_back("13");
    strHeight.push_back("23");
    strWidth.push_back("33");

    strPoints.push_back("135,80,134,81,133");
    strArea.push_back("14");
    strHeight.push_back("24");
    strWidth.push_back("34");

    int i;
    for (i = 0; i < strPoints.size(); i++)
    {
        TiXmlElement* contoursElem = new TiXmlElement("contours");
        root->LinkEndChild(contoursElem);

        TiXmlElement* pointsElem = new TiXmlElement("points");
        contoursElem->LinkEndChild(pointsElem);
        TiXmlText* text = new TiXmlText("");
        text->SetValue(strPoints[i].data());
        pointsElem->LinkEndChild(text);

        TiXmlElement* sizeElem = new TiXmlElement("size");
        contoursElem->LinkEndChild(sizeElem);
        sizeElem->SetAttribute("area", strArea[i].data());
        sizeElem->SetAttribute("height", strHeight[i].data());
        sizeElem->SetAttribute("width", strWidth[i].data());
    }

    bool success = doc.SaveFile("save/trainData.xml");
    doc.Clear();

    return success;
}
/*
int main()
{
    if (loadROIXML())
    {
        cout << "roi.xml 读取成功---------------------------------------------------\n" << endl;
    }
    else
    {
        cout << "roi.xml 读取失败---------------------------------------------------\n" << endl;
    }

    if (loadSysparamXML())
    {
        cout << "Sysparam.xml 读取成功---------------------------------------------------\n" << endl;
    }
    else
    {
        cout << "Sysparam.xml 读取失败---------------------------------------------------\n" << endl;
    }

    if (loadTrainDataXML())
    {
        cout << "trainData.xml 读取成功---------------------------------------------------\n" << endl;
    }
    else
    {
        cout << "trainData.xml 读取失败---------------------------------------------------\n" << endl;
    }

    if (saveTrainDataXML())
    {
        cout << "trainData.xml 保存成功---------------------------------------------------\n" << endl;
    }
    else
    {
        cout << "trainData.xml 保存失败---------------------------------------------------\n" << endl;
    }
}
*/

cv::Mat inverseColor6(cv::Mat srcImage)
{
    int row = srcImage.rows;
    int col = srcImage.cols;
    cv::Mat tempImage = srcImage.clone();
    // 建立LUT反色table
    uchar LutTable[256];
    for (int i = 0; i < 256; ++i)
        LutTable[i] = 255 - i;
    cv::Mat lookUpTable(1, 256, CV_8U);
    uchar* pData = lookUpTable.data;
    //建立映射表
    for (int i = 0; i < 256; ++i)
        pData[i] = LutTable[i];
    //应用索引表进行查找
    cv::LUT(srcImage, lookUpTable, tempImage);
    return tempImage;
}


std::vector<std::string> split(const std::string& str, const std::string& pattern)
{
    //const char* convert to char*
    char* strc = new char[strlen(str.c_str()) + 1];
    strcpy(strc, str.c_str());
    std::vector<std::string> resultVec;
    char* tmpStr = strtok(strc, pattern.c_str());
    while (tmpStr != NULL)
    {
        resultVec.push_back(std::string(tmpStr));
        tmpStr = strtok(NULL, pattern.c_str());
    }

    delete[] strc;

    return resultVec;
};
// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
