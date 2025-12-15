#pragma once
#include "computervisionhal.h"
#include "partscounter.h"
#include "partstrainer.h"
#include <string>
#include <sstream>

using namespace std;


PartsCounterHal::PartsCounterHal()
	: d(new PartsCounter)
{
}

PartsCounterHal::~PartsCounterHal()
{
	delete d;
}

unsigned int PartsCounterHal::getTargetNumber()
{
	return d->getTargetNumber();
}

unsigned int PartsCounterHal::getErrorNumber()
{
	return d->getErrorNumber();
}

vector<int> PartsCounterHal::getMatchIDs()
{
	return d->getMatchIDs();
}

int PartsCounterHal::getNGEndingNumber()
{
	return d->getNGEndingNumber();
}

std::vector<ObjData> PartsCounterHal::getObjData()
{
	return d->getObjData();
}

pair<unsigned int, unsigned int> PartsCounterHal::getObjectNumber()
{
	return d->getObjectNumber();
}

void PartsCounterHal::action(cv::Mat* image, bool showImg)
{
	return d->action(image, showImg);
}

void PartsCounterHal::actionTransObject(double data[][3], int num)
{
	return d->actionTransObject(data, num);
}

void PartsCounterHal::actionHPDetectionObjectMat(cv::Mat* image, bool showImg)
{
	return d->actionHPDetectionObjectMat(image, showImg);
}

int PartsCounterHal::getErrorStatus()
{
	return d->getErrorStatus();
}

bool PartsCounterHal::getRetentionValue()
{
	return false;
}

bool PartsCounterHal::autoSetRoi(cv::Mat* image, int& roiLTX, int& roiLTY, int& roiWidth, int& roiHeight)
{
	return d->autoSetRoi(image, roiLTX, roiLTY, roiWidth, roiHeight);
}

std::string PartsCounterHal::getVersion()
{
	return d->getVersion();
}

bool PartsCounterHal::updateParam()
{
	return d->updateParam();
}

bool PartsCounterHal::updateRoi(std::string strPath)
{
	return d->updateRoi(strPath);
}

bool PartsCounterHal::updateProject(std::string strPath, int trainID)
{
	return d->updateProject(strPath, trainID);
}

bool PartsCounterHal::updateProjectHP(std::string strPath, int trainID)
{
	return d->updateProjectHP(strPath, trainID);
}

bool PartsCounterHal::updateDLMode(std::wstring strPath)
{
	return d->updateDLMode(strPath);
}

bool PartsCounterHal::setDLMode(int halconDLModel)
{
	return d->setDLMode(halconDLModel);
}

void PartsCounterHal::reset()
{
	return d->reset();
}

int PartsCounterHal::getStatus()
{
	return d->getStatus();
}

void PartsCounterHal::initialize()
{
	d->initialize();
}

void PartsCounterHal::setHardTriggerMode(bool flag)
{
	d->setHardTriggerMode(flag);
}

void PartsCounterHal::setNGEndingCountError(bool flag)
{
	d->setNGEndingCountError(flag);
}

void PartsCounterHal::setUseInsideDetection(bool flag)
{
	d->setUseInsideDetection(flag);
}

PartsTrainerHal::PartsTrainerHal()
{
	d = new PartsTrainer;
}

PartsTrainerHal::~PartsTrainerHal()
{
	delete d;
}

bool PartsTrainerHal::updateParam_train()
{
	return d->updateParam_train();
}

bool PartsTrainerHal::HPDetection_start(std::string strProject, std::string roiPath)
{
	return d->HPDetection_start(strProject, roiPath);
}

bool PartsTrainerHal::HPDetection_train(cv::Mat &image, cv::Mat &trainResult)
{
	return d->HPDetection_train(image, trainResult);
}

double PartsTrainerHal::getCalibFactor(cv::Mat &image)
{
	return d->getCalibFactor(image);
}

bool PartsTrainerHal::HPDetection_finish()
{
	return d->HPDetection_finish();
}

int PartsTrainerHal::getDetectObjNum()
{
	return d->getDetectObjNum();
}

int PartsTrainerHal::getTrainedObjNum()
{
	return d->getTrainedObjNum();
}

cv::Mat PartsTrainerHal::getTrainedImage()
{
	return d->getTrainedImage();
}

cv::Size PartsTrainerHal::getTrainedSize()
{
	return d->getTrainedSize();
}

bool PartsTrainerHal::training(cv::Mat *trainImg, cv::Mat *trainResult, int thresh)
{
	return d->training(trainImg, trainResult, thresh);
}

void PartsTrainerHal::dynamicTrainModeOpen()
{
	d->dynamicTrainModeOpen();
}

void PartsTrainerHal::dynamicTrainModeClose()
{
	d->dynamicTrainModeClose();
}

void PartsTrainerHal::setCountDebug()
{
	d->setCountDebug();
}

void PartsTrainerHal::trainCountAdd()
{
	d->trainCountAdd();
}

void PartsTrainerHal::setHalonMode(bool flag)
{
	d->setHalonMode(flag);
}

void PartsTrainerHal::setTrainTarget(int num)
{
	d->setTrainTarget(num);
}

void PartsTrainerHal::saveData()
{
	d->saveData();
}

void PartsTrainerHal::cleanData()
{
	d->cleanData();
}

bool PartsTrainerHal::writeImage(cv::Mat *img)
{
	return d->writeImage(img);
}

bool PartsTrainerHal::writeTrainDataToXml()
{
	return d->writeTrainDataToXml();
}

void PartsTrainerHal::reset(std::string strProjectPath, std::string strRoiPath)
{
	d->reset(strProjectPath, strRoiPath);
}

int PartsTrainerHal::dataCount()
{
	return d->dataCount();
}
