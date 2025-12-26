#include "algorithmmanager.h"
#include "calgorithm.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QCoreApplication>

AlgorithmManager::AlgorithmManager(QObject *parent) : QObject(parent)
{
}

AlgorithmManager::~AlgorithmManager()
{
}

QString AlgorithmManager::getDllNameByType(AlgorithmManager::AlgorithmType algorithmType)
{
    QString dllName;
    switch (algorithmType) {
    case AlgorithmManager::AlgorithmType::none:
        return "";
    case AlgorithmManager::AlgorithmType::HalconTradition:
    case AlgorithmManager::AlgorithmType::HalconAI_DL_CLASSIFICATION:
    case AlgorithmManager::AlgorithmType::HalconAI_DL_OBJECT_DETECTION:
    case AlgorithmManager::AlgorithmType::HalconAI_DL_ANOMALY_DETECTION:
    case AlgorithmManager::AlgorithmType::HalconAI_DL_OBJECT_DETECTION_HOR:
#ifdef QT_DEBUG
        dllName = "algorithm_pscd.dll";
#else
        dllName = "algorithm_psc.dll";
#endif
        break;
    case AlgorithmManager::AlgorithmType::VisionMaster__4_3:
#ifdef QT_DEBUG
        dllName = "algorithm_vmd.dll";
#else
        dllName = "algorithm_vm.dll";
#endif
        break;
    }
    return dllName;
}

void AlgorithmManager::initialization()
{
    if (this->m_dllList.size() > 0)
    {
        return;
    }

    {// 判断是否有插加密狗
        QString dllName = getDllNameByType(AlgorithmManager::AlgorithmType::VisionMaster__4_3);
        QString name = "Senselock EliteIV v2.x";
        QProcess process;
        process.start(QString("wmic path Win32_PnPEntity where \"Name = '%1'\" get Name").arg(name));
        process.waitForFinished();
        QString result = process.readAllStandardOutput();
        int count = getStringCount(result,name);
        qDebug() << "加密狗数量[" << count << "]";
        if (count > 1){
            this->m_loadDllErrorString[dllName] += tr("加密狗数量过多");
        }
        if (count == 0){
            this->m_loadDllErrorString[dllName] += tr("加密狗未识别到");
        }
    }

    QString dllPath = tr("%1/dlls/algorithms").arg(QCoreApplication::applicationDirPath());

    QDir dir(dllPath);
    QStringList dllFileList = dir.entryList(QDir::Files);
    for (QString dllFile : dllFileList) {
        if (dllFile.endsWith(".dll")){
            if (this->m_loadDllErrorString[dllFile].isEmpty()){
                this->m_dllList.append(dllFile);
            }
        }
    }

    if (dllPath.size() == 0){
        return;
    }

    addEnvironmentPath(tr("%1").arg(dllPath));
    addEnvironmentPath(tr("%1_3rdParty").arg(dllPath));

    for (QString dll: this->m_dllList) {
        if (this->m_createFuncs[dll] != nullptr){
            continue;
        }
        QString dllFilePath = dllPath + "/" + dll;
        // 加载DLL
        QLibrary lib(dllFilePath);
        if (!lib.load()) {
            QString err = lib.errorString();
            qDebug() << "DLL加载失败:" << qUtf8Printable(err);
            this->m_loadDllErrorString[dll] += tr("DLL加载失败: %1").arg(qUtf8Printable(err));
        } else {
            qDebug() << "DLL加载成功:";
        }
        // 解析工厂函数
        this->m_createFuncs[dll] = reinterpret_cast<CreateAlgorithmFunc>(lib.resolve("createAlgorithm"));
        if (this->m_createFuncs[dll] == nullptr) {
            qDebug() << "函数解析失败";
            this->m_loadDllErrorString[dll] += tr("工厂函数解析失败");
            lib.unload();
        } else {
            qDebug() << "函数解析成功";
        }
    }
}

void AlgorithmManager::releaseInstance()
{
    for (auto it = m_pAlgorithms.begin(); it != m_pAlgorithms.end(); ++it) {
        if (it.value() != nullptr) {
            delete it.value();
            it.value() = nullptr;
        }
    }
    m_pAlgorithms.clear();

    this->m_dllList.clear();
    this->m_createFuncs.clear();
    this->m_algorithmsType.clear();
    this->m_algorithmsParams.clear();
}

bool AlgorithmManager::startAlgorithmByName(QString name, QString algorithmTypeName,QHash<QString, QVariant> algorithmPara)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<AlgorithmManager::AlgorithmType>();
    AlgorithmManager::AlgorithmType type = static_cast<AlgorithmManager::AlgorithmType>(metaEnum.keyToValue(algorithmTypeName.toStdString().c_str()));
    return AlgorithmManager::startAlgorithmByName(name,type,algorithmPara);
}

bool AlgorithmManager::startAlgorithmByName(QString name, AlgorithmManager::AlgorithmType algorithmType,QHash<QString, QVariant> algorithmPara)
{
    this->m_algorithmsType[name] = algorithmType;
    //根据算法类型使用对应的 dll 实例化对象
    QString useDll = getDllNameByType(algorithmType);
    if(useDll.isEmpty() || this->m_createFuncs[useDll] == nullptr)
    {
        return false;
    }

    if (m_pAlgorithms[name] != nullptr){
        delete m_pAlgorithms[name];
        m_pAlgorithms[name] = nullptr;
    }
    // 创建派生类实例并通过基类指针调用
    m_pAlgorithms[name] = this->m_createFuncs[useDll]();
    if(m_pAlgorithms[name] == nullptr)
    {
        qDebug() << "m_pAlgorithms[" << name << "] nullptr";
        return false;
    }
    connect(m_pAlgorithms[name], SIGNAL(signal_warn(QString, QString, QVariant)),this, SLOT(slot_warn(QString, QString, QVariant)));

    int nAlgorithmExecuteResult;

    /*PSC算法非必要
    algorithmPara.clear();
    m_pAlgorithms[name]->initAlgorithm(algorithmPara);
    algorithmPara.insert("algorithmType", "");
    algorithmPara.insert("algorithmVersion", "");
    m_pAlgorithms[name]->getAlgorithmPara(algorithmPara);
    qDebug() << algorithmPara["algorithmType"] << algorithmPara["algorithmVersion"];
    //*/

    //如果有传入算法的配置项，则调用设置的方法
    if (algorithmPara.keys().size() > 0)
    {
        setAlgorithmParams(name,algorithmPara);
    }

    //根据算法类型设置固定的参数配置
    this->m_algorithmsParams[name].insert("useHalcon", true);
    switch (this->m_algorithmsType[name]) {
    case AlgorithmManager::AlgorithmType::none:return false;
    case AlgorithmManager::AlgorithmType::HalconTradition:
        this->m_algorithmsParams[name].insert("DLModel", PSC_DL_MODE::DL_MODE_CLOSED);
        break;
    case AlgorithmManager::AlgorithmType::HalconAI_DL_CLASSIFICATION:
        this->m_algorithmsParams[name].insert("DLModel", PSC_DL_MODE::DL_CLASSIFICATION);
        break;
    case AlgorithmManager::AlgorithmType::HalconAI_DL_OBJECT_DETECTION:
        this->m_algorithmsParams[name].insert("DLModel", PSC_DL_MODE::DL_OBJECT_DETECTION);
        break;
    case AlgorithmManager::AlgorithmType::HalconAI_DL_ANOMALY_DETECTION:
        this->m_algorithmsParams[name].insert("DLModel", PSC_DL_MODE::DL_ANOMALY_DETECTION);
        break;
    case AlgorithmManager::AlgorithmType::HalconAI_DL_OBJECT_DETECTION_HOR:
        this->m_algorithmsParams[name].insert("DLModel", PSC_DL_MODE::DL_OBJECT_DETECTION_HOR);
        break;
    case AlgorithmManager::AlgorithmType::VisionMaster__4_3:
        this->m_algorithmsParams[name].insert("useHalcon", false);
        this->m_algorithmsParams[name].insert("ParseWay", VM_RESULT_PARSE::VM_RESULT_PARSE_SLAVE);
        break;
    }

    //开启算法
    nAlgorithmExecuteResult = m_pAlgorithms[name]->startAlgorithm(this->m_algorithmsParams[name]);
    if(nAlgorithmExecuteResult != AlgorithmExecuteResult_OK)
    {
        qDebug() << "[error] nAlgorithmExecuteResult:" << nAlgorithmExecuteResult << this->m_algorithmsParams[name];
        return false;
    }
    return true;
}

bool AlgorithmManager::stopAlgorithmByName(QString name)
{
    if (m_pAlgorithms[name] == nullptr) return false;
    /*PSC算法非必要
    hashPara.clear();
    m_pAlgorithms[name]->stopAlgorithm(hashPara);
    //*/
    return true;
}

QHash<QString, QVariant> AlgorithmManager::detection(QString name, cv::Mat cvSrcImage)
{
    QMutexLocker locker(&m_detectionMutex);

    if (m_pAlgorithms[name] == nullptr){
        return QHash<QString, QVariant>();
    }
    QString useDll = getDllNameByType(this->m_algorithmsType[name]);
    if(useDll.isEmpty() || this->m_createFuncs[useDll] == nullptr)
    {
        return QHash<QString, QVariant>();
    }
    cv::Mat cvResImage;
    QHash<QString, QVariant> hashPara;
    QHash<QString, QVariant> hashResult;
    QHash<QString, QVariant> vResultList;
    hashPara.insert("showContour", true);
    hashPara.insert("index", 1);
    if(m_pAlgorithms[name]->detection(hashPara, cvSrcImage, cvResImage, hashResult) != AlgorithmExecuteResult_OK)
    {
        return vResultList;
    }
    return hashResult;
}

QHash<QString, QVariant> AlgorithmManager::getAlgorithmParams(QString instanceName)
{
    return this->m_algorithmsParams[instanceName];
}

void AlgorithmManager::setAlgorithmParams(QString instanceName, QHash<QString, QVariant> AlgorithmParams)
{
    this->m_algorithmsParams[instanceName] = AlgorithmParams;
}

QStringList AlgorithmManager::getAlgorithmTypeOptionList()
{
    QStringList optionList;
    QMetaEnum metaEnumAlgorithmType = QMetaEnum::fromType<AlgorithmManager::AlgorithmType>();
    for (int j = 0; j < metaEnumAlgorithmType.keyCount(); ++j) {
        auto type = static_cast<AlgorithmType>(metaEnumAlgorithmType.value(j));
        if (type == AlgorithmType::none){
            continue;
        }
        //如果算法对应的dll加载成功
        QString dllFileName = getDllNameByType(type);
        if (!dllFileName.isEmpty() && this->m_dllList.contains(dllFileName) && this->m_loadDllErrorString[dllFileName].isEmpty()){
            optionList.append(metaEnumAlgorithmType.key(j));
        }
    }
    return optionList;
}

QStringList AlgorithmManager::getInitErrorAlgorithmTypeOptionList()
{
    QStringList optionList;
    QMetaEnum metaEnumAlgorithmType = QMetaEnum::fromType<AlgorithmManager::AlgorithmType>();
    for (int j = 0; j < metaEnumAlgorithmType.keyCount(); ++j) {
        auto type = static_cast<AlgorithmType>(metaEnumAlgorithmType.value(j));
        if (type == AlgorithmType::none){
            continue;
        }
        //如果算法对应的dll加载失败
        QString dllFileName = getDllNameByType(type);
        if (dllFileName.isEmpty() || !this->m_dllList.contains(dllFileName) || !this->m_loadDllErrorString[dllFileName].isEmpty()){
            optionList.append(metaEnumAlgorithmType.key(j));
        }
    }
    return optionList;
}

QStringList AlgorithmManager::getInitErrorMessages()
{
    QStringList optionList;
    for (QString key: this->m_loadDllErrorString.keys()) {
        if (!this->m_loadDllErrorString[key].isEmpty()){
            optionList.append(tr("dll文件【%1】加载失败:%2").arg(key).arg(this->m_loadDllErrorString[key]));
        }
    }
    return optionList;
}

bool AlgorithmManager::addEnvironmentPath(const QString& strPath)
{
    // 获取当前的PATH环境变量
    QByteArray variable = qgetenv("PATH");
    QList<QByteArray> paths = variable.split(';');

    // 将传入的路径转换为标准格式（处理斜杠和相对路径）
    QString canonicalPath = QDir(strPath).canonicalPath();
    if (canonicalPath.isEmpty()) {
        qDebug() << "Warning: The provided path is invalid:" << strPath;
        return false;
    }
    QByteArray newPathByte = canonicalPath.toUtf8();

    // 检查路径是否已在PATH中
    if (!paths.contains(newPathByte)) {
        // 若不在，则添加新路径
        paths.push_back(newPathByte);
        // 重新组合PATH变量
        QByteArray total;
        for (const QByteArray& path : paths) {
            total += path + ";";
        }
        // 设置新的PATH环境变量
        if (qputenv("PATH", total)) {
            qDebug() << "Successfully added to PATH:" << canonicalPath;
            return true;
        } else {
            qDebug() << "Failed to set new PATH environment variable.";
            return false;
        }
    }
    // 路径已存在，直接返回true
    qDebug() << "路径已存在，直接返回true:" << canonicalPath;
    return true;
}

void AlgorithmManager::slot_warn(QString strWarnType, QString strWarndata, QVariant vAttachment)
{
    qDebug() << "slot_warn" << strWarnType << strWarndata << vAttachment;
    switch (vAttachment.type())
    {//判断vAttachment类型后，根据类型分开处理
    case QVariant::Int://QVariant类型为int
        qDebug() << "vAttachment is int";
        break;
    case QVariant::Image://QVariant类型为QImage
        qDebug() << "vAttachment is QImage";
        break;
    default:
        qDebug() << "vAttachment is other type";
    }
}

