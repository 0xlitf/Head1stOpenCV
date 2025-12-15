#include "hp_detection_partstrainer.h"

HPDetectionPartsTrainer::HPDetectionPartsTrainer()
	: m_pData(NULL)
{
	if (m_pData == NULL)
		m_pData = new MyTrainData;

	init();
}

HPDetectionPartsTrainer::~HPDetectionPartsTrainer()
{
	uninit();

	if (m_pData != NULL)
	{
		delete m_pData;
		m_pData = NULL;
	}
}

void HPDetectionPartsTrainer::init()
{
	m_pData->bTraining = false;
	try
	{
#ifndef HALCONCPlus
		pDevEngine = std::make_shared<HDevEngine>();
		pDevEngine->AddProcedurePath(CONFIG_PATH);

		//pDevProcedure = std::make_shared<HDevProcedure>("generate_shape_model_gray");
		pDevProcedure = std::make_shared<HDevProcedure>("generate_shape_model_dynamic");
		pDevProcedureCall = std::make_shared<HDevProcedureCall>(*pDevProcedure);
#endif

		int width = mySysParam.imgWidth;
		int height = mySysParam.imgHeight;
		OpenWindow(0, 0, width, height, "root", "buffer", "", &m_pData->WindowHandle);
		SetPart(m_pData->WindowHandle, 0, 0, height - 1, width - 1);
		m_isDynamicHD = false;      // 默认手动训练模式
		m_useCountDebug = false;     // 默认关闭视觉计数调试
		m_pData->strError.clear();
	}
	catch (HDevEngineException& e)
	{
		m_pData->strError = e.Message();
	}
	catch (HException& e)
	{
		m_pData->strError = e.ErrorMessage();
		cout << endl << e.ErrorMessage() << endl;
	}
	catch (...)
	{
		m_pData->strError = "init fail";
	}

	if (!m_pData->strError.empty())
		uninit();
}

void HPDetectionPartsTrainer::HD_dynamicTrainMode(bool flag)
{
	m_isDynamicHD = flag;
}

void HPDetectionPartsTrainer::setTrainTarget(int num)
{
	m_trainNumHD = num;
}

void HPDetectionPartsTrainer::uninit()
{
	CloseWindow(m_pData->WindowHandle);
}

std::string HPDetectionPartsTrainer::project()
{
	return m_pData->strProject;
}

void HPDetectionPartsTrainer::setProject(std::string strProject)
{
	m_pData->strProject = strProject;
}


bool HPDetectionPartsTrainer::start(std::string strProject, std::string roiPath)
{
#ifndef HALCONCPlus
	if (pDevProcedureCall == NULL)
		return false;
#endif
	m_trainNumHD = 3;
	m_pData->resultList.clear();
	m_pData->resultData.train_count = 0;
	m_pData->ModelWidthAll = HTuple();
	m_pData->ModelHeightAll = HTuple();
	m_pData->ModelGrayAll = HTuple();
	m_pData->ModelIDs = HTuple();
	GenEmptyObj(&m_pData->hModelRegions);
	ClearAllShapeModels();

	RoiData roiData;
	if (readRoiDataFromXml(roiPath, &roiData))
	{
		m_roiLTX = roiData.roiLTX;
		m_roiLTY = roiData.roiLTY;
		m_roiWidth = roiData.roiWidth;
		m_roiHeight = roiData.roiHeight;
	}
	GenRectangle1(&m_pData->RegionROI, m_roiLTY, m_roiLTX, m_roiLTY + m_roiHeight, m_roiLTX + m_roiWidth);


	updateParam(mySysParam.threshValue, mySysParam.wdiffValue);

	m_pData->strProject = strProject;
	m_pData->bTraining = true;
	return true;
}

bool HPDetectionPartsTrainer::updateParam(int threshValue, int wdiffValue)
{
	try
	{
		//更新参数
		m_pData->hThreshold = threshValue;
		m_pData->hWdiffValue = wdiffValue;
		CloseWindow(m_pData->WindowHandle);
		int width = mySysParam.imgWidth;
		int height = mySysParam.imgHeight;
		OpenWindow(0, 0, width, height, "root", "buffer", "", &m_pData->WindowHandle);
		SetPart(m_pData->WindowHandle, 0, 0, height - 1, width - 1);
		return true;
	}
	catch (HDevEngineException& e)
	{
		m_pData->strError = e.Message();
	}
	catch (HException& e)
	{
		m_pData->strError = e.ErrorMessage();
		cout << endl << e.ErrorMessage() << endl;
	}
	catch (...)
	{
		m_pData->strError = "sysParam update fail";
	}
	return false;
}



void HPDetectionPartsTrainer::stop()
{
	m_pData->bTraining = false;
}

bool HPDetectionPartsTrainer::isTraining()
{
	return m_pData->bTraining;
}

int HPDetectionPartsTrainer::getTrainedObjNum()
{
	// 获取达到训练深度的模板数量
	if (m_pData->bTraining)
	{
		int cnt = 0;
		for (int i = 0; i < m_pData->resultList.size(); i++)
		{
			if ((m_isDynamicHD == 0) || (m_pData->resultList[i].train_count >= m_trainNumHD))
			{
				cnt++;
			}
		}
		return cnt;
	}
	return 0;
}

int HPDetectionPartsTrainer::getDetectObjNum()
{
	if (m_pData->bTraining)
		return m_pData->resultList.size();

	return 0;
}

std::string HPDetectionPartsTrainer::errorString()
{
	return m_pData->strError;
}

double HPDetectionPartsTrainer::getCalibFactor(cv::Mat& image)
{
	double s = 1.0;
	HObject hImageCalib;
	HTuple Channels, scaleFactor;
	if (!matToHobject(image, hImageCalib))   // 将Mat转为halcon图像
	{
		m_pData->strError = "image error";
		return false;
	}
	CountChannels(hImageCalib, &Channels);
	if (Channels[0].I() == 3)
		Rgb1ToGray(hImageCalib, &hImageCalib);

	try
	{
//		get_calib_factor(hImageCalib, &scaleFactor);
		s = scaleFactor[0].D();
	}
	catch (HDevEngineException& e)
	{
		m_pData->strError = e.Message();
	}
	catch (HException& e)
	{
		m_pData->strError = e.ErrorMessage();
		cout << endl << e.ErrorMessage() << endl;
	}

	return s;
}

bool HPDetectionPartsTrainer::train(cv::Mat& image, cv::Mat& trainResult)
{
	HObject hImage, hImageResult, hModelRegionsOrigin;
	HTuple hModelID, hModelWidth, hModelHeight, hModelArea, hModelGrayVal, isTrained;
	HTuple Channels;
	trainResult.create(image.size(), image.type());

	//cv::Mat outImage;
	m_pData->bTraining = true;

	if (!matToHobject(image, hImage))   // 将Mat转为halcon图像
	{
		m_pData->strError = "image error";
		return false;
	}
	CountChannels(hImage, &Channels);
	if (Channels[0].I() == 3)
		Rgb1ToGray(hImage, &hImage);

	try
	{
#ifdef HALCONCPlus
		if (m_halconDLModel == DL_MODE_CLOSED)
		{
		/*generate_shape_model(hImage, m_pData->RegionROI, &hModelRegionsOrigin, m_pData->WindowHandle, m_pData->ModelWidthAll,
			m_pData->ModelHeightAll, m_pData->hThreshold, &hModelID, &hModelWidth, &hModelHeight, &hModelArea, &isTrained);*/
			
		generate_shape_model_dynamic(hImage, m_pData->RegionROI,
		m_pData->hModelRegions, &hModelRegionsOrigin, m_pData->WindowHandle,
		m_pData->ModelIDs, m_pData->ModelWidthAll, m_pData->ModelHeightAll,
		m_pData->ModelGrayAll, m_pData->hThreshold,  m_pData->hWdiffValue,
		m_isDynamicHD, &hModelID,  &hModelWidth, &hModelHeight, &hModelArea, &hModelGrayVal,
		&isTrained);

		}
#else
		pDevProcedureCall->SetInputIconicParamObject("ImageModel", hImage);                      // 输入图像 image 类型
		pDevProcedureCall->SetInputIconicParamObject("RegionROI", m_pData->RegionROI);           // ROI检测区域 region 类型
		pDevProcedureCall->SetInputIconicParamObject("ModelRegions", m_pData->hModelRegions);    // 已训练模型 region 类型
		pDevProcedureCall->SetInputCtrlParamTuple("WindowHandle", m_pData->WindowHandle);        // 窗口句柄 WindowHandle 类型
		pDevProcedureCall->SetInputCtrlParamTuple("ModelIDs", m_pData->ModelIDs);        // 已训练模型ID tuple 类型
		pDevProcedureCall->SetInputCtrlParamTuple("ModelGrays", m_pData->ModelGrayAll);        // 已训练模型平均灰度 tuple 类型
		
		pDevProcedureCall->SetInputCtrlParamTuple("ModelWidthAll", m_pData->ModelWidthAll);        // 已训练模型宽度尺寸 tuple 类型
		pDevProcedureCall->SetInputCtrlParamTuple("ModelHeightAll", m_pData->ModelHeightAll);      // 已训练模型高度尺寸 tuple 类型
		pDevProcedureCall->SetInputCtrlParamTuple("Threshold", m_pData->hThreshold);			 // 图像分割阈值参数 tuple 类型
		pDevProcedureCall->SetInputCtrlParamTuple("WdiffValue", m_pData->hWdiffValue);			 // 图像分割阈值参数 tuple 类型
		pDevProcedureCall->SetInputCtrlParamTuple("IsDynamic", m_isDynamicHD);			 // 是否采用动态训练模式 tuple 类型
		pDevProcedureCall->Execute();                                                            // 执行算法


		hModelRegionsOrigin = pDevProcedureCall->GetOutputIconicParamObject("ModelRegionsOrigin");    // 获取算子的输出变量 ModelRegionsOrigin
		hModelID = pDevProcedureCall->GetOutputCtrlParamTuple("ModelID");                  // 获取算子的输出变量 ModelID	
		hModelWidth = pDevProcedureCall->GetOutputCtrlParamTuple("ModelWidth");                  // 获取算子的输出变量 ModelWidth
		hModelHeight = pDevProcedureCall->GetOutputCtrlParamTuple("ModelHeight");                  // 获取算子的输出变量 ModelHeight
		hModelArea = pDevProcedureCall->GetOutputCtrlParamTuple("ModelArea");                  // 获取算子的输出变量 ModelHeight
		hModelGrayVal = pDevProcedureCall->GetOutputCtrlParamTuple("ModelGrayVal");                  // 获取算子的输出变量 ModelGrayVal
		isTrained = pDevProcedureCall->GetOutputCtrlParamTuple("isTrained");                  // 获取算子的输出变量 ModelID
#endif

		//set_color(m_pData->WindowHandle, "red");
		//set_draw(m_pData->WindowHandle, "margin");
		//set_line_width(m_pData->WindowHandle, 3);
		//disp_image(hImage, m_pData->WindowHandle);
		//disp_region(hModelRegionsOrigin, m_pData->WindowHandle);
		//disp_region(m_pData->RegionROI, m_pData->WindowHandle);
		DumpWindowImage(&hImageResult, m_pData->WindowHandle);               // 把算子输出图像保存下来，转成QImage outImage
		hobjectToMat(hImageResult, trainResult);

		if (m_isOutBoundryHD > 0)
			m_isOutBoundryHD--;

		if (isTrained[0].I() == 1)
		{
			HTuple Area, CenterRow, CenterColumn, CenterRow2, CenterColumn2, AngleCheck, Score, Model, OriginalClipRegion;
			AreaCenter(hModelRegionsOrigin, &Area, &CenterRow2, &CenterColumn2);
			GetSystem("clip_region", &OriginalClipRegion);
			SetSystem("clip_region", "false");
			SetSystem("border_shape_models", "false");
			FindShapeModels(hImage, hModelID, 0, HTuple(360).TupleRad(), 0.01, 1, 0.2, "least_squares", 0, 0.9, &CenterRow, &CenterColumn, &AngleCheck, &Score, &Model);
			MoveRegion(hModelRegionsOrigin, &hModelRegionsOrigin, mySysParam.imgHeight/2 - CenterRow, mySysParam.imgWidth/2 - CenterColumn);
			ConcatObj(m_pData->hModelRegions, hModelRegionsOrigin, &m_pData->hModelRegions);
			m_pData->resultData.region = hModelRegionsOrigin;
			m_pData->resultData.modelID = hModelID;
			m_pData->resultData.modelWidth = hModelWidth;
			m_pData->resultData.modelHeight = hModelHeight;
			m_pData->resultData.modelArea = hModelArea;
			m_pData->resultData.ModelGrayVal = hModelGrayVal;
			TupleConcat(m_pData->ModelWidthAll, hModelWidth, &m_pData->ModelWidthAll);
			TupleConcat(m_pData->ModelHeightAll, hModelHeight, &m_pData->ModelHeightAll);
			TupleConcat(m_pData->ModelIDs, hModelID, &m_pData->ModelIDs);
			TupleConcat(m_pData->ModelGrayAll, hModelGrayVal, &m_pData->ModelGrayAll);
			m_pData->resultData.image = trainResult.clone();
			m_pData->resultData.train_count = 0;   // 重置光纤计数器，掉下才计数，刚创建不计数
			HP_matchID = hModelID;
			saveTrainData();
			return true;
		}
		else if (isTrained[0].I() == 2)
		{

			HP_matchID = hModelID;    // 返回匹配modelID
		}
		else if (isTrained[0].I() == 3)
		{
			if(m_useCountDebug == true)
				trainCountAdd();   // 调试时代替光纤计数
		}

		if (hModelID.Length() == 0)
		{
			m_pData->strError = "result error";
		}
	}
	catch (HDevEngineException& e)
	{
		m_pData->strError = e.Message();
		return false;
	}
	catch (HException& e)
	{
		m_pData->strError = e.ErrorMessage();
		cout << endl << e.ErrorMessage() << endl;
		return false;
	}

	return false;
}




void HPDetectionPartsTrainer::saveTrainData()
{
	m_pData->resultList.push_back(m_pData->resultData);
}

void HPDetectionPartsTrainer::trainCountAdd()
{
	// 如果视觉调试 或者 料不在出视野边界时光纤计数 都有效
	if (m_isOutBoundryHD == 0)
	{
		for (int i = 0; i < m_pData->resultList.size(); ++i)
		{
			if (0 != (int(HTuple(m_pData->resultList[i].modelID) == HP_matchID)))
			{
				m_isOutBoundryHD = 4;
				m_pData->resultList[i].train_count++;
				std::cout << "HP_matchID: " << HP_matchID << "  train_count:" << m_pData->resultList[i].train_count << std::endl;
			}
		}
	}
}

cv::Mat HPDetectionPartsTrainer::getTrainedImage()
{
	cv::Mat outImg;
	for (int i = 0; i < m_pData->resultList.size(); ++i)
	{
		if (0 != (int(HTuple(m_pData->resultList[i].modelID) == HP_matchID)))
		{
			outImg = m_pData->resultList[i].image.clone();
		}
	}
	return outImg;
}

cv::Size HPDetectionPartsTrainer::getTrainedSize()
{
	double width, height;
	for (int i = 0; i < m_pData->resultList.size(); ++i)
	{
		if (0 != (int(HTuple(m_pData->resultList[i].modelID) == HP_matchID)))
		{
			width = m_pData->resultList[i].modelWidth[0].D();
			height = m_pData->resultList[i].modelHeight[0].D();
		}
	}
	return cv::Size(width, height);
}

bool HPDetectionPartsTrainer::finish()
{
#ifndef HALCONCPlus
	if (pDevProcedureCall == NULL)
		return false;
#endif

	if (!m_pData->bTraining)
		return false;

	if (m_pData->resultList.size() == 0)
		return false;

	std::string strPath = m_pData->strProject;

	createDirectory(strPath.c_str());

	_TrainData train_result;

	try
	{
		int indexID = 1;
		HTuple ModelGraySave;
		for (int i = 0; i < m_pData->resultList.size(); ++i)
		{
			std::cout << "trainingID: " << i << "         accumulate times:" << m_pData->resultList[i].train_count << std::endl;
			if ((m_isDynamicHD == 0) || (m_pData->resultList[i].train_count >= m_trainNumHD))      // 训练次数超过3次的就保存结果
			{
				string writePath = strPath + "/train_out_" + to_string(indexID) + ".jpg";
				cv::imwrite(writePath.c_str(), m_pData->resultList[i].image);          // 保存训练图片 todo
				WriteRegion(m_pData->resultList[i].region, (strPath + "/region_" + to_string(indexID) + ".reg").data());       // 保存所有物料区域 region
				WriteShapeModel(m_pData->resultList[i].modelID, (strPath + "/model_" + to_string(indexID) + ".shm").data());  // 保存所有匹配模版 modelID
				cv::Point dummyPoint;
				dummyPoint.x = 1;
				dummyPoint.y = 1;
				std::vector<cv::Point> dummyPointVector;
				dummyPointVector.push_back(dummyPoint);
				train_result.trainContours.push_back(dummyPointVector);
				train_result.trainArea.push_back(int(m_pData->resultList[i].modelArea[0].D()));
				train_result.trainWidth.push_back(int(m_pData->resultList[i].modelWidth[0].D() * 2));
				train_result.trainHeight.push_back(int(m_pData->resultList[i].modelHeight[0].D() * 2));
				TupleConcat(ModelGraySave, m_pData->ModelGrayAll[i], &ModelGraySave);
				m_pData->resultList[i].train_count = 0;
				indexID++;
			}
			ClearShapeModel(m_pData->resultList[i].modelID);    // 逐个清理已缓存的模版
		}
		// 如果没保存过项目，就不保存trainData了
		if (indexID > 1){
			WriteTuple(ModelGraySave, (strPath + "/thresh.tup").data());
			saveTrainDataXML(train_result, strPath);
		}
	}
	catch (HException& e)
	{
		m_pData->strError = e.ErrorMessage();
		cout << endl << e.ErrorMessage() << endl;
		return false;
	}
	m_pData->resultList.clear();
	m_pData->ModelWidthAll = HTuple();
	m_pData->ModelHeightAll = HTuple();
	m_pData->ModelGrayAll = HTuple();
	GenEmptyObj(&m_pData->hModelRegions);
	m_pData->bTraining = false;
	return true;
}

//extern "C" TRANSPARENT_PARTSHARED_EXPORT CTransparentPartsTrainer * createTransparentPartsTrainer()
//{
//    return new CTransparentPartsTrainer;
//}
