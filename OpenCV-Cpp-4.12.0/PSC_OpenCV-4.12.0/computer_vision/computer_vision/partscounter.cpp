/*****************************************************************************
name: couter.cpp
date:  2016.11.19(start)
author: peng mei hua
modification date: 2018.1.6
function:  new counting logic
******************************************************************************/
#include "partscounter.h"
#include "cfg.h"
using namespace std;
using namespace cv;


PartsCounter::PartsCounter()
	: m_errorStatus(0)
{
	///////////////////////////////////////projectPath
		//读取系统参数
	bool bOk = readSysParamFromXml(CONFIG_PATH);

	/*  Sysparam.xml 参数说明  */
	m_threshValue = 200;      //图像二值化阈值。 一般不用修改。 
	m_xCompensate = -20;      //物料允许原地向后滚动最大距离。 单位：pix像素。 一般不用修改。
	m_openElementSize = 15;    //闭合物料之间间隙。  值设置的太小，物料反光倒影容易作为杂料误计。值设置太大，挨得近的物料会算成一个大杂料。单位：pix像素

	m_matchMethod = 2;        //检测方法。  0 比较轮廓相似度，1 比较尺寸（按误差百分比计算），2 比较尺寸（按像素值计算）。 默认选2，一般不用修改。
	m_similarity = 0.95;      //物料轮廓和训练模型的相似度。取值范围0.0~1.0，值越大，检测精度越高。 只在 m_matchMethod = 0 时起作用
	m_Lcoefficient = 0.8;     //物料长宽尺寸允许偏小百分比。取值范围0.0~1.0，值越大，检测精度越高。  只在 m_matchMethod = 1 时起作用
	m_Hcoefficient = 1.2;     //物料长宽尺寸允许偏大百分比。取值范围1.0~2.0，值越小，检测精度越高。  只在 m_matchMethod = 1 时起作用
	m_AdiffValue = 0.2;       //面积大小误差百分比。  取值范围0.0~1.0，值越小，检测精度越高。 不论哪种 m_matchMethod 方法都起作用。
	m_WdiffValue = 8;         //物料长度尺寸允许误差像素范围。  单位：pix像素。 取值范围>0，值越小，检测精度越高。 只在 m_matchMethod = 2 时起作用
	m_HdiffValue = 8;		  //物料宽度尺寸允许误差像素范围。  单位：pix像素。 取值范围>0，值越小，检测精度越高。 只在 m_matchMethod = 2 时起作用
	m_roiLineLimitC = 0.98;   //ROI 左边界起到指定百分比区域（按整个ROI宽比例算），该区域内只跟踪不识别。 
	m_allowContourArea0 = 30000;     //轮廓有效面积上限。面积超过该值的轮廓无效。 一般不用修改。
	m_allowContourArea1 = 50;        //轮廓有效面积下限。面积低于该值的轮廓无效。 一般不用修改。
	m_detectLineC = 20 * mySysParam.imgWidth / 320;   //ROI 前边界后移 m_detectLineC 个像素，该区域内只跟踪不识别。
	m_MinScore = 0.55;    //halcon模版匹配算法最小匹配分值。根据物料轮廓相似度要求设定。取值范围0.0~1.0，值越大，匹配度要求更高，检测精度越高。
	m_MinDefectInnerRadian = 4;    //halcon精细检测算法缺陷部分最小内圆半径。  单位：pix像素。 取值范围>0，值越小，检测精度越高。如果值太小，一小点缺胶都会判断为不合格。一般不用修改。

	///////////////////////////////////

	// initialize();
	m_width = mySysParam.imgWidth;
	m_height = mySysParam.imgHeight;; // 默认值
	m_targetNumber = 0;
	m_errorNumber = 0;

	m_contoursNum = 0;
	m_lastcontoursNum = 0;


	m_contours.clear();
	m_impurityContour.clear();
	m_lastObjContours.clear();
	m_lastImpurityContours.clear();
	m_preMinX = 0;
	m_currentMinX = 0;
	m_preMaxX = m_width;
	m_currentMaxX = m_width;
	m_xCompensate = -20;
	m_yRange = 10;
	m_xRange = 20;
	m_sysparamState = false;
	m_roiState = false;
	m_projectState = false;


	m_isNGEndingCountError = false;
	m_useInsideDetection = false;
	ptr_HPDetectionPartsCounter = std::make_shared<HPDetectionPartsCounter>();
}

PartsCounter::~PartsCounter()
{
}

std::string PartsCounter::getVersion() {
	std::string strVersion;
	strVersion = "V2.6.13 20250921_DL+HP";//PSC_V2.4.25_20221217_alpha
	return strVersion;
}

void PartsCounter::initialize()
{
	m_center.clear();
	m_objArea.clear();
	m_objHeight.clear();
	m_objWidth.clear();
	m_preCenter.clear();
	m_Flag.clear();
	m_preFlag.clear();
	m_contours.clear();
	m_impurityContour.clear();
	m_objContour.clear();
	m_lastObjContours.clear();
	m_lastImpurityContours.clear();
	m_contoursNum = 0;
	m_lastcontoursNum = 0;
	m_preMinX = 0;
	m_currentMinX = 0;
	m_preMaxX = m_roi.roiWidth;
	m_currentMaxX = m_roi.roiWidth;
	m_targetNumber = 0;
	m_errorNumber = 0;
	m_NGEndingNum = 0;
}

int PartsCounter::getStatus()
{
	if (false == m_roiState)
	{
		return roiError;
	}
	if (false == m_projectState)
	{
		return projectError;
	}
	if (false == m_sysparamState)
	{
		return sysParamError;
	}
	return stateNormal;
}

bool  PartsCounter::updateParam()
{
	//读取系统参数
	bool bOk = readSysParamFromXml(CONFIG_PATH);
	
	if (bOk) {
		//更新参数
		m_width = mySysParam.imgWidth;
		m_height = mySysParam.imgHeight;
		m_threshValue = mySysParam.threshValue;
		m_openElementSize = mySysParam.openElementSize;
		m_xCompensate = mySysParam.xCompensate;
		m_matchMethod = mySysParam.matchMethod;
		m_similarity = mySysParam.similarity;
		m_Lcoefficient = mySysParam.lcoefficient;
		m_Hcoefficient = mySysParam.hcoefficient;
		m_AdiffValue = mySysParam.adiffValue;
		m_WdiffValue = mySysParam.wdiffValue;
		m_HdiffValue = mySysParam.hdiffValue;
		m_roiLineLimitC = mySysParam.roiLineLimitC;
		m_MinScore = mySysParam.minScore;
		m_MinDefectInnerRadian = mySysParam.minDefectInnerRadian;
		m_sysparamState = true;

		if (!ptr_HPDetectionPartsCounter->updateParamHP())
			return false;
	}
	return true;
}

bool PartsCounter::updateDLMode(std::wstring strPath)
{
	return ptr_HPDetectionPartsCounter->updateDLMode(strPath);
}

bool PartsCounter::setDLMode(int halconDLModel)
{
	return ptr_HPDetectionPartsCounter->setDLMode(halconDLModel);
}

bool PartsCounter::updateProjectHP(std::string strPath, int trainID)
{
	m_projectState = false;
	m_sysparamState = false;
	std::string strProject = strPath;
	if (!ptr_HPDetectionPartsCounter->updateProject(strProject, trainID))
		return false;

	_TrainData trainData;
	if (!readTrainDataFromXml(strProject, &trainData))
		return false;

	trainData.modelID = trainID;  // 训练ID赋值

	m_projectState = true;
	//m_trainArea.swap(trainData.trainArea);
	//m_trainWidth.swap(trainData.trainWidth);
	//m_trainHeight.swap(trainData.trainHeight);
	//m_trainContours.swap(trainData.trainContours);
	m_trainArea.insert(m_trainArea.end(), trainData.trainArea.begin(), trainData.trainArea.end());
	m_trainWidth.insert(m_trainWidth.end(), trainData.trainWidth.begin(), trainData.trainWidth.end());
	m_trainHeight.insert(m_trainHeight.end(), trainData.trainHeight.begin(), trainData.trainHeight.end());
	m_trainContours.insert(m_trainContours.end(), trainData.trainContours.begin(), trainData.trainContours.end());
	vector<int> vec(trainData.trainWidth.size(), trainData.modelID);
	m_trainID.insert(m_trainID.end(), vec.begin(), vec.end());
	////更新系统参数	
	if (!updateParam())
		return false;
	m_sysparamState = true;
	return true;
}

bool PartsCounter::updateProject(std::string strPath, int trainID)
{
	m_projectState = false;
	m_sysparamState = false;

	std::string strProject = strPath;
	_TrainData trainData;
	if (!readTrainDataFromXml(strProject, &trainData))
		return false;

	trainData.modelID = trainID;  // 训练ID赋值

	m_projectState = true;
	//m_trainArea.swap(trainData.trainArea);
	//m_trainWidth.swap(trainData.trainWidth);
	//m_trainHeight.swap(trainData.trainHeight);
	//m_trainContours.swap(trainData.trainContours);
	m_trainArea.insert(m_trainArea.end(), trainData.trainArea.begin(), trainData.trainArea.end());
	m_trainWidth.insert(m_trainWidth.end(), trainData.trainWidth.begin(), trainData.trainWidth.end());
	m_trainHeight.insert(m_trainHeight.end(), trainData.trainHeight.begin(), trainData.trainHeight.end());
	m_trainContours.insert(m_trainContours.end(), trainData.trainContours.begin(), trainData.trainContours.end());
	vector<int> vec(trainData.trainWidth.size(), trainData.modelID);
	m_trainID.insert(m_trainID.end(), vec.begin(), vec.end());
	//更新参数
	if (!updateParam())
		return false;
	m_sysparamState = true;

	return true;
}

void PartsCounter::reset()
{
	m_trainArea.clear();
	m_trainWidth.clear();
	m_trainHeight.clear();
	m_trainContours.clear();
	m_NGEndingNum = 0;
	m_trainID.clear();
}

bool PartsCounter::updateRoi(std::string strPath)
{
	cout << "read roi from:" << strPath << endl;
	m_roiState = false;
	//读取ROI文件
	if (!readRoiDataFromXml(strPath, &m_roi))
	{
		return false;
	}

	if (!ptr_HPDetectionPartsCounter->updateROI(m_roi.roiLTX, m_roi.roiLTY, m_roi.roiWidth, m_roi.roiHeight))
		return false;

	m_roiState = true;
	//默认最大值为ROI宽度
	m_preMaxX = m_roi.roiWidth;
	m_currentMaxX = m_roi.roiWidth;
	return true;
}

void PartsCounter::drawObjData(cv::Mat* image, bool showImg)
{
	// opencv 绘制物料轮廓和roi边框

		m_objData.clear();
		//如果有目标存在，且设置showImg 为true，则显示轮廓
		if (m_num && showImg)
		{
			m_errorStatus = 16;
			Scalar color(0, 255, 0);
			Rect ri;
			ObjData obj_data;

			m_RoiImage = m_image(Range(m_roi.roiLTY, min(m_image.rows, m_roi.roiLTY + m_roi.roiHeight)), Range(m_roi.roiLTX, min(m_image.cols, m_roi.roiLTX + m_roi.roiWidth)));

			//画矩形框
			vector<vector<Point>>::const_iterator itContours = m_objContour.begin();
			while (itContours != m_objContour.end())
			{
				ri = boundingRect(*itContours);
				//获取ROI图像
				obj_data.objRoiImage = m_RoiImage(Range(ri.y, min(m_RoiImage.rows, ri.y + ri.height)), Range(ri.x, min(m_RoiImage.cols, ri.x + ri.width)));
				obj_data.area = contourArea(*itContours);
				ri.x = ri.x + m_roi.roiLTX;
				ri.y = ri.y + m_roi.roiLTY;
				cv::rectangle(*image, ri, Scalar(0, 255, 0), 2, 8, 0);

				obj_data.x = ri.x + ri.width;
				obj_data.y = ri.y + ri.height*0.5;
				obj_data.width = ri.width;
				obj_data.objRoi = ri;
				obj_data.isOK_halcon = 0;  // 合格
				m_objData.push_back(obj_data);    // 收集物料图片
				++itContours;
			}
			//画轮廓
			drawContours(*image, m_objContour, -1, color, 2, 8, noArray(), INT_MAX, Point(m_roi.roiLTX, m_roi.roiLTY));

			color = Scalar(0, 0, 255);
			//画矩形框
			itContours = m_impurityContour.begin();
			while (itContours != m_impurityContour.end())
			{
				ri = boundingRect(*itContours);
				//获取ROI图像
				obj_data.objRoiImage = m_RoiImage(Range(ri.y, min(m_RoiImage.rows, ri.y + ri.height)), Range(ri.x, min(m_RoiImage.cols, ri.x + ri.width)));
			
				obj_data.area = contourArea(*itContours);
				ri.x = ri.x + m_roi.roiLTX;
				ri.y = ri.y + m_roi.roiLTY;

				cv::rectangle(*image, ri, Scalar(0, 0, 255), 2, 8, 0);
				obj_data.x = ri.x + ri.width;
				obj_data.y = ri.y + ri.height*0.5;
				obj_data.width = ri.width;
				obj_data.objRoi = ri;
				obj_data.isOK_halcon = 1;  // 不合格
				m_objData.push_back(obj_data);    // 收集物料图片
				++itContours;
			}
			//画轮廓
			drawContours(*image, m_impurityContour, -1, color, 2, 8, noArray(), INT_MAX, Point(m_roi.roiLTX, m_roi.roiLTY));

			//画ROI框
			cv::Rect rect;
			rect.x = m_roi.roiLTX;
			rect.y = m_roi.roiLTY;
			rect.width = m_roi.roiWidth;
			rect.height = m_roi.roiHeight;
			cv::rectangle(*image, rect, Scalar(200, 55, 255), 2, 8, 0);
		}
}

void PartsCounter::setNGEndingCountError(bool flag)
{
	m_isNGEndingCountError = flag;
}

void PartsCounter::setUseInsideDetection(bool flag)
{
	m_useInsideDetection = flag;
}

void PartsCounter::setHardTriggerMode(bool flag)
{
	m_useHardTriggerMode = flag;
}

bool PartsCounter::autoSetRoi(Mat* image, int& roiLTX, int& roiLTY, int& roiWidth, int& roiHeight)
{

	m_errorStatus = 11;
	//判断图像是否为空
	if (NULL == image || image->empty())
	{
		throw std::exception();
	}

	Mat_<uchar> m_imageTemp, outThreshold;

	//如果是3通道图像则转换为单通道
	if (3 == image->channels())
		cvtColor(*image, m_imageTemp, CV_BGR2GRAY);
	else
		m_imageTemp = *image;

	threshold(m_imageTemp, outThreshold, m_threshValue, 255, THRESH_BINARY);
	//创建结构元素，大小依据m_openElementSize参数而定
	// 	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2, 2));
	// 	//形态学开运算处理。防止物料反光被一分为二。
	// 	morphologyEx(outThreshold, outThreshold, MORPH_CLOSE, element);
	Mat element1 = getStructuringElement(MORPH_ELLIPSE, Size(15, 15));
	morphologyEx(outThreshold, outThreshold, MORPH_OPEN, element1);

	cv::Mat horMat;
	reduce(outThreshold, horMat, 1, CV_REDUCE_MIN);    //投影图像成一行列向量

	vector<int> m_roiLTY;
	for (int y = 0; y < horMat.rows - 1; y++)
	{
		if (horMat.at<uchar>(y) > 0)
		{
			m_roiLTY.push_back(y);   //检索列向量中所有白点位置
		}
	}

	roiLTX = 2;
	roiWidth = outThreshold.cols - 2;

	// 取上下两个白点位置为ROI边界，若查找不到则默认设置ROI为中间两行像素
	if (0 == m_roiLTY.size())
	{
		roiLTY = outThreshold.rows / 2;
		roiHeight = 2;
	}
	else {
		roiLTY = m_roiLTY[0];
		roiHeight = m_roiLTY[m_roiLTY.size() - 1] - m_roiLTY[0];
	}

	// 显示绘画窗口
	cv::Rect rect;
	rect.x = roiLTX;
	rect.y = roiLTY;
	rect.width = roiWidth;
	rect.height = roiHeight;
	rectangle(m_imageTemp, rect, CV_RGB(255, 0, 0), 2, 8, 0);

	return true;
}

void PartsCounter::segmentObj(Mat* image, bool showImg)
{
	m_errorStatus = 11;
	//判断图像是否为空
	if (NULL == image || image->empty())
	{
		throw std::exception();
	}


	Mat_<uchar> outThreshold, outThreshold2;

	//如果是3通道图像则转换为单通道
	if (3 == image->channels())
		cvtColor(*image, m_image, CV_BGR2GRAY);
	else
		m_image = *image;


	m_errorStatus = 12;

	//获取ROI图像
	m_RoiImage = m_image(Range(m_roi.roiLTY, min(m_image.rows, m_roi.roiLTY + m_roi.roiHeight)), Range(m_roi.roiLTX, min(m_image.cols, m_roi.roiLTX + m_roi.roiWidth)));

	m_errorStatus = 13;



	//阈值分割
	threshold(m_RoiImage, outThreshold, m_threshValue, 255, THRESH_BINARY_INV);
	//创建结构元素，大小依据m_openElementSize参数而定

	//Mat element1 = getStructuringElement(MORPH_ELLIPSE, Size(m_openElementSize, m_openElementSize));    //MORPH_ELLIPSE
	Mat element1 = getStructuringElement(MORPH_RECT, Size(m_openElementSize, 2));    //MORPH_ELLIPSE
	//Mat element2 = getStructuringElement(MORPH_RECT, Size(1, m_openElemesntSize*0.5));
	//创建3*3的圆形结构元素
	//Mat element1 = getStructuringElement(MORPH_RECT, Size(1, 8));

	//目的是去除挡边细小轮廓干扰
	outThreshold.row(0) = uchar(0);
	outThreshold.row(1) = uchar(0);
	outThreshold.row(outThreshold.rows - 1) = uchar(0);
	outThreshold.row(outThreshold.rows - 2) = uchar(0);

	//填充中间空洞
	//Mat fill = outThreshold.clone();
	//for (int y = 1; y < fill.rows - 1; y++)
	//{
	//	if (!fill.at<uchar>(y, 90))   //以黑色背景区域开始填充
	//	{
	//		floodFill(fill, Point(90, y), Scalar(255));
	//		break;
	//	}
	//}	
	//Mat invertFill = inverseColor6(fill);   	//图像反转
	//bitwise_or(outThreshold, invertFill, outThreshold);

	//形态学开运算处理。防止物料反光被一分为二。
	morphologyEx(outThreshold, outThreshold, MORPH_CLOSE, element1);   //MORPH_CLOSE、MORPH_OPEN
	//morphologyEx(outThreshold, outThreshold, MORPH_OPEN, element2);  //MORPH_CLOSE、MORPH_OPEN

	m_errorStatus = 14;
	//将图像四周边的2个像素灰度值均设为255，起闭合视野轮廓作用
	outThreshold.row(0) = uchar(0);
	outThreshold.row(1) = uchar(0);
	outThreshold.row(outThreshold.rows - 1) = uchar(0);
	outThreshold.row(outThreshold.rows - 2) = uchar(0);
	outThreshold.col(0) = uchar(0);
	outThreshold.col(1) = uchar(0);
	outThreshold.col(outThreshold.cols - 1) = uchar(0);
	outThreshold.col(outThreshold.cols - 2) = uchar(0);

	////检测图像边缘轮廓
	Canny(outThreshold, outThreshold, 150, 255, 5);

	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	//形态学膨胀处理，闭合断裂轮廓
	dilate(outThreshold, outThreshold, element);

	m_contours.clear();
	//findContours的输出参数：第一个输出轮廓的指针
	vector<Vec4i> hierarchy;

	m_errorStatus = 15;
	//轮廓检测，CV_RETR_EXTERNAL表示只提取最外层轮廓，CV_CHAIN_APPROX_NONE表示将所有点由链码形式转化为点序列形式 

	findContours(outThreshold, m_contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	vector<vector<Point>>::const_iterator itContours = m_contours.begin();
	vector<Vec4i>::const_iterator itHierarchy = hierarchy.begin();


	while (itContours != m_contours.end())
	{
		//获取轮廓面积
		double conArea = contourArea(*itContours);
		//r1 为目标轮廓的水平外接矩形
		Rect ri = boundingRect(*itContours);
		cv::rectangle(outThreshold, ri, Scalar(255), 2, 8, 0);

		//如果目标面积太小或太大，均予以剔除
		if (((conArea > m_roi.roiWidth*m_roi.roiHeight) || (conArea < m_allowContourArea1)))
		{
			itContours = m_contours.erase(itContours);
			itHierarchy = hierarchy.erase(itHierarchy);
		}
		else
		{
			//靠近ROI上下边界，且高度小于7pix的物体予以剔除（经测试最薄料厚度为7pix,故ri.height取小于4）
			if ((ri.height < 4) && ((ri.y < 5) || (ri.y + ri.height > m_roi.roiHeight - 5)))
			{
				itContours = m_contours.erase(itContours);
				itHierarchy = hierarchy.erase(itHierarchy);
			}
			else
			{
				++itContours;
				//++itHierarchy;		
			}

		}
	}

	//获取轮廓个数，及检测到的目标个数
	unsigned int num = unsigned int(m_contours.size());

	//如果有目标存在，则对轮廓进行排序，返回结果compContours
	if (num > 0)
		sort(m_contours.begin(), m_contours.end(), compContours);


	//将vector大小均设置为num
	m_objArea.resize(num);
	m_objHeight.resize(num);
	m_objWidth.resize(num);
	m_center.resize(num);
	m_tempWidth.resize(num);



	for (unsigned int i = 0; i < m_contours.size(); i++)
	{
		//获取轮廓面积的绝对值
		m_objArea[i] = fabs(contourArea(m_contours[i]));
		//r0 为目标轮廓的最小外接矩形
		RotatedRect r0 = minAreaRect(m_contours[i]);
		//r1 为目标轮廓的水平外接矩形
		Rect r1 = boundingRect(m_contours[i]);
		//rectangle(_imageResult, r1, Scalar(0, 255, 0), 2);

		//获取目标轮廓水平方向宽度
		m_tempWidth[i] = r1.width;

		//取最小外接矩形的长边作为m_objWidth，短边作为m_objHeight
		if (r0.size.width > r0.size.height)
		{
			m_objHeight[i] = r0.size.height;
			m_objWidth[i] = r0.size.width;
		}
		else
		{
			m_objHeight[i] = r0.size.width;
			m_objWidth[i] = r0.size.height;
		}
		////获取目标轮廓的位置坐标
		m_center[i] = r0.center;
		m_center[i].x = r1.x + m_tempWidth[i] + m_roi.roiLTX;

		/*///采用物体最右端轮廓坐标作为位置判断依据
		Point _rightPoint(0, m_height);
		vector<Point> _tempRightContorPts;
		for (int j = 0; j < m_contours[i].size(); j++)
		{
			if (m_contours[i][j].x >= _rightPoint.x){
				_rightPoint = m_contours[i][j];        //取最右轮廓点作为 m_center
				if (m_contours[i][j].y < _rightPoint.y)  _rightPoint = m_contours[i][j];        //取最右上轮廓点作为 m_center
			}
		}
		m_center[i] = Point(_rightPoint.x + m_roi.roiLTX, _rightPoint.y);    //将ROI裁剪部分补偿回来，获得在原图上的x坐标
		*/

		//circle(*image, m_center[i], 4, 255, 2);     //画圆圈显示确认
	}

	if (num)
	{
		m_currentMaxX = m_center[num - 1].x;
		m_currentMinX = m_center[0].x;
	}

}

void PartsCounter::estimateObj()
{
	//获取轮廓个数
	int num = unsigned int(m_contours.size());
	int maxTrainWidth = get_max(m_trainWidth);
	int maxTrainHeight = get_max(m_trainHeight);

	//初始化判断物料是否合格的标志
	m_halcon_flag.clear();
	m_halcon_flag.resize(num);
	
	
	if ((m_lastcontoursNum > num)&&(num>0))
		m_modelIDs.erase(m_modelIDs.begin());   // 假设每次只消失一个

	if ((num > m_lastcontoursNum) && (num>m_modelIDs.size()))
	{
		vector<int> vec(num - m_lastcontoursNum, 0);
		m_modelIDs.insert(m_modelIDs.end(), vec.begin(), vec.end());
	}
	m_modelIDs.resize(num);
	
	
	float min_aDiffValue = 1.0;
	//循环比较目标轮廓是否与训练的结果匹配，如果匹配成功，即视为合格料
	for (int i = 0; i < num; ++i)
	{
		if ((m_objWidth[i] > 1.3 * maxTrainWidth) || (m_objHeight[i] > maxTrainHeight + 10) || (m_objWidth[i] > 0.9 * m_roi.roiWidth))
		{
			// 物料过大，设置为连体杂料
			m_halcon_flag[i] = 3;
		}
		else if ((m_useHardTriggerMode==false)&&((((m_center[i].x - m_tempWidth[i]) < m_roi.roiLTX + m_detectLineC) && (m_tempWidth[i] + m_detectLineC < 0.9 * m_roi.roiWidth)) || (m_center[i].x >(m_roi.roiLTX + m_roi.roiWidth - 5))))
		{
			// 出视野的物料前面会超过m_detectLineC，但是物料长度横跨m_detectLineC到右边视野边缘的除外
			// 进视野的物料尾部一定在视野边缘
			m_halcon_flag[i] = 2;  // 设为进出视野左边缘
		}
		else
		{
			m_halcon_flag[i] = 1;  // 预设为杂料
			//m_modelIDs[i] = 0;   // 杂料匹配为0, 不需要初始化，反而会保留跟踪效果
			for (int j = 0; j < m_trainContours.size(); ++j)
			{
				//方法一：轮廓相似度比较法
				if ((COMP_METHOD_MATHCONTOURS == m_matchMethod))
				{
					double matchScore = matchShapes(m_contours[i], m_trainContours[j], CV_CONTOURS_MATCH_I3, 0);
					if ((((1 - matchScore) >= m_similarity) && ((1 - matchScore) < 1) || ((matchScore > 1)))
						&& (abs(m_trainHeight[j] - m_objHeight[i]) < m_HdiffValue) && (abs(m_trainWidth[j] - m_objWidth[i]) < m_WdiffValue))
					{
						float tempArea = m_trainArea[j] + ((m_objHeight[i] - m_trainHeight[j] - 1) / 2 * m_trainWidth[j])
							+ ((m_objWidth[i] - m_trainWidth[j] - 1) / 2 * m_trainHeight[j]);
						//如果目标面积范围在 模板面积+/-公差面积（现为0.15）以内，则视为合格料
						//if ((((tempArea - m_objArea[i]< m_allowContourArea1*2)) && ((tempArea + m_objArea[i]) > m_allowContourArea1 *2)))
						float aDiffValue = fabs(tempArea - m_objArea[i]) / tempArea;
						if (aDiffValue < m_AdiffValue)
						{
							//将合格的轮廓加入m_objContour
							m_halcon_flag[i] = 0;           //m_Flag=0   合格料
							//对料m_modelID进行赋值
							m_modelIDs[i] = m_trainID[j];
							break;
						}
						if (min_aDiffValue > aDiffValue)
							min_aDiffValue = aDiffValue;
					}
				}

				//方法二：面积、宽、高比较法，m_Lcoefficient与m_Hcoefficient表示最小、最大相似百分比要求
				if ((COMP_METHOD_MATHAREA1 == m_matchMethod)
					//			&& ((m_Lcoefficient * m_trainArea[j] < m_objArea[i]) && (m_Hcoefficient * m_trainArea[j] > m_objArea[i]))
					&& ((m_Lcoefficient * m_trainHeight[j] < m_objHeight[i]) && (m_Hcoefficient * m_trainHeight[j] > m_objHeight[i]))
					&& ((m_Lcoefficient * m_trainWidth[j] < m_objWidth[i]) && (m_Hcoefficient * m_trainWidth[j] > m_objWidth[i]) && (m_center[i].x - (m_tempWidth[i] / 2) >= 5)))
				{
					float tempArea = m_trainArea[j] + ((m_objHeight[i] - m_trainHeight[j] - 1) / 2 * m_trainWidth[j])
						+ ((m_objWidth[i] - m_trainWidth[j] - 1) / 2 * m_trainHeight[j]);
					//如果目标面积范围在 模板面积+/-公差面积（现为0.15）以内，则视为合格料
					//if ((((tempArea - m_objArea[i]< m_allowContourArea1*3)) && ((tempArea + m_objArea[i]) > m_allowContourArea1*3)))
					if (fabs(tempArea - m_objArea[i]) < m_AdiffValue * tempArea)
					{
						m_halcon_flag[i] = 0;           //m_Flag=0   合格料
						//对料m_modelID进行赋值
						m_modelIDs[i] = m_trainID[j];
						break;
					}
				}


				//方法三：宽、高比较法，m_WdiffValue、m_HdiffValue表示宽、高的允许像素值差异范围
				if (COMP_METHOD_MATHAREA2 == m_matchMethod)
				{
					if ((abs(m_trainHeight[j] - m_objHeight[i]) < m_HdiffValue) && (abs(m_trainWidth[j] - m_objWidth[i]) < m_WdiffValue))
					{
						//计算模板面积= 训练面积+（（目标高-模板高）*模板宽）+（（目标宽-模板宽）*模板高）
						//设计该计算方式的意义？ Question-01    A：解决不同相机高度可能存在放大倍数误差问题，起到补偿作用
						float tempArea = m_trainArea[j] + ((m_objHeight[i] - m_trainHeight[j] - 1) / 2 * m_trainWidth[j])
							+ ((m_objWidth[i] - m_trainWidth[j] - 1) / 2 * m_trainHeight[j]);
						//如果目标面积范围在 模板面积+/-公差面积（现为0.15）以内，则视为合格料
						//if ((((tempArea - m_objArea[i]< m_allowContourArea1*2)) && ((tempArea + m_objArea[i]) > m_allowContourArea1 *2)))
						float aDiffValue = fabs(tempArea - m_objArea[i]) / tempArea;
						if (aDiffValue < m_AdiffValue)
						{
							//将合格的轮廓加入m_objContour
							m_halcon_flag[i] = 0;           //m_Flag=0   合格料

							//对料m_modelID进行赋值
							m_modelIDs[i] = m_trainID[j];
							break;
						}
						if (min_aDiffValue > aDiffValue)
							min_aDiffValue = aDiffValue;
					}
				}
			}
			if ((m_halcon_flag[i] > 0) && (min_aDiffValue < 0.9))
				std::cout << "aDiffValue should less than ：" << min_aDiffValue << std::endl;
		}
	}

	hPDetectionEstimateObj();
}

//从集合X中的定顶点u出发，用深度有限的策略寻找增广路
//这种增广路只能是当前的匹配数增加1
int PartsCounter::path(int u) {
	for (int v = ny - 1; v >= 0; --v) {      //考虑所有Yi顶点v, 逆序匹配搜索
		if (g[u][v] == 1 && !mk[v]) {     //Y中顶点v与u邻接，且没有访问过
			mk[v] = 1;                        //访问v

			//如果v没有匹配，则直接将v匹配给u，如果v已经匹配了，但是从cy[v],也就是从v之前已经匹配的x出发，找到一条增广路，但是这里记住这里v已经记录访问过了
			//如果第一个条件成立，则不会递归调用
			//if (cy[v] == -1 || path(cy[v])){
			if (cy[v] == -1) {
				cx[u] = v;         //把Y中v匹配给X中u
				cy[v] = u;            //把X中u匹配给Y中v
				return 1;
			}
		}
	}
	return 0;                        //如果不存在从u出发的增广路，则返回0
}


int PartsCounter::HungarianMatch()
{
	int res = 0;                      // 匹配数
	nx = m_allcontoursNum;             // X是当前帧物料个数
	ny = m_lastallcontoursNum;         // Y是前一帧物料个数
	memset(cx, -1, sizeof(cx));        //从0匹配开始增广，将cx和xy各元素都初始化为-1
	memset(cy, -1, sizeof(cy));
	for (int i = nx - 1; i >= 0; --i) {
		if (cx[i] == -1) {                                    //从X集合中每个没有匹配的点出发开始寻找增广路
			memset(mk, 0, sizeof(mk));
			res += path(i);
		}
	}

	for (int u = nx - 1; u >= 0; --u) {
		// 如果都比较完了u还未匹配上，强制
		for (int v = ny - 1; v >= 0; --v) {
			if ((cy[v] == -1) && (cx[u] == -1) && (g[u][v] != -1)) {
				cx[u] = v;         //把Y中v匹配给X中u
				cy[v] = u;            //把X中u匹配给Y中v
			}
		}
	}
	return res;
}


void PartsCounter::hPDetectionEstimateObj()
{

	//获取轮廓个数
	int num = unsigned int(m_contours.size());
	m_num = num;
	//int maxTrainWidth = get_max(m_trainWidth);

	//判断当前帧中位于上一帧最左端料之前的所有料个数
	int a = 0;
	for (int i = 0; i < num; ++i)
	{
		//防止料在原地不动，但前面有杂质进入，而a只计了1次
		if (m_center[i].x < m_preMinX + 5)
		{
			a++;  //当前帧中位于上一帧最左端料之前的所有料个数  a至少大于1
		}
	}


	int rightBoundryNum = 0;
	int lastRightBoundryNum = 0;

	int leftBoundryNum = 0;
	int lastLeftBoundryNum = 0;
	m_allcontoursNum = unsigned int(m_center.size());
	m_lastallcontoursNum = unsigned int(m_preCenter.size());


	vector<int> dist;
	int min_y = 100;
	int min_y_dist = m_xCompensate;

	for (int i = 0; i < m_lastallcontoursNum; ++i)
	{
		if (m_preCenter[i].x - 5 > m_roi.roiLTX + m_roi.roiWidth - 10)
		{
			lastRightBoundryNum++;  // 进入视野右侧m_preFlag = 2的个数
		}

		if ((m_preCenter[i].x - m_lastTempWidth[i] < m_roi.roiLTX + 5))
		{
			lastLeftBoundryNum = i + 1;
		}
	}


	//判断有两个合格料合并为一个杂料，不计数
	is_merged = -1;
	merge_match = 0;
	if (m_lastallcontoursNum - m_allcontoursNum > 0) {
		for (int i = 0; i < m_allcontoursNum; i++)
		{
			//  未出视野前合并，因为出视野时面积计算不准确。 合并后为连体料的除外
			if ((is_merged == -1) && ((m_center[i].x - m_tempWidth[i] > m_roi.roiLTX + 5) || (m_halcon_flag[i] == 3))) {
				for (unsigned int j = 1; j < m_lastallcontoursNum; j++)
				{
					if (((fabs(m_objArea[i] - m_lastObjArea[j] - m_lastObjArea[j - 1]) < 100) || (m_halcon_flag[i] == 3))    //合并前后面积之和相近是判断依据
						&& (m_objArea[i] - m_lastObjArea[j] > 100)                     // 合并后，料的面积必须有所增加
						&& (m_preCenter[j].x - m_center[i].x > -5)
						&& ((m_preCenter[j - 1].x - m_lastTempWidth[j - 1] > m_roi.roiLTX + 5) || (m_halcon_flag[i] == 3))             // 合并前，前一个料不能是正在出视野
						&& (m_preCenter[j].x - m_preCenter[j - 1].x < m_lastTempWidth[j] + 20))    // 合并前两个料距离不能太远
					{
						is_merged = i;
						merge_match = j - 1;
						break;
					}
				}
			}
		}
	}

	//判断有一个合并料分裂为2个
	is_departed = -1;    // 上一帧的连体料
	departed_match = 0;      // 当前帧的连体料拆分后，未匹配上的物料
	if (m_lastallcontoursNum - m_allcontoursNum < 0) {
		for (int i = 0; i < m_lastallcontoursNum; i++)
		{
			if ((is_departed == -1) && (m_preCenter[i].x - m_lastTempWidth[i] > m_roi.roiLTX + 5)) {
				for (unsigned int j = m_allcontoursNum - 1; j > 0; j--)
				{
					if ((fabs(m_lastObjArea[i] - m_objArea[j] - m_objArea[j - 1]) < 100)
						&& (m_preCenter[i].x - m_center[j].x > -5)
						&& (m_center[j - 1].x - m_tempWidth[j - 1] > m_roi.roiLTX + 5)         // 分裂后，前一个料不能是正在出视野
						&& (m_center[j].x - m_center[j - 1].x < m_tempWidth[j] + 20))    // 分开后两个料距离不能太远
					{
						is_departed = i;
						departed_match = j - 1;
						if (m_objArea[j] < 0.3 * m_lastObjArea[i])
							departed_match = j;  // 如果后面料面积小太多，则认为后面是倒影
						break;
					}
				}
			}
		}
	}

	for (int i = m_allcontoursNum - 1; i >= 0; i--)
	{
		if (m_center[i].x > m_roi.roiLTX + m_roi.roiWidth - 5)
		{
			rightBoundryNum++;  // 进入视野右侧m_Flag = 2的个数
		}

		if ((m_center[i].x - m_tempWidth[i] < m_roi.roiLTX + 5))
		{
			leftBoundryNum = i + 1;
		}

		for (int j = m_lastallcontoursNum - 1; j >= 0; j--)
		{
			if ((is_departed >= 0) && (i == departed_match)) // 物料分离
			{
				g[i][j] = -1;
			}
			else if ((is_merged >= 0) && (j == merge_match)) { // 物料合并
				g[i][j] = -1;
			}
			else if ((abs(m_preCenter[j].y - m_center[i].y) < m_yRange + 5) && (abs(m_preCenter[j].x - m_center[i].x) < m_width * 0.3)
				&& (m_preCenter[j].x - m_center[i].x > -10)
				&& (m_preCenter[j].x < m_roi.roiLTX + m_roi.roiWidth - 5)
				&& ((fabs(m_lastObjArea[j] - m_objArea[i]) < max(m_lastObjArea[j], m_objArea[i]) * 0.5) && (m_center[i].x - m_tempWidth[i] > m_roi.roiLTX + 5))   // 面积差别不能太大，防止0,1位置互换
				&& (m_center[i].x < m_roi.roiLTX + m_roi.roiWidth - 5)) {
				g[i][j] = 1;
			}
			else if ((m_center[i].x - m_preCenter[m_lastallcontoursNum - 1].x > 10)    //在上一帧最后一个料之后新进视野的
				|| (abs(m_preCenter[j].x - m_center[i].x) >= m_width*0.3)   // 距离相差过远的
				|| (m_center[i].x - m_preCenter[j].x > 10)   // 料不会往后退
				|| ((fabs(m_lastObjArea[j] - m_objArea[i]) > max(m_lastObjArea[j], m_objArea[i]) * 0.7) && (m_center[i].x - m_tempWidth[i] > m_roi.roiLTX + 5))   // 面积差别不能太大，防止0,1位置互换
				|| (m_center[i].x > m_roi.roiLTX + m_roi.roiWidth - 5))    // 还未进入视野的
			{
				g[i][j] = -1;    // 以上一律不匹配
			}
			else {
				g[i][j] = 0;
			}
			if ((min_y > abs(m_preCenter[j].y - m_center[i].y)) && (m_preCenter[j].x - m_center[i].x > -10)) {   // 不但要y坐标接近，还要是往前移动。
				min_y = abs(m_preCenter[j].y - m_center[i].y);
				min_y_dist = m_preCenter[j].x - m_center[i].x;
			}
		}
	}

	int matchNum = HungarianMatch();
	for (int num = 0; num < m_allcontoursNum; num++)
	{
		if (cx[num] < 0) continue;
		dist.push_back(m_preCenter[cx[num]].x - m_center[num].x);
	}

	if (dist.size() <= 0) {
		m_xRange = min_y_dist;        // 如果没找到，就取最小的y距离下的x作为dist
	}
	else {
		sort(dist.begin(), dist.end());
		m_xRange = dist[dist.size() / 2];
	}


	//初始化判断物料是否合格的标志
	m_Flag.clear();
	m_Flag.resize(num);
	int lastMatchNo = 10000;
	bool NG3_OK = false;
	NG3_OK_flag = -1;
	//循环比较目标轮廓是否与训练的结果匹配，如果匹配成功，即视为合格料
	for (int i = 0; i < num; ++i)
	{
		if (m_useHardTriggerMode)
		{
			m_Flag[i] = m_halcon_flag[i];
			if (m_halcon_flag[i] == 0)
			{
				m_objContour.push_back(m_contours[i]);
			}
			else if ((1 == m_halcon_flag[i]) || (3 == m_halcon_flag[i]))
			{
				m_impurityContour.push_back(m_contours[i]);
			}
			continue;
		}
		//要求目标轮廓的右端点必须进入视野ROI范围内才予以判断
		if (m_center[i].x < m_roi.roiLTX + m_roi.roiWidth - 5)
		{
			//halcon提取的精细物料OK,NG信息，用于判断物料是否标记合格，杂料
			//如果m_halcon_flag为0直接打合格标记
			if ((m_halcon_flag[i] == 0) && ((m_center[i].x - m_tempWidth[i]) >= m_roi.roiLTX))        //要求目标轮廓的左端点未出图像左边界5像素
			{
				//如果上一帧为合并料物料匹配的m_preFlag=3，且没有分离情况，将保持跟踪结果
				NG3_OK = false;
				if (cx[i] >= 0) {
					if (3 == m_preFlag[cx[i]] && is_departed < 0) {    // 上一帧m_preFlag =3，且在出视野过程(或者)。保持计杂料。
						NG3_OK = true;
						std::cout << "NG3 -> OK" << std::endl;
						//		//if (i == 0) NG3_OK_flag = i;
						m_impurityContour.push_back(m_contours[i]);   // 继续保持其为杂料
						m_Flag[i] = 3;
						continue;
					}
				}
				if (!NG3_OK) {
					//将合格的轮廓加入m_objContour
					m_objContour.push_back(m_contours[i]);
					m_Flag[i] = 0;           //m_Flag=0   合格料
					continue;
				}
			}

			if ((m_halcon_flag[i] == 3) && (is_merged == i))        //融合后变成3；
			{
				m_impurityContour.push_back(m_contours[i]);
				m_Flag[i] = 3;
				continue;
			}

			//比较完训练模板后，仍未continue出循环，且又存在模板轮廓在视野ROI范围内则进入以下判断
			m_errorStatus = 17;
			//如果目标轮廓数超过上一帧中轮廓数（主要包括“正在新进入视野右侧过程中的料”和“一分为二多出来的料”），超过部分均默认设置m_Flag =2


			//如果该料未到视野左边界，即正在跑出视野过程中，则进一步做判断
			if ((m_center[i].x - m_tempWidth[i] > m_roi.roiLTX + 5))
			{
				if (m_lastallcontoursNum == 0)
				{
					if ((m_halcon_flag[i] == 2))      // 如果算法检测为边界料，还是认为是杂料. 上一帧没有料，突然出现的算NGEnding
					{
						m_impurityContour.push_back(m_contours[i]);
						m_Flag[i] = 4;
						continue;
					}
					// 如果上帧中没有物料，则直接设置新物料为m_Flag = 2, 避免视野中突然出现东西被计数
					//m_Flag[i] = 2;
					//continue;
				}
			}

			//搜索当前帧中，与上一帧匹配物料，确认m_Flag
			m_Flag[i] = 2;   // default


			if (cx[i] >= 0) //匹配成功的
			{
				if (0 == m_preFlag[cx[i]]) {
					m_objContour.push_back(m_contours[i]);
					m_Flag[i] = 0;
					continue;
				}
				else if ((1 == m_preFlag[cx[i]]) || (3 == m_preFlag[cx[i]]))
				{
					m_impurityContour.push_back(m_contours[i]);
					if ((m_center[i].x - m_tempWidth[i] > m_roi.roiLTX + m_detectLineC))// 如果没到传送带尾端，则以m_halcon_flag为准，否则以m_preFlag为准
						m_Flag[i] = m_halcon_flag[i];
					else
						m_Flag[i] = m_preFlag[cx[i]];
					continue;
				}
			}
			else {
				// 如果没有匹配成功，按前一帧由后到前顺序，第一个未匹配到当前帧的料作为其m_Flag
				for (int j = m_lastallcontoursNum - 1; j >= 0; j--)
				{
					if ((cy[j] < 0) && (j < lastMatchNo) && (m_preCenter[j].x - m_center[i].x - m_xRange > -15) && (m_xRange > 0) && (m_objArea[i] > 100)) {    // 上次匹配过的，这次不再匹配
						lastMatchNo = j;

						if (0 == m_preFlag[j]) {
							m_objContour.push_back(m_contours[i]);
							m_Flag[i] = 0;
						}
						else if ((1 == m_preFlag[j]) || (3 == m_preFlag[j]))
						{
							m_impurityContour.push_back(m_contours[i]);
							if ((m_center[i].x - m_tempWidth[i] > m_roi.roiLTX + m_detectLineC))// && !NG3_OK
								m_Flag[i] = m_halcon_flag[i];
							else
								m_Flag[i] = m_preFlag[j];
						}
						break;
					}else if ((is_departed >= 0) && (i == departed_match)) {   // 如果是分离料， 未匹配上的又未能识别的打杂料
						m_impurityContour.push_back(m_contours[i]);
						m_Flag[i] = 1;
						break;
					}
				}
			}

			if ((m_halcon_flag[i] == 1) || (m_halcon_flag[i] == 3))      // 如果算法检测为杂料，就认为是杂料. 上一帧本来就是3，现在也是3
			{
				m_impurityContour.push_back(m_contours[i]);
				m_Flag[i] = m_halcon_flag[i];
				continue;
			}

			if ((m_halcon_flag[i] == 2))      // 如果算法检测为边界料，还是认为是杂料. 上一帧有被跟踪关联，则不会流转到该位置，未被跟踪上则作杂料处理
			{
				m_impurityContour.push_back(m_contours[i]);
				m_Flag[i] = 4;
				continue;
			}

			continue;
		}
		//未完全进入视野右侧的，均m_Flag标志设为2
		m_Flag[i] = 2;
	}

	m_obj_num = 0;
	m_err_num = 0;

	//计算合格料和杂质料总数目
	m_obj_num = int(m_objContour.size());  // 视野内合格料数量
	m_err_num = int(m_impurityContour.size());  // 视野内杂料数量
	//m_err_num = m_num - m_obj_num;   // 视野内杂料数量，进出视野的物料暂定归属于为不合格料（20250215发现未进视野的料算不合格的话，会导致objData无数据）

	//计算合格料和杂质料总数目
	// for (int i = 0; i < num; ++i)
	// {
		// if (m_halcon_flag[i] == 0)
		// {
			// m_obj_num++;
		// }
		// else if ((m_halcon_flag[i] == 1) || (m_halcon_flag[i] == 3))
		// {
			// m_err_num++;
		// }
	// }

	m_contoursNum = int(m_objContour.size() + m_impurityContour.size());


	if (m_allcontoursNum)
	{
		int temp0 = m_center[0].x;        //最大X坐标，先初始化为第一个料X坐标
		int temp1 = m_center[0].x;        //最小X坐标，先初始化为第一个料X坐标
		unsigned int k = 0;
		for (int i = 0; i < num; i++)
		{
			//从左端起，选择第一个m_Flag不等于2的料赋值其x坐标给temp0和temp1，然后直接break出循环
			if (2 != m_Flag[i])
			{
				temp0 = m_center[i].x;
				temp1 = m_center[i].x;
				break;
			}
			k++;        //统计从左往右一直到第一个正常料为止，m_Flag=2的目标个数
		}

		//从k+1个目标开始起判断，直到 l < num-1
		for (int j = k, l = k; j < num - 1, l < num - 1; j++, l++)
		{
			//若有遇到m_Flag=2的，跳过continue
			if (2 == m_Flag[j + 1])   continue;

			//如果目标轮廓x坐标大于temp0，则赋值给temp0
			if (m_center[j + 1].x > temp0)
			{
				temp0 = m_center[j + 1].x;
			}
			//如果目标轮廓x坐标小于temp1，则赋值给temp1，不过通常经排序后第一个目标x坐标已经是最小值
			if (m_center[l + 1].x < temp1)
			{
				temp1 = m_center[l + 1].x;
			}
		}
		m_currentMaxX = temp0;                     //最大X坐标
		m_currentMinX = temp1;                     //最小X坐标
	}
}

void PartsCounter::countObj()
{

	if (m_lastcontoursNum > 0)
	{
		/////////////////////上帧有料，当前帧有料////////////////////////////
		if (m_contoursNum > 0)
		{
			/////////////////////上帧轮廓数大于当前帧轮廓数，料减少/////////////////
			if (m_lastcontoursNum > m_contoursNum)
			{
				//如果上帧杂质数和合格料数 均未增加
					 // 					m_targetNumber = static_cast<unsigned int>(m_lastObjContours.size()) - static_cast<unsigned int>(m_objContour.size());
					// 					m_errorNumber = static_cast<unsigned int>(m_lastImpurityContours.size()) - static_cast<unsigned int>(m_impurityContour.size());  
				for (int i = 0; i < m_lastallcontoursNum; i++)
				{
					if ((i < merge_match) || is_merged == -1)
					{
						//则上一帧中，在当前帧最左料之前的料，视为已出视野并计数（增加限制条件：且该料必须从左视野边界处消失才计数）
						if ((m_preCenter[i].x - m_lastTempWidth[i] < m_roi.roiLTX + 50) && (m_preCenter[i].x < m_roi.roiLTX + m_width * 0.3))
						{
							int _tmpXCompensate = m_xRange + 30;
							for (int j = 0; j < m_center.size(); j++)
							{
								if (((m_preCenter[i].x - m_center[j].x) > -10) && cy[i] >= 0) {
									_tmpXCompensate = m_xCompensate / 4;              //如果是物料y坐标接近，且x坐标相近，则认为可能是同一物料
									break;
								}
								else if ((abs(m_preCenter[i].x - m_center[j].x - m_xRange) < 15))         //如果y坐标不相近，仅x坐标相近，原则上认为是不同物料。
								{
									if (m_targetNumber == m_lastcontoursNum - m_contoursNum)          //如果计数已达到物料消失个数，则计数不再增加
									{
										_tmpXCompensate = m_xCompensate / 4;
										break;
									}
								}
							}
							if (m_preCenter[i].x - m_currentMinX < _tmpXCompensate)
							{
								if (0 == m_preFlag[i])
								{
									m_targetNumber++;
								}
								else if ((1 == m_preFlag[i]) || (3 == m_preFlag[i]))
								{
									m_errorNumber++;
								}
								else if (4 == m_preFlag[i])
								{
									if (m_isNGEndingCountError)
									{
										m_errorNumber++;
									}
									m_NGEndingNum++;
								}
							}
						}
					}
				}

			}
			/////////////////////上帧轮廓数小于当前帧轮廓数        料数量增加或不变///////////////// 
			if (m_lastcontoursNum <= m_contoursNum)
			{
				//上一帧中位于当前帧最左料之前的料，仍然视为已出视野，并计数
				for (int i = 0; i < m_lastallcontoursNum; i++)
				{
					if ((i < merge_match) || is_merged == -1)
					{
						if ((m_preCenter[i].x - m_lastTempWidth[i] < m_roi.roiLTX + 50) && (m_preCenter[i].x < m_roi.roiLTX + m_width * 0.3))
						{
							int _tmpXCompensate = m_xRange + 20;
							for (int j = 0; j < m_allcontoursNum; j++)
							{
								if (((m_preCenter[i].x - m_center[j].x) > -15) && cy[i] >= 0) {
									_tmpXCompensate = m_xCompensate / 2;              //如果是物料y坐标接近，或x坐标相近，默认是同一物料
									break;
								}
							}
							if (m_preCenter[i].x - m_currentMinX < _tmpXCompensate)
							{
								if (0 == m_preFlag[i])
								{
									m_targetNumber++;
								}
								else if ((1 == m_preFlag[i]) || (3 == m_preFlag[i]))
								{
									m_errorNumber++;
								}
								else if (4 == m_preFlag[i])
								{
									if (m_isNGEndingCountError)
									{
										m_errorNumber++;
									}
									m_NGEndingNum++;
								}
							}
						}
					}
				} 
			}
			//初始化上一帧相关参数，并赋值
			m_lastObjContours.clear();
			m_lastImpurityContours.clear();
			m_lastObjContours.assign(m_objContour.begin(), m_objContour.end());
			m_lastImpurityContours.assign(m_impurityContour.begin(), m_impurityContour.end());
			m_preMinX = m_currentMinX;
			m_preMaxX = m_currentMaxX;
		}
		////////////////////上帧有料，当前帧无料，全部计数//////////////////////
		else
		{
			for (int a = 0; a < m_lastallcontoursNum; a++)
			{
				//只有料处于出ROI视野左侧的过程中，才认为计数有效。其他情况一概不计数，防止料刚进入视野情况不稳定以及手在视野下接触料造成的误计。
				if ((m_preCenter[a].x - m_lastTempWidth[a] < m_roi.roiLTX + m_detectLineC) && (m_preCenter[a].x < m_roi.roiLTX + m_width * 0.3))
				{
					//若当前帧还是存在料（即m_Flag=2的那一类料）,如果该料不是位于视野左边界（即正在出视野过程中）则不计数
					if ((2 == m_preFlag[a]) && (m_preCenter[a].x - m_lastTempWidth[a] > m_roi.roiLTX + 5))
					{
						continue;
					}
					//剩下的正常计数
					if (0 == m_preFlag[a])
						m_targetNumber++;
					else if ((1 == m_preFlag[a]) || (3 == m_preFlag[a]))
						m_errorNumber++;
					else if (4 == m_preFlag[a])
					{
						if (m_isNGEndingCountError)
						{
							m_errorNumber++;
						}
						m_NGEndingNum++;
					}
				}

			}

			m_preMinX = 0;
			m_preMaxX = m_roi.roiWidth;
			m_lastObjContours.clear();
			m_lastImpurityContours.clear();
		}
	}
	///////////////////////上帧无料，不计数/////////////////
	else
	{
		//直接赋值传递参数
		m_lastObjContours.clear();
		m_lastImpurityContours.clear();
		m_lastObjContours.assign(m_objContour.begin(), m_objContour.end());
		m_lastImpurityContours.assign(m_impurityContour.begin(), m_impurityContour.end());
		m_preMinX = m_currentMinX;
		m_preMaxX = m_currentMaxX;
	}

	m_lastcontoursNum = m_contoursNum;
	m_preFlag.clear();
	m_preFlag.assign(m_Flag.begin(), m_Flag.end());
	m_preCenter.clear();
	m_preCenter.assign(m_center.begin(), m_center.end());
	m_lastTempWidth.clear();
	m_lastTempWidth.assign(m_tempWidth.begin(), m_tempWidth.end());
	m_lastObjWidth.clear();
	m_lastObjWidth.assign(m_objWidth.begin(), m_objWidth.end());
	m_lastObjArea.clear();
	m_lastObjArea.assign(m_objArea.begin(), m_objArea.end());
	m_objContour.clear();
	m_impurityContour.clear();
}

void PartsCounter::action(Mat* image, bool showImg)
{
	CV_Assert(NULL != image);
	m_targetNumber = 0;
	m_errorNumber = 0;

	m_errorStatus = 0;

	//	autoSetRoi(image, &m_roi);
	segmentObj(image, showImg);

	m_errorStatus = 100;
	estimateObj();

	// 绘制物料框
	drawObjData(image, showImg);

	m_errorStatus = 200;
	countObj();

	m_errorStatus = 300;

	//outImage->create(Size(m_width, m_height), CV_8UC3);
	//*outImage = image->clone();
}

void PartsCounter::actionTransObject(double data[][3], int num)
{
	CV_Assert(NULL != data);
	m_targetNumber = 0;
	m_errorNumber = 0;

	m_errorStatus = 0;

	//reconstruct data
	m_contoursNum = num;
	m_currentMinX = data[0][0];
	m_currentMaxX = data[0][0];
	int m_impurity = 0;
	m_tempWidth.resize(num);

	m_Flag.clear();
	m_Flag.resize(num);

	m_center.clear();
	m_center.resize(num);

	cv::Point dummyPoint;
	dummyPoint.x = 0;
	dummyPoint.y = 0;
	for (int i = 0; i < num; i++)
	{
		if (data[i][0] < m_currentMinX)
			m_currentMinX = data[i][0];
		if (data[i][0] > m_currentMaxX)
			m_currentMaxX = data[i][0];
		if (data[i][2] != 0) //非正常料
			m_impurity++;
		m_tempWidth[i] = 200;
		if (data[i][2] == 0) //正常料
			m_Flag[i] = 0;
		else
			m_Flag[i] = 1;
		m_center[i] = dummyPoint;
		m_center[i].x = data[i][0];
		m_center[i].y = data[i][1];
	}

	std::vector<cv::Point> dummyPointVector;
	dummyPointVector.push_back(dummyPoint);
	for (int i = 0; i < m_impurity; i++)
	{
		m_impurityContour.push_back(dummyPointVector);
	}
	for (int i = 0; i < num - m_impurity; i++)
	{
		m_objContour.push_back(dummyPointVector);
	}

	//count object
	m_errorStatus = 200;
	countObj();

	m_lastallcontoursNum = unsigned int(m_preCenter.size());
}


void PartsCounter::actionHPDetectionObjectMat(cv::Mat* image, bool showImg)
{
	m_targetNumber = 0;
	m_errorNumber = 0;
	m_num = 0;
	m_obj_num = 0;
	m_errorStatus = 0;

	m_image = *image;

	vector<ObjData> data;

	ptr_HPDetectionPartsCounter->action(m_image, data, m_image);
	if(showImg) 
		*image = m_image.clone();
	//reconstruct data	
	cv::Point dummyPoint;
	dummyPoint.x = 0;
	dummyPoint.y = 0;
	std::vector<cv::Point> dummyPointVector;
	dummyPointVector.push_back(dummyPoint);
	m_contours.clear();

	m_num = data.size();
	for (unsigned int i = 0; i < m_num; i++) {
		m_contours.push_back(dummyPointVector);
	}

	m_center.clear();
	m_center.resize(m_num);
	m_tempWidth.clear();
	m_tempWidth.resize(m_num);
	m_objArea.clear();
	m_objArea.resize(m_num);
	m_modelIDs.clear();
	m_modelIDs.resize(m_num);
	m_halcon_flag.clear();
	m_halcon_flag.resize(m_num);
	m_objData.clear();
	m_objData = data;         // ObjData 直接赋值
	for (unsigned int i = 0; i < m_num; i++) {
		m_center[i] = dummyPoint;
		m_center[i].x = data[i].x;     // 物料坐标
		m_center[i].y = data[i].y;
		m_tempWidth[i] = data[i].width;    // 目标轮廓水平方向宽度
		m_objArea[i] = data[i].area;     // 目标轮廓长轴方向宽度
		m_modelIDs[i] = 0;    // 默认是杂料，匹配id为0
		
		
		if (m_halconDLModel == DL_ANOMALY_DETECTION)
		{
			m_halcon_flag[i] = data[i].isOK_halcon;    // 异常检测，0为合格，1为不合格
			m_modelIDs[i] = data[i].isOK_halcon;
		}
		else{
			if (data[i].isOK_halcon > 0) // 分类和检测，modelID＞0都认为匹配到物料了，判断为OK，未匹配到判断为NG,modelID为0
				m_halcon_flag[i] = 0;   //合格
			else
				m_halcon_flag[i] = 1;   //NG
			
			if (m_halconDLModel == DL_MODE_CLOSED)
				m_modelIDs[i] = m_trainID[data[i].model_id];        // 获取匹配物料ID
			else
				m_modelIDs[i] = data[i].isOK_halcon;
			
		}
		
		if (m_halcon_flag[i] == 0)
			m_obj_num++;         // 视野内合格料数量
	}

	//计算合格料和杂质料总数目
	m_err_num = m_num - m_obj_num;   // 视野内杂料数量，进出视野的物料暂定归属于为不合格料

	if ((m_useHardTriggerMode == false) && (m_halconDLModel == DL_MODE_CLOSED))
	{
		//*  硬触发模式 和 深度学习模式下，不需要跟踪   *//
		m_errorStatus = 100;
		hPDetectionEstimateObj();

		m_errorStatus = 200;
		countObj();
	}
	
	m_errorStatus = 300;
}
