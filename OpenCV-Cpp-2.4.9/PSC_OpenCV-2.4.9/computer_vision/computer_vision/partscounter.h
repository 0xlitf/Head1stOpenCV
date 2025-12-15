/*****************************************************************************
name: couter.h
date:  2016.11.19(start)
author: peng mei hua
modification date: 2016.12.19
function:  new counting logic
******************************************************************************/
#ifndef PARTSCOUNTER_H
#define PARTSCOUNTER_H

#include "hp_detection_partscounter.h"
#include "cfg.h"


#define  COMP_METHOD_MATHCONTOURS  0
#define  COMP_METHOD_MATHAREA1  1
#define  COMP_METHOD_MATHAREA2  2

//struct ObjectData
//{
//	cv::Mat            objRoiImage; //物料区域图像
//	Rect               objRoi;      //物料水平外包围矩形框坐标
//	int                objArea;     //物料面积
//	unsigned int       isOK_Flag;   //物料识别结果  0-合格，1-杂质，2-其他（不作判断）
//};

class PartsCounter
{
	public:
		PartsCounter();
		~PartsCounter();

		std::string getVersion();
		bool autoSetRoi(cv::Mat* image, int& roiLTX, int& roiLTY, int& roiWidth, int& roiHeight);					 //自动更新ROI    Edit by pmh 20170516
		bool updateParam();                                                      //更新系统参数
		bool updateRoi(std::string strPath);									 //更新ROI参数
		bool updateProject(std::string strPath, int trainID = 0);								 //更新工程项目参数
		bool updateProjectHP(std::string strPath, int trainID = 0);                               //更新HP工程项目参数
		bool updateDLMode(std::wstring strPath);                                  //更新深度学习模型
		bool setDLMode(int halconDLModel);                                       //设置深度学习模型任务类别
		void reset();                                                            //清空工程项目参数
		unsigned int getTargetNumber() { return m_targetNumber; }				 //获取正常料计数结果
		unsigned int getErrorNumber()  { return m_errorNumber; }				 //获取杂料计数结果
		std::vector<int> getMatchIDs()  { return m_modelIDs; }				 //获取匹配结果
		int getNGEndingNumber()  { return m_NGEndingNum; }				 //获取空降物料计数结果
		void drawObjData(cv::Mat* image, bool showImg);                            //绘制物料信息
		std::vector<ObjData> getObjData() { return m_objData; }                 //获取物料信息
		 
		std::pair<unsigned int, unsigned int> getObjectNumber()  { return make_pair(m_obj_num, m_err_num); }				 //获取当前帧检测到的合格物料和杂料个数（无论是否合格）
		void action(cv::Mat* image, bool showImg = false);     // 主函数
		void actionTransObject(double data[][3], int num);              //透明料
		void actionHPDetectionObjectMat(cv::Mat* image, bool showImg = false);      //halcon精细缺陷检测
		int getErrorStatus(){ return m_errorStatus; }                            //获取错误状态
		int getStatus();														 //获取读取系统参数文件 结果
		void initialize();                                                       //初始化参数
		void setNGEndingCountError(bool flag = true);                // 末端NG物料是否打杂料
		void setUseInsideDetection(bool flag = true);                //设置内轮廓检测开关，默认false 不打开
		void setHardTriggerMode(bool flag = true);                   //打开硬触发模式，默认false不打开
		//初始化参数
		

	private:
		void segmentObj(cv::Mat* image, bool showImg); //目标物料检测识别函数
		void estimateObj();							   //判断是否为合格料函数
		void hPDetectionEstimateObj();                 //halcon精细缺陷检测，评估物料
		void countObj();                               //计数逻辑函数
		int HungarianMatch();                         //匈牙利匹配算法
		int path(int u);
		
		int m_threshValue;       //阈值分割参数
		int m_errorStatus;       //错误标志
		int m_width;             //图像宽度  默认320
		int m_height;             //图像宽度  默认240
		int m_allowContourArea0;            //允许识别物料的最大面积
		int m_allowContourArea1;            //允许识别物料的最小面积
		double m_Lcoefficient;              //最小公差系数  %
		double m_Hcoefficient;              //最大公差系数  %
		double m_AdiffValue;                //面积公差范围  pix*pix
		int m_WdiffValue;                   //宽度公差范围  pix
		int m_HdiffValue;                   //高度公差范围  pix
		int m_detectLineC;                  //跟踪线位置    pix
		double m_MinScore;                  //最小匹配分数
		int m_MinDefectInnerRadian;         //最小缺陷内圆半径
		int m_MinDefectArea;                //最小缺陷面积
		
		
		cv::Mat m_image, m_RoiImage;                    //当前帧处理图像, 以及提取ROI后的图像
		cv::Mat m_imageBG;                  //背景图像
		std::vector<ObjData> m_objData;               //物料ROI数据
		
		std::vector<unsigned int>  m_halcon_flag;      //halcon精细缺陷检测物料OK，NG标识
		std::vector<double> m_objArea;                 //目标面积
		std::vector<double> m_lastObjArea;             //上一帧目标面积
		std::vector<float> m_objHeight;                //目标高度
		std::vector<float> m_objWidth;                 //目标宽度
		std::vector<float> m_lastObjWidth;             //上一帧中目标宽度
		std::vector<int> m_modelIDs;                                 //目标物料序号,0为未匹配上
		std::vector<cv::Point> m_center;               //目标中心坐标
		std::vector<cv::Point> m_preCenter;            //上一帧中目标中心坐标
		std::vector<int> m_trainArea;                  //训练模板面积
		std::vector<int> m_trainHeight;                //训练模板高度
		std::vector<int> m_trainWidth;                 //训练模板宽度
		std::vector<std::vector<cv::Point>> m_trainContours;                 //训练模板轮廓
		std::vector<int> m_trainID;                    //训练目标序号
		unsigned int m_targetNumber;                   //检测到正常料计数个数
		unsigned int m_errorNumber;                    //检测到杂料计数个数
		unsigned int m_num;                            //当前帧检测到的物料个数之和（无论是否合格）
		unsigned int m_obj_num;                            //当前帧检测到的合格物料个数
		unsigned int m_err_num;                            //当前帧检测到的杂料物料个数
		int m_NGEndingNum = 0;                         // 当前帧检测到的末尾空降杂料数量

		bool m_useInsideDetection = false;    // 是否使用内轮廓检测， 默认false-不使用
		bool m_isNGEndingCountError = false;		// 末端NG是否打杂料， 默认true-使用
		bool m_useHardTriggerMode = false;

		int m_openElementSize;                         //开运算模板大小
		std::vector<unsigned int> m_Flag;              //物料识别标志  0-合格，1-杂质，2-其他（不作判断）
		std::vector<unsigned int> m_preFlag;           //上一帧的物料识别标志
		std::vector<int> m_tempWidth;                  //目标物料的水平宽度
		std::vector<int> m_lastTempWidth;              //上一帧中目标物料的水平宽度
		std::vector<std::vector<cv::Point> > m_contours;            //检测到目标轮廓
		int m_contoursNum, m_lastcontoursNum, m_allcontoursNum, m_lastallcontoursNum;                 //当前帧目标轮廓数， 上一帧目标轮廓数
		std::vector<std::vector<cv::Point> > m_objContour;          //合格料轮廓
		std::vector<std::vector<cv::Point> > m_impurityContour;     //杂料轮廓
		RoiData m_roi;                                    //ROI参数
		double m_roiLineLimitC;                        //ROI中计数有效的区域所占比例·
		double m_similarity;                           //相似度
		int m_matchMethod;                             //与训练模型比较的方法
		std::vector<std::vector<cv::Point> > m_lastObjContours;                         //上一帧的正常料轮廓
		std::vector<std::vector<cv::Point> > m_lastImpurityContours;                    //上一帧的杂料轮廓
		int m_preMinX;                                 //上一帧最左端物料的X坐标
		int m_currentMinX;                             //当前帧最左端物料的X坐标
		int m_preMaxX;                                 //上一帧最右端物料的X坐标
		int m_currentMaxX;                             //当前帧最右端物料的X坐标
		int m_xCompensate;                             //物料允许回退范围
		int m_yRange;                                  //y轴搜索范围
		int m_xRange;                                  //物料x轴移动范围
		int is_merged;                                //是否有物料合并
		int merge_match;                             //合并前对应物料下标
		int is_departed;                              //是否有物料分离
		int departed_match;                           //分离后对应物料下标
		int NG3_OK_flag;                          //合并料转成了合格料
		bool m_sysparamState;                          //系统参数文件读取状态
		bool m_roiState;                               //ROI参数文件读取状态
		bool m_projectState;                           //训练模板参数文件读取状态

		std::shared_ptr<HPDetectionPartsCounter> ptr_HPDetectionPartsCounter;

		#define MAXN 20                  //MAXN表示X集合和Y集合顶点个数的最大值
		int nx, ny;                                       //x和y集合中顶点的个数
		int g[MAXN][MAXN];                //邻接矩阵，g[i][j]为1表示有连接
		int cx[MAXN], cy[MAXN];        //cx[i],表示最终求得的最大匹配中，与x集合中元素Xi匹配的集合Y中顶点的索引
										//cy[i],表示最终求得的最大匹配中，与y集合中元素Yi匹配的集合X中顶点的索引
		int mk[MAXN];            //DFS算法中记录顶点访问状态的数据mk[i]=0表示未访问过，为1表示访问过
};


#endif PARTSCOUNTER_H
