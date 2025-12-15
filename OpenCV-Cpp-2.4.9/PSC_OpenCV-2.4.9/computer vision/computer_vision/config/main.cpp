// #include "stdafx.h"
#include "computervisionhal.h"
//#include "qt_halcon.h"
#include <chrono>
#include <iomanip>

using namespace std;
using namespace cv;

#define OPENCV
//#define TRAIN
#define DEBUG

bool showImg = true;   // false, true

std::shared_ptr<PartsTrainerHal> ptr_partsTrainer;
std::shared_ptr<PartsCounterHal> partsCounterHal;
int t_gap;         // 帧间隔时间，0为单帧运行
int suspend_no;     // -1 则连续运行， 否则运行到该帧停下来
int target_imageNO;   // -1 没有指定目标文件夹，否则只运行指定文件夹
string g_strProject;    //项目模板路径
string g_img_path;    //待测试图像路径
string g_fileName;
cv::Mat sourceImg, trainResult;
cv::Mat* outImg;
//int max_image_num_per_path = capture.get(CV_CAP_PROP_FRAME_COUNT);
int max_image_num_per_path = 5000;
double tTime = (double)cv::getTickCount();
double CountTime = (double)cv::getTickCount();
double total_time = 0;
double LastCountTime = 0;
double Difference_time = 0;
double scaleFactor = 1.0;
int NGEndingNumLast = 0;
int tObj, eObj;
int OK_num = 0;
int NG_num = 0;

bool flag = true;     //项目参数更新flag
int batchNumber = 0;
int setNumberPerBatch = 4;
int closeFlag = 0;
char fileNo[255];
void run();


int main(int argc, char** argv)
{	
	//cv::Mat image_calib, tmp;
	//image_calib = cv::imread("E:/VC++  Project/1.PartsCounter/Counter_halcon/20240408/image/ok-8/20240306110219873.png", cv::IMREAD_COLOR);   //IMREAD_COLOR   // IMREAD_GRAYSCALE
	//if (image_calib.empty())
	//{
	//	throw std::exception();
	//}
	//HObject hImage;
	//matToHobject(image_calib, hImage);
	//hobjectToMat(hImage, tmp);


	ptr_partsTrainer = std::make_shared<PartsTrainerHal>();
	partsCounterHal = std::make_shared<PartsCounterHal>();
	std::string str = partsCounterHal->getVersion();
	//ptr_partsTrainer->dynamicTrainModeOpen();    // 打开动态训练模式，连续训练统计物料数量达到目标才算训练成功
	partsCounterHal->setHardTriggerMode(true);   // 设置硬触发模式，无结果跟踪效果
	flag = partsCounterHal->setDLMode(DL_MODE_CLOSED);  // 设置深度学习检测模式，如果用传统halcon精细检测可以设置为DL_MODE_CLOSED
	//*   halcon深度学习模型类型    *//
	//	DL_CLASSIFICATION,          // 分类模型
	//	DL_OBJECT_DETECTION,        // 目标检测模型
	//	DL_ANOMALY_DETECTION,       // 异常检测模型
	//	DL_OBJECT_DETECTION_HOR,    // 目标检测模型-水平框
	//	DL_MODE_CLOSED              // 关闭DL模式，采用传统halcon精细检测模型


#ifdef DEBUG
	ptr_partsTrainer->setCountDebug();    // 视觉算法调试开关
#endif


#ifndef OPENCV
	ptr_partsTrainer->setHalonMode(true);    // 设置halcon模式
	partsCounterHal->setNGEndingCountError(false);        // 关闭空降打杂
#endif // OPENCV

	//String s = "E:/VC++  Project/Counter_halcon/20210928/0928error/";
	string root = "E:/VC++  Project/1.PartsCounter/Counter_halcon/20250921/20250921/";
	//string root = "E:/DL/Halcon_DL/20250721轴对称/";
	string roiPath = root + "/roi.xml";
	t_gap = 1;         // 帧间隔时间，0为单帧运行
	suspend_no = -1;     // -1 则连续运行， 否则运行到该帧停下来
	target_imageNO = -1;   // -1 没有指定目标文件夹，否则只运行指定文件夹
	//ptr_partsTrainer->dynamicTrainModeOpen();
#ifdef DEBUG
	ptr_partsTrainer->setCountDebug();
#endif

#ifdef TRAIN   
	//cv::Mat image_calib;
	//image_calib = cv::imread("E:/VC++  Project/Counter_halcon/20250109/1280x1024/0.png", cv::IMREAD_GRAYSCALE);
	//if (image_calib.empty())
	//{
	//	throw std::exception();
	//}

	//scaleFactor = ptr_partsTrainer->getCalibFactor(image_calib);
#endif
		
	// 更新ROI参数
	roiPath = root + "\\roi.xml";
	if (!isFolderExist(roiPath.data()))
		roiPath = root + "roi.xml";

	//flag &= ptr_partsTrainer->HPDetection_start(g_strProject, roiPath);

	flag = partsCounterHal->updateRoi(roiPath);

	if (!flag) {
		std::cout << "update roi failed! " << std::endl;
		cv::waitKey(0);
		return 0;
	}



	//设置文件过滤器
	vector<string> dirLists, projectLists, projectDirsRoot, projectDirs, imageDirsRoot, imageDirs;
	getFiles(root, dirLists);

	//设置文件过滤格式,将过滤后的文件名称存入到列表中
	projectDirsRoot = fileFilter(dirLists, "project");  //项目路径下没有project文件，说明路径无效
	getFiles(projectDirsRoot[0], projectDirs);
	//projectDirs = fileFilter(projectDirs, "project_");
	if (projectDirs.size() == 0) 	
		return 0;

	//如果设置了目标物料编号则只检测该物料
	//int imageNo = 0;
	//try{
	//	imageNo = std::stoi(strProject.substr(strProject.find_last_of("\\") + 1));
	//}
	//catch(...)
	//{
	//	cout << "Error: imageNo string to int. 目录结构异常,请重新检查" << endl;

	//}
	//if (target_imageNO >= 0 && imageNo != target_imageNO) continue;

	// std::cout << "Path Error!  Found no projects! " << std::endl;
	g_strProject = projectDirs[0]; //仅取第一个project




#ifdef TRAIN            // 训练模块
		
	for (int index = 0; index < projectDirs.size(); index++) {

		g_strProject = projectDirs[index]; //imagePath 此时也是 project路径
		std::cout << "g_strProject: " << g_strProject << std::endl;
		g_img_path = g_strProject;
#ifdef OPENCV
		ptr_partsTrainer->reset(g_strProject, roiPath);
#else
		// 重新载入项目
		//ptr_partsTrainer->updateParam_train();
		//ptr_partsTrainer->dynamicTrainModeOpen();
		flag &= ptr_partsTrainer->HPDetection_start(g_strProject, roiPath);
#endif // OPENCV

	ptr_partsTrainer->setTrainTarget(1); // 设置单个物料训练超过3次才保存结果

	run();   //每个路径执行一轮训练


		// 完成训练后保存数据
#ifdef OPENCV
		ptr_partsTrainer->writeTrainDataToXml();
		ptr_partsTrainer->cleanData();
#else
		ptr_partsTrainer->HPDetection_finish();
#endif // OPENCV

		std::cout << endl << endl << "train finish" << endl << endl;
		cv::waitKey(500);

	}

#else   //检测模块
	
	if (m_halconDLModel == DL_MODE_CLOSED)     // 传统halcon精细检测算法更新模型
	{
		partsCounterHal->reset();   //清空已读取训练模型
		//projectDirs = fileFilter(dirLists, "project");  //项目路径下没有project文件，说明路径无效
		//if (projectDirs.size() == 0)
		//	return 0;


		////getFiles(projectDirsRoot[0], projectDirs);    // 同时获取多个项目
		//projectDirs = fileFilter(projectDirs, "project_");
		////projectDirs = fileFilter(dirLists, "image");
		//if (projectDirs.size() == 0)
		//	return 0;


		//for (int index = 0; index < projectDirs.size(); index++) {

		//	g_strProject = projectDirsRoot[0]; //imagePath 此时也是 project路径
		//std::cout << g_strProject << std::endl;


#ifdef OPENCV
			flag &= partsCounterHal->updateProject(g_strProject);
#else
			flag &= partsCounterHal->updateProjectHP(g_strProject);
#endif // OPENCV
			if (!flag) {
				std::cout << "update project failed! " << std::endl;
				cv::waitKey(0);
				return 0;
			}
		//}
	}
	else {  // 深度学习更新模型
		std::wstring model_Path = L"./config/model_objDetector-250715.hdl";
		// 执行检测模型
		
		flag &= partsCounterHal->updateDLMode(model_Path);
	
	}

	imageDirsRoot = fileFilter(dirLists, "image");  //项目路径下没有project文件，说明路径无效
	if (imageDirsRoot.size() == 0)
		return 0;
	getFiles(imageDirsRoot[0], imageDirs);

	dirLists = fileFilter(imageDirs, "");
	for (int No = 0; No < dirLists.size(); No++) {
		g_img_path = dirLists[No];
		std::cout << "qimg_path: " << g_img_path << std::endl;
		g_fileName = g_img_path;

		OK_num = 0;
		NG_num = 0;

		run();   //全部项目导入后才执行检测程序
	}

#endif // TRAIN

	return 0;
}




void run() {

//#ifdef TRAIN  
//	string path = g_strProject;
//#else
//	string path = g_img_path;
//#endif

	string path = g_img_path;

	vector<string> files;
	files.clear();
	getFiles(path, files);

	char resultName[255];
	sprintf_s(resultName, "%s/out.txt", path.c_str());
	ofstream out(resultName, ios::out | ios::binary);


	int lastTimeStamp = 0;
	int index = 1;
	out << path << "\n";

	std::cout << "files path: " << path << std::endl;
	int totalObj = 0;
	int totalError = 0;
	int currentNumber = 0;
	int trainNum = 0;
	int lastTrainNum = 0;
	for (int j = 0; j < files.size(); j++)
	{
		string filePath = files[j];
		sourceImg = cv::imread(filePath.c_str());

		//判断图像是否为空
		if (sourceImg.empty())
		{
			continue;
		}

		std::cout << "filePath:" << filePath << std::endl;
#ifndef TRAIN  
		int pos = filePath.find_last_of("\\");
		int pos_end = filePath.find_last_of(".");
		//if (strcmp(filePath.substr(pos + 1, 2).c_str(), files[j - 1].substr(pos + 1, 2).c_str()))
		//{
		//	index = 1;
		//}

		sprintf_s(fileNo, "%04d", index);

		// 统计帧间隔时间差
		//string fileName = filePath.substr(pos + 1 + 12, 5);
		//int timeStamp = std::stoi(fileName);
		//if ((timeStamp - lastTimeStamp > 20) && (j > 1))
		//{
		//	std::cout << files[j-1] << std::endl;
		//	std::cout << filePath << std::endl;
		//	std::cout << "time interval: " << int(timeStamp - lastTimeStamp) << std::endl;
		//}
		//lastTimeStamp = timeStamp;


		// 更改文件名,成功返回0，否则返回-1
		//filePath.replace(unsigned int(pos) + 1, unsigned int(pos_end - pos - 1), fileNo);
		//int a = rename(files[j].c_str(), filePath.c_str());
#endif  

		//capture >> sourceImg;
		//std::cout << j << std::endl;
		if ((j == suspend_no) && (suspend_no > 0))
		{
			int a;
			cin >> a;
		}

		//cv::resize(sourceImg, sourceImg, cv::Size(320, 240));

#ifdef TRAIN     

#ifdef OPENCV
		bool bok = ptr_partsTrainer->training(&sourceImg, &trainResult, 200);   //OPENCV
		

		cv::imshow("trainResult", trainResult);
		cv::waitKey(50);
		//if (m_isDynamic == 0)
		//{
		//	ptr_partsTrainer->saveData();
		//}
#else

		bool flag = ptr_partsTrainer->HPDetection_train(sourceImg, trainResult);      //HALCON训练


		cv::imshow("trainResult", trainResult);

		int KEY = cv::waitKey(50);		
 	//	if (KEY > 0)
		//{// 如果触发按键，手动调用计数器
		//	ptr_partsTrainer->trainCountAdd();
		//}

		trainNum = ptr_partsTrainer->getTrainedObjNum();  // 获取已训练模型个数
		int objNum = ptr_partsTrainer->getDetectObjNum();  // 获取已训练模型个数
		out << "detected Obj num: " << objNum << ", trained Obj num: " << trainNum << ",   time: " << getTimeString() << std::endl;
		std::cout << "detected Obj num: " << objNum << ", trained Obj num: " << trainNum << ",   time: " << getTimeString() << std::endl;


		if (trainNum != lastTrainNum)
		{
			cv::Mat trainImg = ptr_partsTrainer->getTrainedImage();
			string writePath = g_strProject + "/train_" + to_string(trainNum) + ".jpg";
			cv::imwrite(writePath.c_str(), trainImg);          // 保存训练图片 todo

			cv::Size trainSize = ptr_partsTrainer->getTrainedSize();
			double realObjWidth = trainSize.width * scaleFactor;
			double realObjHeight = trainSize.height * scaleFactor;
			cv::imshow("trainResult", trainImg);
			cv::waitKey(500);
		}
		lastTrainNum = trainNum;
#endif // OPENCV

#else

#ifdef OPENCV

		if ((showImg) && (1 == sourceImg.channels()))
			cvtColor(sourceImg, sourceImg, CV_GRAY2BGR);

		tTime = (double)cv::getTickCount();
		partsCounterHal->action(&sourceImg, showImg);
		tTime = ((double)cv::getTickCount() - tTime) / cv::getTickFrequency();

		if (showImg) {
			putText(sourceImg, to_string(j), Point(30, 30), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 225, 255), 2);
			cv::imshow("sourceImg", sourceImg);
			cv::waitKey(t_gap);
		}
#else
		tTime = (double)cv::getTickCount();
		partsCounterHal->actionHPDetectionObjectMat(&sourceImg, showImg);	
		tTime = ((double)cv::getTickCount() - tTime) / cv::getTickFrequency();
		if (showImg) {
			putText(sourceImg, to_string(j), Point(30, 30), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 225, 255), 2);
			cv::imshow("sourceImg", sourceImg);
			cv::waitKey(10);
		}
#endif // OPENCV

		// 统计时间
		total_time += tTime;

		tObj = partsCounterHal->getTargetNumber();
		eObj = partsCounterHal->getErrorNumber();
		vector<int> matchIDs;
		matchIDs = partsCounterHal->getMatchIDs();   // 获取物料编号，正常从1开始，全部都未匹配上的杂料，id为0
		std::pair<unsigned int, unsigned int> mObj = partsCounterHal->getObjectNumber();
		int ok_obj = mObj.first;
		int ng_obj = mObj.second;
		// 批次数统计
		//if (tObj || eObj)
		//{
		//	out << endl << "time: " << tTime;
		//Difference_time = j - LastCountTime;
		//if (Difference_time < 0)
		//{
		//	Difference_time = max_image_num_per_path - LastCountTime + j;
		//}
		//LastCountTime = j;

		//currentNumber++;
		//}
		if (tTime > 0.015){
			std::cout << "frame: " << j << "   use time:" << tTime << endl;
			//cv::waitKey(0);
		}
		if ((ok_obj > 0) || (ng_obj > 0))
		{
			std::cout << "ok num: " << ok_obj << ",  ng num: " << ng_obj << ",   frame: " << j << "   use time:" << tTime << endl;
			for (auto i : matchIDs)
				std::cout << "  matchID: " << i << ", ";
			std::cout << endl;
			
			if (m_halconDLModel == DL_MODE_CLOSED)     // 传统halcon精细检测算法更新模型
			{
				std::vector<ObjData> objData = partsCounterHal->getObjData();
				for (int i = 0; i < objData.size(); i++)
				{
					cv::imshow("ObjImg", objData[i].objRoiImage);
					cv::waitKey(0);
				}
			}
			cv::waitKey(0);
		}

		int NGEndingNum = partsCounterHal->getNGEndingNumber();
		if (NGEndingNum > NGEndingNumLast)
		{
			std::cout << "m_NGEndingNum: " << NGEndingNum << endl;
			NGEndingNumLast = NGEndingNum;
		}
		//int aaa = j % 48;
		//int bbb = j / 48;
		//if ((j % 48 == 0) && (NGEndingNum > mySysParam.NGEndingThresh))
		//{
		//	//ptr_partsCounterHal->reset();  // 每4800帧判断一次空降料数量m_NGEndingNum，是否大于阈值NGEndingThresh（默认为3）
		//	cv::waitKey(0);
		//}
		if (out.is_open())
		{
			// 输出每个料计数位置
			if (tObj)
			{
				int pos2 = filePath.find_first_of("\\");
				string filename = filePath.substr(pos2 + 1);
				out << filename.c_str() << "\n";
				cout << filename.c_str() << "\n";
				totalObj = totalObj + tObj;
				out << "target num: " << tObj << "  in:" << g_fileName << ". frameID:" << index << ", use time: " << tTime << "  total obj: " << totalObj << endl;
				cout << "target num: " << tObj << "  in:" << g_fileName << ". frameID:" << index << ", use time: " << tTime << "  total obj: " << totalObj << endl;
				for (auto i : matchIDs)
					std::cout << "  matchID: " << i << ", ";
				std::cout << endl;
			}
			if (eObj)
			{
				int pos2 = files[j].find_first_of("\\");
				string filename = files[j].substr(pos2 + 1);
				out << filename.c_str() << "\n";
				cout << filename.c_str() << "\n";
				totalError = totalError + eObj;
				out << "error num: " << eObj << "  in:" << g_fileName << ". frameID:" << index << ", use time: " << tTime << endl;
				cout << "error num: " << eObj << "  in:" << g_fileName << ". frameID:" << index << ", use time: " << tTime << endl;
			}

			if(tObj + eObj > 0)
				cv::waitKey(0);
		}
#endif // TRAIN

		index++;
	}


#ifdef TRAIN     
	out << endl << endl << "train finish" << endl << endl;
#else
	std::cout << "\n  \n  total obj: " << totalObj << endl;
	std::cout << "\n  total error: " << totalError << endl;
	std::cout << "\n  total time: " << total_time << endl;

	out << "\n  \n  total obj: " << totalObj << endl;
	out << "\n  total error: " << totalError << endl;
	out << "\n  total time: " << total_time << endl;

	if ((totalObj != OK_num) || (totalError != NG_num)) {
		std::cout << endl << endl << "Error!!!!  true result is: " << endl;
		std::cout << " OK = " << OK_num << ",  NG = " << NG_num;
		//cv::waitKey(0);
	}
	else {
		std::cout << endl << " Good !!!    Result matched!!" << endl;
	}

	std::cout << endl << endl << "detect finish" << endl << endl;
	out.close();

	cv::waitKey(0);
#endif // TRAIN

}