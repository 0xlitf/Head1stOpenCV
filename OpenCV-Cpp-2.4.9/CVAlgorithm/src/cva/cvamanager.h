#ifndef ALGORITHMMANAGER_H
#define ALGORITHMMANAGER_H

#include <QObject>
#include <QVariant>
#include <QHash>
#include <QString>
#include "algorithm_global.h"

//枚举值转为key字符串
#ifndef EnumValToKeyQString
#define EnumValToKeyQString(_enum_,_enumVal_) QMetaEnum::fromType<_enum_>().valueToKey(_enum_::_enumVal_)
#endif

namespace cv {
class Mat;
}
class CAlgorithm;
typedef CAlgorithm* (*CreateAlgorithmFunc)();

class ALGORITHMSHARED_EXPORT AlgorithmManager : public QObject
{
    Q_OBJECT
public:
    explicit AlgorithmManager(QObject *parent = nullptr);
    ~AlgorithmManager();

    //算法类型可选项
    enum AlgorithmType{
        none = 0,							//空
        HalconTradition,					//halcon 传统
        HalconAI_DL_CLASSIFICATION,			//halcon AI 分类检测模型
        HalconAI_DL_OBJECT_DETECTION,       //目标检测模型
        HalconAI_DL_ANOMALY_DETECTION,      //异常检测模型
        HalconAI_DL_OBJECT_DETECTION_HOR,   //目标检测模型-水平框
        VisionMaster__4_3,					//海康4.3
    };
    Q_ENUM(AlgorithmType)

    /** 初始化
     * @brief initialization
     * @return
     */
    void initialization();
    /** 释放资源
     * @brief releaseInstance
     * @return
     */
    void releaseInstance();
    /** 获取某个算法的所有参数配置
     * @brief getAlgorithmParams
     * @param instanceName 实例唯一名称
     * @return
     */
    QHash<QString, QVariant> getAlgorithmParams(QString instanceName);
    /** 设置某个算法的所有参数配置
     * @brief setAlgorithmParams
     * @param instanceName 实例唯一名称
     * @param AlgorithmParams 参数配置
     */
    void setAlgorithmParams(QString instanceName, QHash<QString, QVariant> AlgorithmParams);
    /** 获取所有正常加载的算法类型可选列表
     * @brief getAlgorithmTypeOptionList
     * @return
     */
    QStringList getAlgorithmTypeOptionList();
    /** 获取所有异常加载的算法类型可选列表
     * @brief getInitErrorAlgorithmTypeOptionList
     * @return
     */
    QStringList getInitErrorAlgorithmTypeOptionList();
    /** 获取初始化异常消息
     * @brief getInitErrorMessages
     * @return
     */
    QStringList getInitErrorMessages();
    /** 根据名称开启算法
     * @brief startAlgorithmByName
     * @param QString instanceName 实例唯一名称
     * @param QString algorithmTypeString 类型字符 / AlgorithmType algorithmType 类型枚举
     * @param QHash<QString, QVariant> AlgorithmParams 算法库配置参数
     * @return 是否开启成功
     */
    bool startAlgorithmByName(QString instanceName,QString algorithmTypeString,QHash<QString, QVariant> AlgorithmParams = QHash<QString,QVariant>());
    bool startAlgorithmByName(QString instanceName,AlgorithmType algorithmType,QHash<QString, QVariant> AlgorithmParams = QHash<QString,QVariant>());
    /** 根据名称停止算法
     * @brief stopAlgorithmByName
     * @param QString instanceName 实例唯一名称
     * @return 是否停止成功
     */
    bool stopAlgorithmByName(QString instanceName);
    /** 检测图片
     * @brief detection
     * @param QString instanceName 实例唯一名称
     * @param cv::Mat detectionImage 要检测的图片
     * @return 检测结果 TODO 待确定统一的检测结果数据结构
     */
    QHash<QString, QVariant> detection(QString instanceName,cv::Mat);
signals:

private slots:
    void slot_warn(QString strWarnType, QString strWarndata, QVariant vAttachment);

private:
    QMutex m_detectionMutex;

    QList<QString> m_dllList;										//dll文件列表
    QHash<QString,QString> m_loadDllErrorString;					//dll文件加载失败的字符串消息
    QHash<QString,CreateAlgorithmFunc> m_createFuncs;				//创建算法实例的方法指针
    QHash<QString,CAlgorithm*> m_pAlgorithms;						//算法实例指针
    QHash<QString,AlgorithmType> m_algorithmsType;					//算法实例类型
    QHash<QString,QHash<QString, QVariant>> m_algorithmsParams;		//算法实例的传入配置参数

    //新增某个路径到临时环境变量
    bool addEnvironmentPath(const QString& strPath);
    //获取算法类型对应的 dll 文件名称
    QString getDllNameByType(AlgorithmManager::AlgorithmType);
    //查找字符串是否存在并获取出现次数
    int getStringCount(const QString& mainStr, const QString& subStr)
    {
        int count = 0;
        int pos = 0;
        while ((pos = mainStr.indexOf(subStr, pos)) != -1) {
            ++count;
            pos += subStr.length();
        }
        return count;
    }
};

#endif // ALGORITHMMANAGER_H
