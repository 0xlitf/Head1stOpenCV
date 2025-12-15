// #include "stdafx.h"
#include "computervisionhal.h"
#include <chrono>
#include <iomanip>

using namespace std;
using namespace cv;

#define OPENCV
#define TRAIN
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
int tObj, eObj;
int OK_num = 0;
int NG_num = 0;

int batchNumber = 0;
int setNumberPerBatch = 4;
int closeFlag = 0;
char fileNo[255];
void run();


int main(int argc, char** argv)
{	
	ptr_partsTrainer = std::make_shared<PartsTrainerHal>();
	partsCounterHal = std::make_shared<PartsCounterHal>();
	std::string str = partsCounterHal->getVersion();

	int sampleNo = 1;
	//String s = "E:/VC++  Project/Counter_halcon/20210928/0928error/";
	string root = "E:/VC++  Project/Counter_halcon/20240306-Match/";
	string roiPath = root + "/roi.xml";
	t_gap = 1;         // 帧间隔时间，0为单帧运行
	suspend_no = -1;     // -1 则连续运行， 否则运行到该帧停下来
	target_imageNO = -1;   // -1 没有指定目标文件夹，否则只运行指定文件夹
	ptr_partsTrainer->dynamicTrainModeOpen();
#ifdef DEBUG
	ptr_partsTrainer->setCountDebug();
#endif
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
	vector<string> dirLists, projectLists, projectDirs, imageDirs;
	getFiles(root, dirLists);
	for (auto& strProject : dirLists) {
		projectLists.clear();
		getFiles(strProject, projectLists);
		//设置文件过滤格式,将过滤后的文件名称存入到列表中
		projectDirs = fileFilter(projectLists, "project");  //项目路径下没有project文件，说明路径无效
		if (projectDirs.size() == 0) 	
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
		g_strProject = projectDirs[0]; //仅取第一个project

		std::cout << strProject << std::endl;
		for (auto& imagePath : projectLists) {
			OK_num = 0;
			NG_num = 0;
#ifdef TRAIN   
			int pos0 = imagePath.find("image");
			if (pos0 < 0) continue;
#else
			int pos0 = imagePath.find("ok");
			if (pos0 >= 0) {
				OK_num = std::stoi(imagePath.substr(imagePath.find_last_of("-") + 1));
			}
			int pos1 = imagePath.find("ng");
			if (pos1 >= 0) {
				NG_num = std::stoi(imagePath.substr(imagePath.find_last_of("-") + 1));
			}			
			if ((pos0 < 0) && (pos1 < 0)) continue;
#endif

			g_img_path = imagePath;

			std::cout << "qimg_path: " << imagePath << std::endl;
			g_fileName = imagePath;

			//while (1){
			bool flag = true;     //项目更新flag
			roiPath = strProject + "\\roi.xml";
			if(!isFolderExist(roiPath.data()))
				roiPath = root + "roi.xml";


#ifdef TRAIN            
			// 训练
#ifdef OPENCV
			ptr_partsTrainer->reset(g_strProject, roiPath);
#else
			// 重新载入项目
			//ptr_partsTrainer->updateParam_train();
			//ptr_partsTrainer->dynamicTrainModeOpen();
			flag &= ptr_partsTrainer->HPDetection_start(g_strProject, roiPath);
			ptr_partsTrainer->setTrainTarget(1); // 设置单个物料训练超过3次才保存结果
#endif // OPENCV

#else
			// 检测
			/*if(strProject.find("project1"))
				strProject = "";*/
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

#endif // TRAIN

			//flag &= partsCounterHal->updateRoi(roiPath);
			//if (!flag) {
			flag = partsCounterHal->updateRoi(roiPath);
			//}
			if (!flag) {
				std::cout << "update roi failed! " << std::endl;
				cv::waitKey(0);
				return 0;
			}

			//cv::VideoCapture capture("H:/parts_counting_opencv_1015/parts_counting/x64/Debug/logimage/20161017/M001/a1/videotest.avi"); 


			run();


#ifdef TRAIN            
#ifdef OPENCV
			ptr_partsTrainer->writeTrainDataToXml();
			ptr_partsTrainer->cleanData();
#else
			ptr_partsTrainer->HPDetection_finish();
#endif // OPENCV
			
			cout << endl << endl << "train finish" << endl << endl;
			cv::waitKey(500);
#endif // TRAIN
		//}

	}

	}
	//while (1);
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
		sourceImg = cv::imread(filePath.c_str(), cv::IMREAD_GRAYSCALE);

		//判断图像是否为空
		if (sourceImg.empty())
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
		ptr_partsTrainer->training(&sourceImg, &trainResult, 200);   //OPENCV
		ptr_partsTrainer->saveData();
		cv::imshow("trainResult", trainResult);
		cv::waitKey(0);
#else

		bool flag = ptr_partsTrainer->HPDetection_train(sourceImg, trainResult);      //HALCON训练
		trainNum = ptr_partsTrainer->getTrainedObjNum();  // 获取已训练模型个数
		int objNum = ptr_partsTrainer->getDetectObjNum();  // 获取已训练模型个数
		out << "detected Obj num: " << objNum << ", trained Obj num: " << trainNum << ",   time: " << getTimeString() << std::endl;
		std::cout << "detected Obj num: " << objNum << ", trained Obj num: " << trainNum << ",   time: " << getTimeString() << std::endl;
		cv::imshow("trainResult", trainResult);
		int KEY = cv::waitKey(50);
		if (flag)
		{
			cv::waitKey(0);
		}
		if (KEY > 0)
		{// 如果触发按键，手动调用计数器
			ptr_partsTrainer->trainCountAdd();
		}
		trainNum = ptr_partsTrainer->getTrainedObjNum();  // 获取已训练模型个数
		if (trainNum != lastTrainNum)
		{
			cv::Mat trainImg = ptr_partsTrainer->getTrainedImage();
			string writePath = g_strProject + "/train_" + to_string(trainNum) + ".jpg";
			cv::imwrite(writePath.c_str(), trainImg);          // 保存训练图片 todo
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
			cv::waitKey(t_gap);
		}
#endif // OPENCV

		// 统计时间
		total_time += tTime;

		tObj = partsCounterHal->getTargetNumber();
		eObj = partsCounterHal->getErrorNumber();
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
			//cv::waitKey(0);
		}

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