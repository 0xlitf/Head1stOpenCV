#include "hp_detection_partscounter.h"


HPDetectionPartsCounter::HPDetectionPartsCounter()
	: m_pData(NULL)
{
	if (m_pData == NULL)
		m_pData = new CounterData;

	m_pData->state = sysParamError;
	m_pData->bRun = false;
	m_pData->bRoi = false;
	m_pData->bProject = false;
	init();
}

HPDetectionPartsCounter::~HPDetectionPartsCounter()
{
	uninit();

	if (m_pData != NULL)
	{
		delete m_pData;
		m_pData = NULL;
	}
}

void HPDetectionPartsCounter::init()
{
	try
	{
#ifndef HALCONCPlus
		pDevEngine = std::make_shared<HDevEngine>();
		pDevEngine->AddProcedurePath(CONFIG_PATH);

		pDevProcedure = std::make_shared<HDevProcedure>("search_shape_model_gray");
		pDevProcedureCall = std::make_shared<HDevProcedureCall>(*pDevProcedure);
		if (pDevProcedureCall == NULL)
			m_pData->strError = "pDevProcedureCall init failed ! ";
#endif
		int width = mySysParam.imgWidth;
		int height = mySysParam.imgHeight;
		open_window(0, 0, width, height, "root", "buffer", "", &m_pData->WindowHandle);
		set_part(m_pData->WindowHandle, 0, 0, height - 1, width - 1);

		set_system("clip_region", "false");
		set_system("border_shape_models", "true");
		set_system("timer_mode", "performance_counter");
		set_color(m_pData->WindowHandle, "green");
		set_draw(m_pData->WindowHandle, "margin");
		set_line_width(m_pData->WindowHandle, 3);
	}
	catch (HDevEngineException& e)
	{
		m_pData->strError = e.Message();
	}
	catch (HException& e)
	{
		m_pData->strError = e.message;
	}
	catch (...)
	{
		m_pData->strError = "halcon init fail";
	}

	if (!m_pData->strError.empty())
		uninit();
}

void HPDetectionPartsCounter::uninit()
{
}

bool HPDetectionPartsCounter::updateParamHP()
{
#ifndef HALCONCPlus
	if (pDevProcedureCall == NULL){
		m_pData->strError = "pDevProcedureCall init failed ! ";
		return false;
	}
#endif
	try
	{
		m_pData->hThreshold =  mySysParam.threshValue;
		m_pData->hWdiffValue = mySysParam.wdiffValue;
		m_pData->hMinScore = mySysParam.minScore;
		m_pData->hMinDefectInnerRadian = mySysParam.minDefectInnerRadian;
		m_pData->hMinDefectArea = mySysParam.minDefectArea;
		m_pData->hDefectThresh = mySysParam.defectThresh;

		close_window(m_pData->WindowHandle);
		int width = mySysParam.imgWidth;
		int height = mySysParam.imgHeight;
		open_window(0, 0, width, height, "root", "buffer", "", &m_pData->WindowHandle);
		set_part(m_pData->WindowHandle, 0, 0, height - 1, width - 1);

		set_system("clip_region", "false");
		set_system("border_shape_models", "true");
		set_system("timer_mode", "performance_counter");
		//set_colored(m_pData->WindowHandle, 3);
		set_color(m_pData->WindowHandle, "green");
		set_draw(m_pData->WindowHandle, "margin");
		set_line_width(m_pData->WindowHandle, 3);
		return true;
	}
	catch (HDevEngineException& e)
	{
		m_pData->strError = e.Message();
	}
	catch (HException& e)
	{
		m_pData->strError = e.message;
	}
	catch (...)
	{
		m_pData->strError = "sysParam update fail";
	}
	return false;
}


bool HPDetectionPartsCounter::updateProject(std::string strProjectPath, int trainID)
{
#ifndef HALCONCPlus
	if (pDevProcedureCall == NULL){
		m_pData->strError = "pDevProcedureCall init failed ! ";
		return false;
	}
#endif

	if (!isFolderExist(strProjectPath.c_str()))
	{
		m_pData->strError = "Update failed! Can not find project path! ";
	}

	try
	{
		vector<string> fileLists, regionFiles, modelFiles, tupleFiles;
		getFiles(strProjectPath, fileLists);

		//设置文件过滤格式,将过滤后的文件名称存入到列表中
		regionFiles = fileFilter(fileLists, ".reg");
		modelFiles = fileFilter(fileLists, ".shm");
		tupleFiles = fileFilter(fileLists, ".tup");
		

#ifndef HALCONCPlus
		if ((regionFiles.size() == 0) || (modelFiles.size() == 0))
		{
			m_pData->strError = "Failed ! Can not find 'ModelRegionsOrigin' or 'ModelID' !";
			return false;
		}
#endif

		Hobject _tmphRegion, hImage;
		m_strRegionPathList.clear();
		m_strModelPathList.clear();
		m_strProjectPath = strProjectPath;

		if (trainID <= 1)   //首次重置，从1开始
		{
			/*  重置模型（注意：多分类模型不能重置）*/
			gen_empty_obj(&m_pData->hModelRegionsOrigin);
			m_pData->hModelID = HTuple();
			m_pData->hModelGrays = HTuple();
		}

		HTuple _tmpModelID, hNum, hArea, hRow, hCol, hModGray;
		for (auto& regionPath : regionFiles)
		{
			m_pData->hRegionPath = regionPath.data();
			read_region(&_tmphRegion, m_pData->hRegionPath);
			m_strRegionPathList.push_back(regionPath);
			concat_obj(m_pData->hModelRegionsOrigin, _tmphRegion, &m_pData->hModelRegionsOrigin);
			std::cout << regionPath << std::endl;
		}
		for (auto& modelPath : modelFiles)
		{
			m_pData->hModelPath = modelPath.data();
			m_strModelPathList.push_back(modelPath);
			read_shape_model(m_pData->hModelPath, &_tmpModelID);       // todo:改为引入多个模型
			set_shape_model_param(_tmpModelID, "timeout", -1);
			tuple_concat(m_pData->hModelID, _tmpModelID, &m_pData->hModelID);
			std::cout << modelPath << std::endl;
		}
		for (auto& tuplePath : tupleFiles)
		{
			read_tuple(tuplePath.data(), &hModGray);
			tuple_concat(m_pData->hModelGrays, hModGray, &m_pData->hModelGrays);
			std::cout << tuplePath << std::endl;
		}
		m_pData->bProject = true;
		m_pData->state = stateNormal;
		return true;
	}
	catch (HDevEngineException& e)
	{
		m_pData->strError = e.Message();
	}
	catch (HException& e)
	{
		m_pData->strError = e.message;
	}
	catch (...)
	{
		m_pData->strError = "project update fail";
	}

	m_pData->bProject = false;
	m_pData->state = roiError;
	return false;

}

bool HPDetectionPartsCounter::updateROI(int x, int y, int width, int height)
{
#ifndef HALCONCPlus
	if (pDevProcedureCall == NULL){
		m_pData->strError = "pDevProcedureCall init failed ! ";
		return false;
	}
#endif
	try
	{
		gen_rectangle1(&m_pData->RegionROI, y, x, y + height, x + width);
		m_pData->bRoi = true;
		return true;
	}
	catch (HDevEngineException& e)
	{
		m_pData->strError = e.Message();
	}
	catch (HException& e)
	{
		m_pData->strError = e.message;
	}
	catch (...)
	{
		m_pData->strError = "roi fail";
	}
	m_pData->bRoi = false;
	m_pData->state = projectError;
	return false;
}


void HPDetectionPartsCounter::stop()
{
	m_pData->bRun = false;
}

bool HPDetectionPartsCounter::isWorking()
{
	return m_pData->bRun;
}

std::string HPDetectionPartsCounter::errorString()
{
	return m_pData->strError;
}

int HPDetectionPartsCounter::getStatus()
{
	//     if (!m_pData->bRoi)
	//         return roiError;

	if (!m_pData->bProject)
		return projectError;

	return stateNormal;
}

bool compCols(const ObjData& a, const ObjData& b)
{
	//比较a和b最小外接矩形的x坐标，返回偏小的值
	return a.x < b.x;
}

//bool HPDetectionPartsCounter::action(const uchar *pImageBits,vector<double> &resultList)
bool HPDetectionPartsCounter::action(cv::Mat& img, vector<ObjData>& resultList, cv::Mat& result)
{
#ifndef HALCONCPlus
	if (pDevProcedureCall == NULL)
	{
		m_pData->strError = "pDevProcedureCall init failed ! ";
		return false;
	}
#endif

	Hobject hImage, hObjImage, hObjRegions;
	HTuple hModelPathList, hRegionlPathList, hChannels;
	HTuple hModel, rows, cols, widths, areas, results;

	//     if (!m_pData->bRun)
	//         return false;
#ifndef HALCONCPlus
	if ((m_pData->hModelRegionsOrigin.Id() == H_EMPTY_REGION) || (m_pData->hModelID.Num() == 0))
	{
		m_pData->strError = "Failed ! Can not find 'ModelRegionsOrigin' or 'ModelID' !";
		return false;
	}
#endif
	try
	{
		//         Hlong hWidth;
		//         Hlong hHeight;
		//         uchar *pPixelBlock = NULL;
		//         get_image_pointer1(m_pData->ImageNew, (Hlong*)&pPixelBlock, NULL, &hWidth, &hHeight);
		//         memcpy(pPixelBlock, pImageBits, hWidth * hHeight);

		//logText(CLog::Info, "pDevProcedureCall Execute");

		matToHobject(img, m_pData->ImageNew);

		

#ifdef HALCONCPlus

		//颜色物料分割算法
		//segment_color(m_pData->ImageNew, &hImage, &hObjRegion, m_pData->WindowHandle, m_pData->hThreshold);
		
		// 圆螺母缺胶检测算法
		//circle_detector(m_pData->ImageNew, &hObjImage, &hObjRegions, m_pData->WindowHandle, m_pData->hMinDefectInnerRadian, m_pData->hMinDefectArea, m_pData->hDefectThresh, m_pData->hThreshold, &results);
		//area_center(hObjRegions, &areas, &rows, &cols);
			
			//search_shape_model(m_pData->ImageNew, m_pData->RegionROI, m_pData->hModelRegionsOrigin,
			//	m_pData->WindowHandle, m_pData->hModelID, m_pData->hThreshold,
			//	m_pData->hWdiffValue, m_pData->hMinScore, m_pData->hMinDefectInnerRadian, &rows, &cols, &widths, &areas, &results);

		//精细目标检测算法c++版本
			search_shape_model_gray(m_pData->ImageNew, m_pData->RegionROI, m_pData->hModelRegionsOrigin,
				&hObjRegions, m_pData->WindowHandle, m_pData->hModelID, 
				m_pData->hModelGrays, m_pData->hThreshold,
				m_pData->hWdiffValue, m_pData->hMinScore, m_pData->hMinDefectInnerRadian, 
				&rows, &cols, &widths, &areas, &hModel, &results);
			
#else
		try
		{
			pDevProcedureCall->SetInputIconicParamObject("ImageNew", m_pData->ImageNew);
			pDevProcedureCall->SetInputIconicParamObject("RegionROI", m_pData->RegionROI);
			pDevProcedureCall->SetInputIconicParamObject("ModelRegionsOrigins", m_pData->hModelRegionsOrigin);
			hModelPathList = m_strProjectPath.data();
			hRegionlPathList = m_strProjectPath.data();
			pDevProcedureCall->SetInputCtrlParamTuple("WindowHandle", m_pData->WindowHandle);
			pDevProcedureCall->SetInputCtrlParamTuple("ModelIDs", m_pData->hModelID);
			pDevProcedureCall->SetInputCtrlParamTuple("ModelGrays", m_pData->hModelGrays);
			int hThreshold = m_pData->hThreshold[0].I();
			int hWdiffValue = m_pData->hWdiffValue[0].I();
			double hMinScore = m_pData->hMinScore[0].D();
			int hMinDefectInnerRadian = m_pData->hMinDefectInnerRadian[0].I();
			pDevProcedureCall->SetInputCtrlParamTuple("Threshold", m_pData->hThreshold);
			pDevProcedureCall->SetInputCtrlParamTuple("WdiffValue", m_pData->hWdiffValue);
			pDevProcedureCall->SetInputCtrlParamTuple("MinScore", m_pData->hMinScore);
			pDevProcedureCall->SetInputCtrlParamTuple("MinDefectInnerRadian", m_pData->hMinDefectInnerRadian);
			pDevProcedureCall->Execute();
		}
		catch (HDevEngineException& e)
		{
			m_pData->strError = e.Message();
			std::cout << "halcon error: " << m_pData->strError << std::endl;
			//logText(CLog::Info, m_pData->strError);
			return false;
		}
		catch (HException& e)
		{
			m_pData->strError = e.message;
			//logText(CLog::Info, m_pData->strError);
			return false;
		}
		catch (...)
		{
			m_pData->strError = "Execute fail";
			//logText(CLog::Info, m_pData->strError);
			return false;
		}

		//logText(CLog::Info, "pDevProcedureCall Execute end");
		hObjRegions = pDevProcedureCall->GetOutputIconicParamObject("ObjRegions");
		rows = pDevProcedureCall->GetOutputCtrlParamTuple("NewRow");
		cols = pDevProcedureCall->GetOutputCtrlParamTuple("NewColumn");
		widths = pDevProcedureCall->GetOutputCtrlParamTuple("NewWidth");
		areas = pDevProcedureCall->GetOutputCtrlParamTuple("NewArea");
		hModel = pDevProcedureCall->GetOutputCtrlParamTuple("Model");
		results = pDevProcedureCall->GetOutputCtrlParamTuple("Result");

		
#endif
		if (results.Num())
		{
			int aa = results[0].I();
			cout << "isDefect: " << aa << endl;
		}

		cv::Mat tmp;	
		dump_window_image(&hImage, m_pData->WindowHandle);               // 把算子输出图像保存下来，转成 outImage	
		hobjectToMat(hImage, tmp);
		result = tmp.clone();
		int iMin = rows.Num();
		iMin = iMin < cols.Num() ? iMin : cols.Num();
		iMin = iMin < widths.Num() ? iMin : widths.Num();
		iMin = iMin < areas.Num() ? iMin : areas.Num();
		iMin = iMin < results.Num() ? iMin : results.Num();

		iMin = results.Num();
		ObjData resultData;
		for (int i = 0; i < iMin; ++i)
		{
			resultData.x = cols[i].D();
			resultData.y = rows[i].D();
			resultData.width = widths[i].D();
			resultData.area = areas[i].D();
			resultData.model_id = 0;  //默认0为杂料
			if (i < hModel.Num())     //可能存在有料，但是没有匹配modelID的情况
			{
				resultData.model_id = hModel[i].I();
			}
			resultData.isOK_halcon = results[i].I();

			// 图片和rectangle录入
			//shape_trans(hObjRegions, &hObjRegions, "rectangle1");
			Hobject ObjSelect;
			HTuple ObjNum, x, y, row2, col2;
			//count_obj(hObjRegions, &ObjNum);
			//for (int i = 1; i <= ObjNum; i += 1)
			//{
				select_obj(hObjRegions, &ObjSelect, i+1);
				smallest_rectangle1(ObjSelect, &y, &x, &row2, &col2);
				resultData.objRoi = Rect(x[0].I(), y[0].I(), (col2-x)[0].I(), (row2-y)[0].I());
				reduce_domain(m_pData->ImageNew, ObjSelect, &hObjImage);
				crop_domain(hObjImage, &hObjImage);
				hobjectToMat(hObjImage, tmp);
				resultData.objRoiImage = tmp.clone();
			//}
			resultList.push_back(resultData);
		}

		if (resultList.size() > 1)
			std::sort(resultList.begin(), resultList.end(), compCols);
	}
	catch (HDevEngineException& e)
	{
		m_pData->strError = e.Message();
	}
	catch (HException& e)
	{
		m_pData->strError = e.message;
	}
	catch (...)
	{
		m_pData->strError = "init fail";
	}
	

	if (m_pData->strError.empty())
	{
		m_pData->state = stateNormal;
		m_pData->bRun = true;
		return true;
	}

	return m_pData->strError.empty();
}


//extern "C" TRANSPARENT_PARTSHARED_EXPORT CTransparentPartsCounter * createTransparentPartsCounter()
//{
//	return new CTransparentPartsCounter;
//}
