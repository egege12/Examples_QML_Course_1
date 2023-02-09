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
typedef QMap<QString,dataContainer*>  interface;

class DBCHandler : public QObject
{
    Q_OBJECT

public:
    static unsigned int selectedMessageCounter;
    explicit DBCHandler(QObject *parent = nullptr);
    QString dbcPath;
    QString displayReqSignalID;


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
    //Private Variables
    //***********************************
    interface comInterface;
    QString folderLoc;
    QString dutName;
    bool isAllInserted;
    QString m_errCode;

    //Private methods
    //***********************************


    // Reading Process from dbc file
    //***********************************
    void openFile();
    bool parseMessages(QFile *ascFile);
    bool generateNewMessage(QString messageID, QString messageName , unsigned short messageDLC);
    bool addSignalToMessage(QString messageID, dataContainer::signal curSignal);
    unsigned short parseLength(QString  splitedPart);
    unsigned short parseStartBit(QString  splitedPart);
    double parseResolution(QString  splitedPart);
    double parseOffset(QString  splitedPart);
    double parseMaxValue(QString  splitedPart);
    double parseMinValue(QString  splitedPart);
    QString parseComment(QString splitedPart);
    // Reading Process from asc file


};

#endif // ASCHANDLER_H
