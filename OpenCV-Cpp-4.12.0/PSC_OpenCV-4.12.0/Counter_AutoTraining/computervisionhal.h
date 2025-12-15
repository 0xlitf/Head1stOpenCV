#ifndef COMPUTERVISIONHAL_H
#define COMPUTERVISIONHAL_H

#ifdef COMPUTER_VISION_EXPORTS
#define COMPUTERVISIONDLL_API __declspec(dllexport)
#else
#define COMPUTERVISIONDLL_API __declspec(dllimport)
#endif

#include "cfg.h"

namespace MyCV
{
	enum
	{
		stateNormal,
		sysParamError,
		roiError,
		projectError
	};
}

BOOL isFolderExist(const char* folder);
int32_t createDirectory(const char* directoryPath);
void getFiles(std::string path, vector<std::string>& files);
vector<std::string> fileFilter(vector<std::string>& Files, const std::string& keyWord);

namespace cv{ class Mat; }
class PartsCounter;
class COMPUTERVISIONDLL_API PartsCounterHal
{
public:
	PartsCounterHal();
	~PartsCounterHal();

	std::string getVersion();
	bool updateParam();                                                      //更新系统参数
	bool autoSetRoi(cv::Mat* image, int& roiLTX, int& roiLTY, int& roiWidth, int& roiHeight);					 //自动更新ROI    Edit by pmh 20170516
	bool updateRoi(std::string strPath);
	bool updateProject(std::string strPath, int trainID = 0);
	bool updateProjectHP(std::string strPath, int trainID = 0);
	bool updateDLMode(std::wstring strPath);
	bool setDLMode(int halconDLModel);
	void reset();
	unsigned int getTargetNumber();
	unsigned int getErrorNumber();
	std::vector<int> getMatchIDs();
	int getNGEndingNumber();				 //获取空降物料计数结果
	std::vector<ObjData> getObjData();
	std::pair<unsigned int, unsigned int> getObjectNumber();
	void action(cv::Mat* image, bool showImg = false);
	void actionTransObject(double data[][3], int num);
	void actionHPDetectionObjectMat(cv::Mat* image, bool showImg = false);
	int getErrorStatus();
	bool getRetentionValue();
	int getStatus();

	void initialize();

	void setHardTriggerMode(bool flag);                //打开硬触发模式，默认false不打开
	void setNGEndingCountError(bool flag = true);      //末端空降是否打杂开关. 默认true 打开
	void setUseInsideDetection(bool flag = true);      //设置内轮廓检测开关，默认false 不打开
private:
	PartsCounter *d;
};

class PartsTrainer;
class COMPUTERVISIONDLL_API PartsTrainerHal
{
public:
	PartsTrainerHal();
	~PartsTrainerHal();

	bool updateParam_train();                                                      //更新系统参数
	bool HPDetection_start(std::string strProject, std::string roiPath);
	bool HPDetection_train(cv::Mat &image, cv::Mat &trainResult);
	double getCalibFactor(cv::Mat &image);

	bool HPDetection_finish();
	int getDetectObjNum();
	int getTrainedObjNum();

	void reset(std::string strProjectPath, std::string strRoiPath);
	bool training(cv::Mat *trainImg, cv::Mat *trainResult, int thresh = 200);
	void saveData();
	void cleanData();
	int dataCount();
	void dynamicTrainModeOpen();
	void dynamicTrainModeClose();
	void setCountDebug();
	void trainCountAdd();
	void setHalonMode(bool flag);
	
	void setTrainTarget(int num);
	cv::Mat getTrainedImage();
	cv::Size getTrainedSize();
	
	bool writeImage(cv::Mat *img);
	bool writeTrainDataToXml();

private:
	PartsTrainer *d;
};

#endif
