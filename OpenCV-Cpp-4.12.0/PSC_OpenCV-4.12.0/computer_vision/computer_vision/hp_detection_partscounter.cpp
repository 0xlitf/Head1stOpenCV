#include "hp_detection_partscounter.h"
#include "qt_halcon.h"
#include <codecvt>
// #include <boost/locale/encoding_utf.hpp>

HPDetectionPartsCounter::HPDetectionPartsCounter() : m_pData(NULL) {
    if (m_pData == NULL)
        m_pData = new CounterData;

    m_pData->state = sysParamError;
    m_pData->bRun = false;
    m_pData->bRoi = false;
    m_pData->bProject = false;
    init();
}

HPDetectionPartsCounter::~HPDetectionPartsCounter() {
    uninit();

    if (m_pData != NULL) {
        delete m_pData;
        m_pData = NULL;
    }
}

void HPDetectionPartsCounter::init() {
    try {
#ifndef HALCONCPlus
        pDevEngine = std::make_shared<HDevEngine>();
        pDevEngine->AddProcedurePath(CONFIG_PATH);

        pDevProcedure = std::make_shared<HDevProcedure>("search_shape_model_gray");
        pDevProcedureCall = std::make_shared<HDevProcedureCall>(*pDevProcedure);
        if (pDevProcedureCall == NULL)
            m_pData->strError = "pDevProcedureCall init failed ! ";
#else

        dev_update_off();
        if (HDevWindowStack::IsOpen())
            CloseWindow(HDevWindowStack::Pop());
        SetSystem("seed_rand", 73);

#endif
        int width = mySysParam.imgWidth;
        int height = mySysParam.imgHeight;

        OpenWindow(0, 0, width, height, "root", "buffer", "",
                   &m_pData->WindowHandle);
        SetPart(m_pData->WindowHandle, 0, 0, height - 1, width - 1);

        SetSystem("clip_region", "false");
        SetSystem("border_shape_models", "true");
        SetSystem("timer_mode", "performance_counter");
        SetColor(m_pData->WindowHandle, "green");
        SetDraw(m_pData->WindowHandle, "margin");
        SetLineWidth(m_pData->WindowHandle, 3);
    } catch (HDevEngineException &e) {
        m_pData->strError = e.Message();
    } catch (HException &e) {
        m_pData->strError = e.ErrorMessage();
        cout << endl << e.ErrorMessage() << endl;
    } catch (...) {
        m_pData->strError = "halcon init fail";
    }

    if (!m_pData->strError.empty())
        uninit();
}

void HPDetectionPartsCounter::uninit() {}

bool HPDetectionPartsCounter::updateParamHP() {
#ifndef HALCONCPlus
    if (pDevProcedureCall == NULL) {
        m_pData->strError = "pDevProcedureCall init failed ! ";
        return false;
    }
#endif
    try {
        m_pData->hThreshold = mySysParam.threshValue;
        m_pData->hWdiffValue = mySysParam.wdiffValue;
        m_pData->hMinScore = mySysParam.minScore;
        m_pData->hMinDefectInnerRadian = mySysParam.minDefectInnerRadian;
        m_pData->hMinDefectArea = mySysParam.minDefectArea;
        m_pData->hDefectThresh = mySysParam.defectThresh;

        CloseWindow(m_pData->WindowHandle);
        int width = mySysParam.imgWidth;
        int height = mySysParam.imgHeight;
        OpenWindow(0, 0, width, height, "root", "buffer", "",
                   &m_pData->WindowHandle);
        SetPart(m_pData->WindowHandle, 0, 0, height - 1, width - 1);

        SetSystem("clip_region", "false");
        SetSystem("border_shape_models", "true");
        SetSystem("timer_mode", "performance_counter");
        SetColor(m_pData->WindowHandle, "green");
        SetDraw(m_pData->WindowHandle, "margin");
        SetLineWidth(m_pData->WindowHandle, 3);
        return true;
    } catch (HDevEngineException &e) {
        m_pData->strError = e.Message();
    } catch (HException &e) {
        m_pData->strError = e.ErrorMessage();
        cout << endl << e.ErrorMessage() << endl;
    } catch (...) {
        m_pData->strError = "sysParam update fail";
    }
    return false;
}

bool HPDetectionPartsCounter::setDLMode(int halconDLModel) {
    if (0 <= halconDLModel) {
        m_halconDLModel = halconDLModel;
        return true;
    } else {
        return false;
    }
}

std::wstring string_to_wstring(const std::string &str) {
    if (str.empty())
        return L"";

    // 获取所需缓冲区大小
    int size_needed =
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0],
                        size_needed);
    return wstr;
}

bool HPDetectionPartsCounter::updateDLMode(std::wstring strPath) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string str = converter.to_bytes(strPath);

    if (!isFolderExist(str.c_str()) || (str.find_last_of(".hdl") < 0)) {
        m_pData->strError = "Update DL model failed! Can not find project path! ";
        std::cout << m_pData->strError << std::endl;
        return false;
    }

    try {
        //***   1.) PREPARE   ***     读取深度学习模型
        // 方法一：改变编码规则
        // std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        // auto wstr = converter.from_bytes(strPath);
        // std::string s = converter.to_bytes(wstr);
        // m_pData->hModelPath = s.data();

        // m_pData->hModelPath.FromString(strPath.c_str());

        // 方法二：将 std::string 转换为 HString
        // HString hstr(strPath.c_str());
        //// 将 HString 转换为 HTuple
        // HTuple::HStringToString(hstr, &m_pData->hModelPath);

        // HString hstr(strPath.c_str());
        // m_pData->hModelPath = HTuple(hstr);
        //
        // Load the Global Context Anomaly Detection model and
        // set key parameters.
        //  方法三：使用宽字符版本
        // boost::locale::conv::utf_to_utf<wchar_t>(str.c_str(), str.c_str() +
        // str.size()) std::wstring widePath =
        // L"./config/model_训练-250616-183342_opt.hdl"; std::wstring widePath =
        // string_to_wstring(strPath);
        HTuple modelPath(strPath.c_str());
        std::cout << "Read DL Model, Path: " << str << std::endl;
        ReadDlModel(modelPath, &hv_DLModelHandle);
        std::cout << "   Successed!" << std::endl;
        //
        // Set preprocessing parameters and preprocess.
        create_dl_preprocess_param_from_model(hv_DLModelHandle, "none",
                                              "full_domain", HTuple(), HTuple(),
                                              HTuple(), &hv_DLPreprocessParam);
        CreateDict(&hv_PreprocessSettings);
        SetDictTuple(hv_PreprocessSettings, "overwrite_files", "true");

        //***************************   ʨѸ׼Ѹ   ****************************
        //
        // Use either a GPU or a CPU for inference later.
        QueryAvailableDlDevices(
            ((HTuple("runtime").Append("runtime")).Append("id")),
            ((HTuple("gpu").Append("cpu")).Append(0)), &hv_DLDeviceHandles);
        if (0 != (int(hv_DLDeviceHandles == HTuple()))) {
            throw HException("No suitable CPU or GPU was found.");
        }
        hv_DLDeviceHandle = ((const HTuple &)hv_DLDeviceHandles)[0];
        //
        // Set the chosen device. In case of memory difficulties,
        // use a more powerful GPU or a more powerful CPU instead.
        SetDlModelParam(hv_DLModelHandle, "device", hv_DLDeviceHandle);
        //
        // Use AI2 Interface for inference if available.
        std::cout << "QueryAvailable tensorrt DlDevices ... passed" << std::endl;

        if (m_halconDLModel == DL_OBJECT_DETECTION_HOR) {
            SetDlModelParam(hv_DLModelHandle, "min_confidence", 0.6);
            SetDlModelParam(hv_DLModelHandle, "max_overlap", 0.2);
            SetDlModelParam(hv_DLModelHandle, "max_overlap_class_agnostic", 0.7);
        }

        if (m_halconDLModel == DL_ANOMALY_DETECTION) {
            GetDlModelParam(hv_DLModelHandle, "meta_data", &hv_MetaData);
            hv_InferenceClassificationThreshold =
                (hv_MetaData.TupleGetDictTuple("anomaly_classification_threshold"))
                                                      .TupleNumber();
            hv_InferenceSegmentationThreshold =
                (hv_MetaData.TupleGetDictTuple("anomaly_segmentation_threshold"))
                                                    .TupleNumber();
            CreateDict(&hv_DLDatasetInfo);
            SetDictTuple(hv_DLDatasetInfo, "class_names",
                         (HTuple( // 合格
                              "ok")
                              .Append( // 不合格
                                  "ng")));
            SetDictTuple(hv_DLDatasetInfo, "class_ids", (HTuple(0).Append(1)));
        } else {
            CreateDict(&hv_DLDatasetInfo);
            // Create a dictionary with dataset parameters used for display.
            GetDlModelParam(hv_DLModelHandle, "class_ids", &hv___Tmp_Ctrl_0);
            SetDictTuple(hv_DLDatasetInfo, "class_ids", hv___Tmp_Ctrl_0);
            GetDlModelParam(hv_DLModelHandle, "class_names", &hv___Tmp_Ctrl_0);
            SetDictTuple(hv_DLDatasetInfo, "class_names", hv___Tmp_Ctrl_0);
        }
        m_pData->bProject = true;
        m_pData->state = stateNormal;
        return true;
    } catch (HDevEngineException &e) {
        m_pData->strError = e.Message();
    } catch (HException &e) {
        m_pData->strError = e.ErrorMessage();
        cout << endl << e.ErrorMessage() << endl;
    } catch (...) {
        m_pData->strError = "DL modeL update fail";
    }
    m_pData->bProject = false;
    m_pData->state = roiError;
    return false;
}

bool HPDetectionPartsCounter::updateProject(std::string strProjectPath,
                                            int trainID) {
#ifndef HALCONCPlus
    if (pDevProcedureCall == NULL) {
        m_pData->strError = "pDevProcedureCall init failed ! ";
        return false;
    }
#endif

    if (!isFolderExist(strProjectPath.c_str())) {
        m_pData->strError = "Update failed! Can not find project path! ";
    }

    try {
        vector<string> fileLists, regionFiles, modelFiles, tupleFiles;
        getFiles(strProjectPath, fileLists);

        // 设置文件过滤格式,将过滤后的文件名称存入到列表中
        regionFiles = fileFilter(fileLists, ".reg");
        modelFiles = fileFilter(fileLists, ".shm");
        tupleFiles = fileFilter(fileLists, ".tup");

        if ((regionFiles.size() == 0) || (modelFiles.size() == 0)) {
            m_pData->strError =
                "Failed ! Can not find 'ModelRegionsOrigin' or 'ModelID' !";
            return false;
        }

        HObject _tmphRegion, hImage;
        m_strRegionPathList.clear();
        m_strModelPathList.clear();
        m_strProjectPath = strProjectPath;

        if (trainID <= 1) // 首次重置，从1开始
        {
            // 重置模型（注意：多分类模型不能重置）
            GenEmptyObj(&m_pData->hModelRegionsOrigin);
            m_pData->hModelID = HTuple();
            m_pData->hModelGrays = HTuple();
        }

        HTuple _tmpModelID, hNum, hArea, hRow, hCol, hModGray;
        for (auto &regionPath : regionFiles) {
            m_pData->hRegionPath = regionPath.data();
            ReadRegion(&_tmphRegion, m_pData->hRegionPath);
            m_strRegionPathList.push_back(regionPath);
            ConcatObj(m_pData->hModelRegionsOrigin, _tmphRegion,
                      &m_pData->hModelRegionsOrigin);
            std::cout << regionPath << std::endl;
        }
        for (auto &modelPath : modelFiles) {
            m_pData->hModelPath = modelPath.data();
            m_strModelPathList.push_back(modelPath);
            ReadShapeModel(m_pData->hModelPath,
                           &_tmpModelID); // todo:改为引入多个模型
            SetShapeModelParam(_tmpModelID, "timeout", -1);
            TupleConcat(m_pData->hModelID, _tmpModelID, &m_pData->hModelID);
            std::cout << modelPath << std::endl;
        }
        for (auto &tuplePath : tupleFiles) {
            ReadTuple(tuplePath.data(), &hModGray);
            TupleConcat(m_pData->hModelGrays, hModGray, &m_pData->hModelGrays);
            std::cout << tuplePath << std::endl;
        }
        m_pData->bProject = true;
        m_pData->state = stateNormal;
        return true;
    } catch (HDevEngineException &e) {
        m_pData->strError = e.Message();
    } catch (HException &e) {
        m_pData->strError = e.ErrorMessage();
        cout << endl << e.ErrorMessage() << endl;
    } catch (...) {
        m_pData->strError = "project update fail";
    }

    m_pData->bProject = false;
    m_pData->state = roiError;
    return false;
}

bool HPDetectionPartsCounter::updateROI(int x, int y, int width, int height) {
#ifndef HALCONCPlus
    if (pDevProcedureCall == NULL) {
        m_pData->strError = "pDevProcedureCall init failed ! ";
        return false;
    }
#endif
    try {
        GenRectangle1(&m_pData->RegionROI, y, x, y + height, x + width);
        m_pData->bRoi = true;
        return true;
    } catch (HDevEngineException &e) {
        m_pData->strError = e.Message();
    } catch (HException &e) {
        m_pData->strError = e.ErrorMessage();
        cout << endl << e.ErrorMessage() << endl;
    } catch (...) {
        m_pData->strError = "roi fail";
    }
    m_pData->bRoi = false;
    m_pData->state = projectError;
    return false;
}

void HPDetectionPartsCounter::stop() { m_pData->bRun = false; }

bool HPDetectionPartsCounter::isWorking() { return m_pData->bRun; }

std::string HPDetectionPartsCounter::errorString() { return m_pData->strError; }

int HPDetectionPartsCounter::getStatus() {
    //     if (!m_pData->bRoi)
    //         return roiError;

    if (!m_pData->bProject)
        return projectError;

    return stateNormal;
}

bool compCols(const ObjData &a, const ObjData &b) {
    // 比较a和b最小外接矩形的x坐标，返回偏小的值
    return a.x < b.x;
}

// bool HPDetectionPartsCounter::action(const uchar *pImageBits,vector<double>
// &resultList)
bool HPDetectionPartsCounter::action(cv::Mat &img, vector<ObjData> &resultList,
                                     cv::Mat &result) {
#ifndef HALCONCPlus
    if (pDevProcedureCall == NULL) {
        m_pData->strError = "pDevProcedureCall init failed ! ";
        return false;
    }
#endif
    // Local iconic variables
    HObject hImage, hObjImage, hObjRegions;
    HTuple hModelPathList, hRegionlPathList, hChannels;
    HTuple hModel, rows, cols, widths, areas, results;
    HTuple hv_Width, hv_Height;
    //     if (!m_pData->bRun)
    //         return false;

    try {
        //         Hlong hWidth;
        //         Hlong hHeight;
        //         uchar *pPixelBlock = NULL;
        //         get_image_pointer1(m_pData->ImageNew, (Hlong*)&pPixelBlock, NULL,
        //         &hWidth, &hHeight); memcpy(pPixelBlock, pImageBits, hWidth *
        //         hHeight);

        // logText(CLog::Info, "pDevProcedureCall Execute");

        matToHobject(img, m_pData->ImageNew);

        // 颜色物料分割算法
        // segment_color(m_pData->ImageNew, &hImage, &hObjRegion,
        // m_pData->WindowHandle, m_pData->hThreshold);

        // 圆螺母缺胶检测算法
        // circle_detector(m_pData->ImageNew, &hObjImage, &hObjRegions,
        // m_pData->WindowHandle, m_pData->hMinDefectInnerRadian,
        // m_pData->hMinDefectArea, m_pData->hDefectThresh, m_pData->hThreshold,
        // &results); area_center(hObjRegions, &areas, &rows, &cols);

        // 精细目标检测算法c++版本
        if (m_halconDLModel == DL_MODE_CLOSED) {
            if (m_pData->hModelID.Length() == 0)
            // if ((m_pData->hModelRegionsOrigin.CountObj() == 0) ||
            // (m_pData->hModelID.Length() == 0))
            {
                m_pData->strError =
                    "Failed ! Can not find 'ModelRegionsOrigin' or 'ModelID' !";
                return false;
            }
#ifdef HALCONCPlus
            // search_shape_model_gray(m_pData->ImageNew, m_pData->RegionROI,
            // m_pData->hModelRegionsOrigin, 	&hObjRegions, m_pData->WindowHandle,
            //m_pData->hModelID, 	m_pData->hModelGrays, m_pData->hThreshold,
            //	m_pData->hWdiffValue, m_pData->hMinScore,
            //m_pData->hMinDefectInnerRadian, 	&rows, &cols, &widths, &areas, &hModel,
            //&results);
#else
            try {
                pDevProcedureCall->SetInputIconicParamObject("ImageNew",
                                                             m_pData->ImageNew);
                pDevProcedureCall->SetInputIconicParamObject("RegionROI",
                                                             m_pData->RegionROI);
                pDevProcedureCall->SetInputIconicParamObject(
                    "ModelRegionsOrigins", m_pData->hModelRegionsOrigin);
                hModelPathList = m_strProjectPath.data();
                hRegionlPathList = m_strProjectPath.data();
                pDevProcedureCall->SetInputCtrlParamTuple("WindowHandle",
                                                          m_pData->WindowHandle);
                pDevProcedureCall->SetInputCtrlParamTuple("ModelIDs",
                                                          m_pData->hModelID);
                pDevProcedureCall->SetInputCtrlParamTuple("ModelGrays",
                                                          m_pData->hModelGrays);
                // pDevProcedureCall->SetInputCtrlParamTuple("RegionPath",
                // hRegionlPathList);
                pDevProcedureCall->SetInputCtrlParamTuple("Threshold",
                                                          m_pData->hThreshold);
                pDevProcedureCall->SetInputCtrlParamTuple("WdiffValue",
                                                          m_pData->hWdiffValue);
                pDevProcedureCall->SetInputCtrlParamTuple("MinScore",
                                                          m_pData->hMinScore);
                pDevProcedureCall->SetInputCtrlParamTuple(
                    "MinDefectInnerRadian", m_pData->hMinDefectInnerRadian);
                pDevProcedureCall->Execute();
            } catch (HDevEngineException &e) {
                m_pData->strError = e.Message();
                std::cout << "halcon error: " << m_pData->strError << std::endl;
                // logText(CLog::Info, m_pData->strError);
                return false;
            } catch (HException &e) {
                m_pData->strError = e.ErrorMessage();
                cout << endl << e.ErrorMessage() << endl;
                return false;
            } catch (...) {
                m_pData->strError = "Execute fail";
                // logText(CLog::Info, m_pData->strError);
                return false;
            }

            // logText(CLog::Info, "pDevProcedureCall Execute end");
            hObjRegions = pDevProcedureCall->GetOutputIconicParamObject("ObjRegions");
            rows = pDevProcedureCall->GetOutputCtrlParamTuple("NewRow");
            cols = pDevProcedureCall->GetOutputCtrlParamTuple("NewColumn");
            widths = pDevProcedureCall->GetOutputCtrlParamTuple("NewWidth");
            areas = pDevProcedureCall->GetOutputCtrlParamTuple("NewArea");
            hModel = pDevProcedureCall->GetOutputCtrlParamTuple("Model");
            results = pDevProcedureCall->GetOutputCtrlParamTuple("Result");
            if (results.Length()) {
                int aa = results[0].I();
                cout << "isDefect: " << aa << endl;
            }

#endif

        } else {

            //   深度学习检测算法
            //
            //***   3.) INSPECT THE ANOMALY IMAGES OF THE LOCAL AND GLOBAL NETWORK ***
            //
            // In the following, we visualize the difference between
            // logical and structural anomalies.
            //
            // As a result one should see that the local network is more
            // likely to detect structural anomalies, while the global network
            // is better at finding logical anomalies. Some of the defects
            // are detected by both subnetworks.
            //
            //
            // Infer images with the local and global network.
            CreateDict(&hv_WindowDict);

            gen_dl_samples_from_images(m_pData->ImageNew, &hv_DLSample);
            preprocess_dl_samples(hv_DLSample, hv_DLPreprocessParam);

            // if (HDevWindowStack::IsOpen())
            //	DispText(HDevWindowStack::GetActive(), "Press F5\nto continue",
            //"window", "bottom", 	"right", "black", HTuple(), HTuple());

            HTuple hv_Result, hv_score, hv_class, hv_classThresh, hv_segmentThresh;

            if (m_halconDLModel == DL_CLASSIFICATION) {
                ApplyDlModel(hv_DLModelHandle, hv_DLSample, HTuple(), &hv_DLResult);
                results = hv_DLResult.TupleGetDictTuple("classification_class_ids");
                int bb = results[0].I();
                hv_score = hv_DLResult.TupleGetDictTuple("classification_confidences");
                hv_class = hv_DLResult.TupleGetDictTuple("classification_class_names");
            } else if (m_halconDLModel == DL_OBJECT_DETECTION_HOR) {
                // dev_display_dl_data(hv_DLSample, hv_DLResult, hv_DLDatasetInfo,
                // "bbox_result", 	HTuple(), hv_WindowDict);
                ApplyDlModel(hv_DLModelHandle, hv_DLSample, HTuple(), &hv_DLResult);
                results = hv_DLResult.TupleGetDictTuple("bbox_class_id");
                hv_score = hv_DLResult.TupleGetDictTuple("bbox_confidence");
                hv_class = hv_DLResult.TupleGetDictTuple("bbox_class_name");
                cols = hv_DLResult.TupleGetDictTuple("bbox_col2");
                rows = (hv_DLResult.TupleGetDictTuple("bbox_row1") +
                        hv_DLResult.TupleGetDictTuple("bbox_row2")) *
                       0.5;
                widths = hv_DLResult.TupleGetDictTuple("bbox_col2") -
                         hv_DLResult.TupleGetDictTuple("bbox_col1");
                areas = (hv_DLResult.TupleGetDictTuple("bbox_row2") -
                         hv_DLResult.TupleGetDictTuple("bbox_row1")) *
                        widths;
                if (results.Length() > 0) {
                    int dd = results[0].D();
                    double score = hv_score.D();
                }
            } else if (m_halconDLModel == DL_OBJECT_DETECTION) {
                // dev_display_dl_data(hv_DLSample, hv_DLResult, hv_DLDatasetInfo,
                // "bbox_result", 	HTuple(), hv_WindowDict);
                ApplyDlModel(hv_DLModelHandle, hv_DLSample, HTuple(), &hv_DLResult);
                results = hv_DLResult.TupleGetDictTuple("bbox_class_id");
                hv_score = hv_DLResult.TupleGetDictTuple("bbox_confidence");
                hv_class = hv_DLResult.TupleGetDictTuple("bbox_class_name");
                cols = hv_DLResult.TupleGetDictTuple("bbox_col");
                rows = hv_DLResult.TupleGetDictTuple("bbox_row");
                widths = hv_DLResult.TupleGetDictTuple("bbox_length1");
                areas = hv_DLResult.TupleGetDictTuple("bbox_length1") *
                        hv_DLResult.TupleGetDictTuple("bbox_length2");
                if (results.Length() > 0) {
                    int dd = results[0].D();
                    double score = hv_score.D();
                }
            } else if (m_halconDLModel == DL_ANOMALY_DETECTION) {
                ApplyDlModel(hv_DLModelHandle, hv_DLSample, "anomaly_image_global",
                             &hv_DLResult);
                threshold_dl_anomaly_results(hv_InferenceSegmentationThreshold,
                                             hv_InferenceClassificationThreshold,
                                             hv_DLResult);
                results = hv_DLResult.TupleGetDictTuple("anomaly_class_id_global");
                // hv_ClassResult =
                // HTuple((hv_DLDatasetInfo.TupleGetDictTuple("class_names"))[hv_class]);

                hv_score = hv_DLResult.TupleGetDictTuple("anomaly_score_global");
                hv_class = hv_DLResult.TupleGetDictTuple("anomaly_class_global");

                hv_classThresh =
                    hv_DLResult.TupleGetDictTuple("anomaly_classification_threshold");
                hv_segmentThresh =
                    hv_DLResult.TupleGetDictTuple("anomaly_segmentation_threshold");
                if (results.Length() > 0) {
                    int dd = results[0].D();
                    double score = hv_score.D();
                    std::cout << "result: " << dd << "/   score: " << score << std::endl;
                }
            }
        }

        DumpWindowImage(
            &hImage,
            m_pData->WindowHandle); // 把算子输出图像保存下来，转成 outImage

        cv::Mat tmp;
        // if (m_halconDLModel == DL_MODE_CLOSED)
        //{
        //	hobjectToMat(hImage, tmp);
        //	result = tmp.clone();
        // }
        // else{

        result = img.clone();
        //}
        // int iMin = rows.Length();
        // iMin = iMin < cols.Length() ? iMin : cols.Length();
        // iMin = iMin < widths.Length() ? iMin : widths.Length();
        // iMin = iMin < areas.Length() ? iMin : areas.Length();
        // iMin = iMin < results.Length() ? iMin : results.Length();

        int iMin;
        if (m_halconDLModel == DL_CLASSIFICATION)
            iMin = 1; // 分类模型一次只输出一个结果（即首个分值最高结果）
        else
            iMin = results.Length();

        ObjData resultData;
        resultData.isOK_halcon = 0; // 检测分类结果默认为0
        for (int i = 0; i < iMin; ++i) {
            resultData.model_id = 0; // 默认0为杂料
            if ((m_halconDLModel == DL_OBJECT_DETECTION_HOR) ||
                (m_halconDLModel == DL_OBJECT_DETECTION) ||
                (m_halconDLModel == DL_MODE_CLOSED)) {
                resultData.x = cols[i].D();
                resultData.y = rows[i].D();
                resultData.width = widths[i].D();
                resultData.area = areas[i].D();
                if (i < hModel.Length()) // 可能存在有料，但是没有匹配modelID的情况
                {
                    resultData.model_id = hModel[i].I();
                }
                resultData.isOK_halcon = results[i].I() + 1;
            } else {
                resultData.isOK_halcon = results[i].I();
            }

            if (m_halconDLModel == DL_MODE_CLOSED) {
                resultData.isOK_halcon = results[i].I();
                // 图片和rectangle录入
                ShapeTrans(hObjRegions, &hObjRegions, "rectangle1");
                HObject ObjSelect;
                HTuple ObjNum, x, y, row2, col2;

                SelectObj(hObjRegions, &ObjSelect, i + 1);
                SmallestRectangle1(ObjSelect, &y, &x, &row2, &col2);
                resultData.objRoi =
                    Rect(x[0].I(), y[0].I(), (col2 - x)[0].I(), (row2 - y)[0].I());
                ReduceDomain(m_pData->ImageNew, ObjSelect, &hObjImage);
                CropDomain(hObjImage, &hObjImage);
                hobjectToMat(hObjImage, tmp);
                resultData.objRoiImage = tmp.clone();
            }

            resultList.push_back(resultData);
        }

        if (resultList.size() > 1)
            std::sort(resultList.begin(), resultList.end(), compCols);

    } catch (HDevEngineException &e) {
        m_pData->strError = e.Message();
    } catch (HException &e) {
        m_pData->strError = e.ErrorMessage();
        cout << endl << e.ErrorMessage() << endl;
    } catch (...) {
        m_pData->strError = "init fail";
    }

    if (m_pData->strError.empty()) {
        m_pData->state = stateNormal;
        m_pData->bRun = true;
        return true;
    }

    return m_pData->strError.empty();
}

// extern "C" TRANSPARENT_PARTSHARED_EXPORT CTransparentPartsCounter *
// createTransparentPartsCounter()
//{
//	return new CTransparentPartsCounter;
// }
