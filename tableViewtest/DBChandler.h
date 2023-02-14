#ifndef ASCHANDLER_H
#define ASCHANDLER_H

#include <QObject>
#include "datacontainer.h"
#include <QMap>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QDomDocument>
#include <QDateTime>
#include <random>
typedef QMap<QString,dataContainer*>  interface;
typedef QList<QList<QString>> fbType;
class DBCHandler : public QObject
{
    Q_OBJECT

public:
    static unsigned int selectedMessageCounter;
    explicit DBCHandler(QObject *parent = nullptr);



    Q_PROPERTY(QString errCode READ errCode WRITE setErrCode NOTIFY errCodeChanged)

    QString errCode() const;
    void setErrCode(const QString &newErrCode);
    const dataContainer *getMessage(QString messageID);

public slots:

    void update();
    void clearData();
    void readFile(QString fileLocation);
    void setSelected(QString messageID);
    void setDisplayReqSignal(QString signalID);
    void setFolderLoc(QString folderLoc);
    void setDutName(QString dutName);
    void setIOType(QString setIOType);
    //Tableview format nested vectors
    QList<QList<QString>> messagesList();
    QList<QList<QString>> signalsList();
    void startToGenerate();

signals:

    void interfaceReady();
    void selectedViewChanged();
    void selectedStatChanged();
    void procesStarted();
    void errCodeChanged();
    void fileandLockOk();

private:
    //Private Variables starts
    //***********************************
    interface comInterface;
    QString folderLoc;
    QString dbcPath;
    QString displayReqSignalID;
    QString dutName;
    QString dutHeader;
    QString IOType;
    bool isAllInserted;
    QString m_errCode;
    fbType fbNameandObjId;
    QString dutObjID;
    QString pouObjID;
    //***********************************
    //Private Variables ends
    //Private methods starts
    //***********************************

    //Private methods ends
    //***********************************
    // Reading Process from dbc file start
    //***********************************
    void openFile();
    bool parseMessages(QFile *ascFile);
    bool generateNewMessage(QString messageID, QString messageName , unsigned short messageDLC);
    bool createObjIds();
    QString getObjID();
    QString getrand(unsigned long long max);

    bool addSignalToMessage(QString messageID, dataContainer::signal curSignal);
    unsigned short parseLength(QString  splitedPart);
    unsigned short parseStartBit(QString  splitedPart);
    double parseResolution(QString  splitedPart);
    double parseOffset(QString  splitedPart);
    double parseMaxValue(QString  splitedPart);
    double parseMinValue(QString  splitedPart);
    QString parseComment(QString splitedPart);
    //***********************************
    // Reading Process from dbc file end
    //Generate XML file start
    //***********************************
    void extracted();
    bool createXml_STG1(QFile *xmlFile);
    void generateVariables(QDomElement*strucT,QDomDocument &doc);
    void generateIIPous(QDomElement*pous,QDomDocument &doc);
    void generateIOPous(QDomElement*pous,QDomDocument &doc);
    //***********************************
    //Generate XML file end

};

#endif // ASCHANDLER_H
