// #include "stdafx.h"
#include "computervisionhal.h"
#include <chrono>
#include <iomanip>

using namespace std;
using namespace cv;

//#define OPENCV
#define TRAIN
#define DEBUG

bool showImg = true;   // false, true
//PartsTrainerHal ptr_partsTrainer_01, ptr_partsTrainer_02;
//PartsCounterHal ptr_partsCounterHal_01, ptr_partsCounterHal_02;
std::shared_ptr<PartsTrainerHal> ptr_partsTrainer_01;
std::shared_ptr<PartsCounterHal> ptr_partsCounterHal_01;
std::shared_ptr<PartsTrainerHal> ptr_partsTrainer_02;
std::shared_ptr<PartsCounterHal> ptr_partsCounterHal_02;
int t_gap;         // 帧间隔时间，0为单帧运行
int suspend_no;     // -1 则连续运行， 否则运行到该帧停下来
int target_imageNO;   // -1 没有指定目标文件夹，否则只运行指定文件夹
string g_strProject_1, g_strProject_2;    //项目模板路径
string g_img_path_1, g_img_path_2;    //待测试图像路径
string g_fileName_1, g_fileName_2;
cv::Mat sourceImg_1, sourceImg_2, trainResult_1, trainResult_2;
cv::Mat* outImg_1, outImg_2;
//int max_image_num_per_path = capture.get(CV_CAP_PROP_FRAME_COUNT);
int max_image_num_per_path = 5000;
double tTime = (double)cv::getTickCount();
double CountTime = (double)cv::getTickCount();
double total_time = 0;
double LastCountTime = 0;
double Difference_time = 0;
int tObj_1, eObj_1, tObj_2, eObj_2;
int OK_num = 0;
int NG_num = 0;

int batchNumber = 0;
int setNumberPerBatch = 4;
int closeFlag = 0;
char fileNo[255];
void run();


int main(int argc, char** argv)
{	

	ptr_partsTrainer_01 = std::make_shared<PartsTrainerHal>();
	ptr_partsCounterHal_01 = std::make_shared<PartsCounterHal>();
	std::string str = ptr_partsCounterHal_01->getVersion();
	ptr_partsTrainer_01->dynamicTrainModeOpen();


	ptr_partsTrainer_02 = std::make_shared<PartsTrainerHal>();
	ptr_partsCounterHal_02 = std::make_shared<PartsCounterHal>();
	std::string str2 = ptr_partsCounterHal_02->getVersion();
	ptr_partsTrainer_02->dynamicTrainModeOpen();


#ifdef DEBUG
	ptr_partsTrainer_01->setCountDebug();
	ptr_partsTrainer_02->setCountDebug();
#endif

	int sampleNo = 1;
	//String s = "E:/VC++  Project/Counter_halcon/20210928/0928error/";
	string root = "E:/VC++  Project/Counter_halcon/20230715/";
	string roiPath_1 = root + "/roi_1.xml";
	string roiPath_2 = root + "/roi_2.xml";
	t_gap = 1;         // 帧间隔时间，0为单帧运行
	suspend_no = -1;     // -1 则连续运行， 否则运行到该帧停下来
	target_imageNO = -1;   // -1 没有指定目标文件夹，否则只运行指定文件夹
	
	//string strProject = root + "22";
	//String img_path = qToStdString(root) + "ng";

	//cv::Mat image_sample, image_sample_roi;
	//image_sample = cv::imread("E:/VC++  Project/Counter_halcon/20210911/0909V/1-2 opencv2/0021.png", cv::IMREAD_GRAYSCALE);
	//if (image_sample.empty())
	//{
	//	throw std::exception();
	//}
	//cv::resize(image_sample, image_sample, cv::Size(320, 240));
	//int roiLTX = 0;
	//int roiLTY = 0;
	//int roiWidth = 0;
	//int roiHeight = 0;
	//partsCounter.autoSetRoi(&image_sample, roiLTX, roiLTY, roiWidth, roiHeight);
	//// 获取模型的特征点
	//image_sample_roi = image_sample(Rect(roiLTX, roiLTY, roiWidth, roiHeight));

	//设置文件过滤器
	vector<string> dirLists, projectLists, projectDirs_1, projectDirs_2, imageDirs;
	getFiles(root, dirLists);
	for (auto& strProject : dirLists) {
		projectLists.clear();
		getFiles(strProject, projectLists);
		//设置文件过滤格式,将过滤后的文件名称存入到列表中
		projectDirs_1 = fileFilter(projectLists, "project_1");  //项目路径下没有project文件，说明路径无效
		projectDirs_2 = fileFilter(projectLists, "project_2");  //项目路径下没有project文件，说明路径无效
		if (projectDirs_1.size() == 0 || projectDirs_2.size() == 0)
			continue;
		
		//如果设置了目标物料编号则只检测该物料
		int imageNo = 0;
		try{
			imageNo = std::stoi(strProject.substr(strProject.find_last_of("\\") + 1));
		}
		catch(...)
		{
			cout << "Error: imageNo string to int. 目录结构异常,请重新检查" << endl;

		}
		if (target_imageNO >= 0 && imageNo != target_imageNO) continue;

		// std::cout << "Path Error!  Found no projects! " << std::endl;
		g_strProject_1 = projectDirs_1[0]; //仅取第一个project
		g_strProject_2 = projectDirs_2[0]; //仅取第一个project

		std::cout << strProject << std::endl;
		for (auto& imagePath : projectLists) {
			OK_num = 0;
			NG_num = 0;
			int pos0 = imagePath.find("1ok");
			if (pos0 >= 0) {
				OK_num = std::stoi(imagePath.substr(imagePath.find_last_of("-") + 1));
			}
			int pos1 = imagePath.find("1ng");
			if (pos1 >= 0) {
				NG_num = std::stoi(imagePath.substr(imagePath.find_last_of("-") + 1));
			}
			if ((pos0 < 0) && (pos1 < 0)) continue;

			g_img_path_1 = imagePath;

			std::cout << "qimg_path: " << imagePath << std::endl;
			g_fileName_1 = imagePath;
		}

		bool flag = true;     //项目更新flag
		roiPath_1 = strProject + "\\roi_1.xml";
		roiPath_2 = strProject + "\\roi_2.xml";
		if (!isFolderExist(roiPath_1.data())){
			roiPath_1 = root + "roi_1.xml";
		}
		if (!isFolderExist(roiPath_2.data())){
			roiPath_2 = root + "roi_2.xml";
		}
#ifdef TRAIN            
		// 训练
#ifdef OPENCV
		ptr_partsTrainer_01->reset(g_strProject_1, roiPath_1);
		ptr_partsTrainer_02->reset(g_strProject_2, roiPath_2);
#else
		flag &= ptr_partsTrainer_01->HPDetection_start(g_strProject_1, roiPath_1);
		ptr_partsTrainer_01->setTrainTarget(1); // 设置单个物料训练超过3次才保存结果

		flag &= ptr_partsTrainer_02->HPDetection_start(g_strProject_2, roiPath_2);
		ptr_partsTrainer_02->setTrainTarget(1); // 设置单个物料训练超过3次才保存结果
#endif // OPENCV

#else
		// 检测
		/*if(strProject.find("project1"))
			strProject = "";*/
#ifdef OPENCV
		flag &= ptr_partsCounterHal_01->updateProject(g_strProject);
		flag &= ptr_partsCounterHal_02->updateProject(g_strProject);
#else
		flag &= ptr_partsCounterHal_01->updateProjectHP(g_strProject);
		flag &= ptr_partsCounterHal_02->updateProjectHP(g_strProject);
#endif // OPENCV
		if (!flag) {
			std::cout << "update project failed! " << std::endl;
			cv::waitKey(0);
			return 0;
		}

#endif // TRAIN

		flag = ptr_partsCounterHal_01->updateRoi(roiPath_1);
		flag &= ptr_partsCounterHal_02->updateRoi(roiPath_2);
		if (!flag) {
			std::cout << "update roi failed! " << std::endl;
			cv::waitKey(0);
			return 0;
		}

		//cv::VideoCapture capture("H:/parts_counting_opencv_1015/parts_counting/x64/Debug/logimage/20161017/M001/a1/videotest.avi"); 


		run();


#ifdef TRAIN            
#ifdef OPENCV
		ptr_partsTrainer_01->writeTrainDataToXml();
		ptr_partsTrainer_01->cleanData();

		ptr_partsTrainer_02->writeTrainDataToXml();
		ptr_partsTrainer_02->cleanData();
#else
		ptr_partsTrainer_01->HPDetection_finish();
		ptr_partsTrainer_02->HPDetection_finish();
#endif // OPENCV
			
		cout << endl << endl << "train finish" << endl << endl;
		cv::waitKey(500);
#endif // TRAIN
		

	}
	//while (1);
	return 0;
}




void run() {

#ifdef TRAIN  
	string path_1 = g_strProject_1;
	string path_2 = g_strProject_2;
#else
	string path_1 = g_img_path_1; 
	string path_2 = g_img_path_2;
#endif

	vector<string> files_1, files_2;
	getFiles(path_1, files_1);
	getFiles(path_2, files_2);

	char resultName[255];
	sprintf_s(resultName, "%s/out_1.txt", path_1.c_str());
	ofstream out_1(resultName, ios::out | ios::binary);
	sprintf_s(resultName, "%s/out_2.txt", path_2.c_str());
	ofstream out_2(resultName, ios::out | ios::binary);

	int lastTimeStamp = 0;
	int index = 1;
	out_1 << path_1 << "\n";
	out_2 << path_2 << "\n";

	std::cout << "files path_1: " << path_1 << std::endl;
	std::cout << "files path_2: " << path_2 << std::endl;
	int totalObj_1 = 0;
	int totalObj_2 = 0;
	int totalError_1 = 0;
	int totalError_2 = 0;
	int currentNumber_1 = 0;
	int currentNumber_2 = 0;
	int trainNum_1 = 0;
	int trainNum_2 = 0;
	int lastTrainNum_1 = 0;
	int lastTrainNum_2 = 0;
	for (int j = 0; j < min(files_1.size(), files_2.size()); j++)
	{
		string filePath_1 = files_1[j];
		string filePath_2 = files_2[j];
		sourceImg_1 = cv::imread(filePath_1.c_str(), cv::IMREAD_GRAYSCALE);
		sourceImg_2 = cv::imread(filePath_2.c_str(), cv::IMREAD_GRAYSCALE);

		//判断图像是否为空
		if (sourceImg_1.empty() || sourceImg_2.empty())
		{
			continue;
		}

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
		filePath.replace(unsigned int(pos) + 1, unsigned int(pos_end - pos - 1), fileNo);
		int a = rename(files[j].c_str(), filePath.c_str());
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
		ptr_partsTrainer_01->training(&sourceImg_1, &trainResult_1, 200);   //OPENCV
		ptr_partsTrainer_01->saveData();
		cv::imshow("trainResult", trainResult_1);
		cv::waitKey(0);

		ptr_partsTrainer_02->training(&sourceImg_2, &trainResult_2, 200);   //OPENCV
		ptr_partsTrainer_02->saveData();
		cv::imshow("trainResult", trainResult_2);
		cv::waitKey(0);
#else
		ptr_partsTrainer_01->HPDetection_train(sourceImg_1, trainResult_1);      //HALCON训练
		trainNum_1 = ptr_partsTrainer_01->getTrainedObjNum();  // 获取已训练模型个数
		int objNum_1 = ptr_partsTrainer_01->getDetectObjNum();  // 获取已训练模型个数
		out_1 << "detected Obj num_1: " << objNum_1 << ", trained Obj num: " << trainNum_1 << ",   time: " << getTimeString() << std::endl;
		std::cout << "detected Obj num_1: " << objNum_1 << ", trained Obj num: " << trainNum_1 << ",   time: " << getTimeString() << std::endl;
		cv::imshow("trainResult_1", trainResult_1);
		int KEY = cv::waitKey(50);
		if (KEY > 0)
		{// 如果触发按键，手动调用计数器
			ptr_partsTrainer_01->trainCountAdd();
		}
		if (trainNum_1 != lastTrainNum_1)
		{
			cv::Mat trainImg_1 = ptr_partsTrainer_01->getTrainedImage();
			cv::imshow("trainResult_1", trainImg_1);
			cv::waitKey(500);
		}
		lastTrainNum_1 = trainNum_1;


		ptr_partsTrainer_02->HPDetection_train(sourceImg_2, trainResult_2);      //HALCON训练
		trainNum_2 = ptr_partsTrainer_02->getTrainedObjNum();  // 获取已训练模型个数
		int objNum_2 = ptr_partsTrainer_02->getDetectObjNum();  // 获取已训练模型个数
		out_2 << "detected Obj num_2: " << objNum_2 << ", trained Obj num: " << trainNum_2 << ",   time: " << getTimeString() << std::endl;
		std::cout << "detected Obj num_2: " << objNum_2 << ", trained Obj num: " << trainNum_2 << ",   time: " << getTimeString() << std::endl;
		cv::imshow("trainResult_2", trainResult_2);
		KEY = cv::waitKey(50);
		if (KEY > 0)
		{// 如果触发按键，手动调用计数器
			ptr_partsTrainer_02->trainCountAdd();
		}
		if (trainNum_2 != lastTrainNum_2)
		{
			cv::Mat trainImg_2 = ptr_partsTrainer_02->getTrainedImage();
			cv::imshow("trainResult_2", trainImg_2);
			cv::waitKey(500);
		}
		lastTrainNum_2 = trainNum_2;
#endif // OPENCV

#else

#ifdef OPENCV

		if ((showImg) && (1 == sourceImg_1.channels()))
			cvtColor(sourceImg_1, sourceImg_1, CV_GRAY2BGR);
		if ((showImg) && (1 == sourceImg_2.channels()))
			cvtColor(sourceImg_2, sourceImg_2, CV_GRAY2BGR);

		tTime = (double)cv::getTickCount();
		ptr_partsCounterHal_01->action(&sourceImg_1, showImg);
		ptr_partsCounterHal_02->action(&sourceImg_2, showImg);
		tTime = ((double)cv::getTickCount() - tTime) / cv::getTickFrequency();

		if (showImg) {
			putText(sourceImg_1, to_string(j), Point(30, 30), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 225, 255), 2);
			cv::imshow("sourceImg_1", sourceImg_1);

			putText(sourceImg_2, to_string(j), Point(30, 30), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 225, 255), 2);
			cv::imshow("sourceImg_2", sourceImg_2);
			cv::waitKey(t_gap);
		}
#else
		tTime = (double)cv::getTickCount();
		ptr_partsCounterHal_01->actionHPDetectionObjectMat(&sourceImg_1, showImg);
		ptr_partsCounterHal_02->actionHPDetectionObjectMat(&sourceImg_2, showImg);
		tTime = ((double)cv::getTickCount() - tTime) / cv::getTickFrequency();

		if (showImg) {
			putText(sourceImg_1, to_string(j), Point(30, 30), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 225, 255), 2);
			cv::imshow("sourceImg_1", sourceImg_1);

			putText(sourceImg_2, to_string(j), Point(30, 30), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 225, 255), 2);
			cv::imshow("sourceImg_2", sourceImg_2);
			cv::waitKey(t_gap);
		}

#endif // OPENCV

		// 统计时间
		total_time += tTime;

		tObj_1 = ptr_partsCounterHal_01->getTargetNumber();
		eObj_1 = ptr_partsCounterHal_01->getErrorNumber();
		std::pair<unsigned int, unsigned int> mObj_1 = ptr_partsCounterHal_01->getObjectNumber();
		int ok_obj_1 = mObj_1.first;
		int ng_obj_1 = mObj_1.second;

		tObj_2 = ptr_partsCounterHal_02->getTargetNumber();
		eObj_2 = ptr_partsCounterHal_02->getErrorNumber();
		std::pair<unsigned int, unsigned int> mObj_2 = ptr_partsCounterHal_02->getObjectNumber();
		int ok_obj_2 = mObj_2.first;
		int ng_obj_2 = mObj_2.second;
		// 批次数统计
		//if (tObj_1 || eObj_1)
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
		if ((ok_obj_1> 0) || (ng_obj_1 > 0))
		{
			std::cout << "ok num_1: " << ok_obj_1 << ",  ng num_1: " << ng_obj_1 << "ok num_2: " << ok_obj_2 << ",  ng num_2: " << ng_obj_2 << ",   frame: " << j << "   use time:" << tTime << endl;
		}

		if (out.is_open())
		{
			// 输出每个料计数位置
			if (tObj_1)
			{
				int pos2 = filePath.find_first_of("\\");
				string filename = filePath.substr(pos2 + 1);
				out << filename.c_str() << "\n";
				cout << filename.c_str() << "\n";
				totalObj = totalObj + tObj_1;
				out << "target num: " << tObj_1 << "  in:" << g_fileName << ". frameID:" << index << ", use time: " << tTime << "  total obj: " << totalObj << endl;
				cout << "target num: " << tObj_1 << "  in:" << g_fileName << ". frameID:" << index << ", use time: " << tTime << "  total obj: " << totalObj << endl;
			}
			if (eObj_1)
			{
				int pos2 = files[j].find_first_of("\\");
				string filename = files[j].substr(pos2 + 1);
				out << filename.c_str() << "\n";
				cout << filename.c_str() << "\n";
				totalError = totalError + eObj_1;
				out << "error num: " << eObj_1 << "  in:" << g_fileName << ". frameID:" << index << ", use time: " << tTime << endl;
				cout << "error num: " << eObj_1 << "  in:" << g_fileName << ". frameID:" << index << ", use time: " << tTime << endl;
			}
			//if(tObj_1 + eObj_1 > 0)
			//	cv::waitKey(0);
		}
#endif // TRAIN

		index++;
	}


#ifdef TRAIN     
	out_1 << endl << endl << "train finish" << endl << endl;
	out_2 << endl << endl << "train finish" << endl << endl;
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

	//cv::waitKey(0);
#endif // TRAIN

}