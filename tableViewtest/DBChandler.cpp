#include "DBChandler.h"
#include "datacontainer.h"

unsigned int DBCHandler::selectedMessageCounter = 0;

DBCHandler::DBCHandler(QObject *parent)
    : QObject{parent}
{
    this->isAllInserted = false;
}

QString DBCHandler::errCode() const
{
    return m_errCode;
}

void DBCHandler::setErrCode(const QString &newErrCode)
{
    this->m_errCode= newErrCode;
    emit errCodeChanged();
    qInfo()<<"Error: "<<m_errCode;
}


QList<QList<QString>> DBCHandler::messagesList()
{
    if (isAllInserted){
        QList<QList<QString>> data;
        data.append({"Name","ID","DLC","Status"});
        foreach(dataContainer *const curValue , comInterface){
            data.append({curValue->getName(),curValue->getID(),QString::number(curValue->getDLC()),curValue->getIfSelected() ? "X" : "O" });
        }

        return data;
    }
}

QList<QList<QString> > DBCHandler::signalsList()
{
    if (isAllInserted){

        QList<QList<QString>> dataSignal;
        dataSignal.append({"Name","StartBit","Length","Resolution","Offset","MinValue","MaxValue","DataType"});
        for ( const dataContainer::signal *data : *comInterface.value(this->displayReqSignalID)->getSignalList()){
            dataSignal.append({data->name,QString::number(data->startBit),QString::number(data->length),QString::number(data->resolution),QString::number(data->offset),QString::number(data->minValue),QString::number(data->maxValue),data->appDataType});
        }
        qInfo()<<"Signal List turned signal"<<displayReqSignalID;
        return dataSignal;
    }
}

void DBCHandler::startToGenerate()
{
    if(!DBCHandler::selectedMessageCounter){
        this->setErrCode("En az bir mesaj seçmelisin");
    }else if(!isAllInserted){
        this->setErrCode("Veri tabanı okunamadı");
    }else
        emit this->procesStarted();
        //do here
}


void DBCHandler::update()
{
    isAllInserted = false;
    foreach(dataContainer * curValue , comInterface){
        delete curValue;
    }
    comInterface.clear();
        qInfo()<<"Updating";
        openFile();
}

void DBCHandler::clearData()
{
    isAllInserted = false;
    foreach(dataContainer * curValue , comInterface){
        delete curValue;
    }
    comInterface.clear();
    setErrCode("Table cleaned");
}

void DBCHandler::readFile(QString fileLocation)
{
    dbcPath = fileLocation;
    if(this->isAllInserted){
        this->setErrCode("DBC is already inserted");
        qInfo()<<"alreadyinserted";
    }else{
        openFile();
    }
}

void DBCHandler::openFile()
{
    try {
        if (dbcPath.isEmpty()){
            throw QString("Dosya konumu boş olamaz!");
        }else if(!dbcPath.contains(".dbc")){
            throw QString("Lütfen \".dbc\" uzantılı bir dosya seçin!");
        }
        else{
            QFile *ascFile = new QFile(dbcPath);
            if(!ascFile->open(QIODevice::ReadOnly | QIODevice::Text)){
                throw QString("Dosya açılamadı, lütfen konumu kontrol edin!");
            }
            else{
                    emit fileandLockOk();
                if (!parseMessages(ascFile)){
                    ascFile->close();
                    throw QString("Arayüzü okurken bir şeyler yanlış gitti!");
                }else
                    emit interfaceReady();
                ascFile->close();
            }

        }
    } catch (QString text) {
        this->setErrCode(text);
    }

}

const dataContainer *DBCHandler::getMessage(QString messageID)
{

    return comInterface.value(messageID);
}

void DBCHandler::setSelected(QString messageID)
{
    comInterface.value(messageID)->setSelected();
    qInfo()<<"Selection status changed to "<<QString::number(comInterface.value(messageID)->getIfSelected()) <<"for message ID:"<<displayReqSignalID;
    if(comInterface.value(messageID)->getIfSelected()){
        DBCHandler::selectedMessageCounter++;
    }else{
        DBCHandler::selectedMessageCounter--;
    }
    emit selectedStatChanged();
}




void DBCHandler::setDisplayReqSignal(QString signalID)
{
    this->displayReqSignalID = signalID;
    qInfo()<<"Signal set to display"<<displayReqSignalID;
    emit selectedViewChanged();
}

void DBCHandler::setFolderLoc(QString folderLoc)
{
    this->folderLoc = folderLoc;
}

void DBCHandler::setDutName(QString dutName)
{
    this->dutName = dutName;
    qInfo()<<"Dut name set to :"<<dutName;
}

bool DBCHandler::parseMessages(QFile *ascFile)
{
    QTextStream lines(ascFile);
    bool inlineOfMessage=false;
    bool inlineOfMessageOld=false;
    QString messageID;
    QString messageName ;
    unsigned short  messageDLC ;

    while (!lines.atEnd()) {
        QString curLine = lines.readLine();

        //Message parse - split message line to items and rise message block flasg (inlineOfMessage)
        if(curLine.contains("BO_ ") && curLine.indexOf("BO_") < 2){
            inlineOfMessage = true;

            QStringList messageList = curLine.split(" ");

            messageID = QString::number(messageList.at(1).toUInt(),16).toUpper();
            messageName= messageList.at(2);
            messageName.remove(QChar(':'),Qt::CaseInsensitive);
            messageDLC = messageList.at(3).toUShort();
            if (!comInterface.contains(messageID)){
                generateNewMessage(messageID,messageName,messageDLC);
            }
            // Release pointer locations

            //Signal parse - split signal lines to items
        }else if(inlineOfMessage && curLine.contains("SG_")){
            QStringList signalList = curLine.split(" ");
            dataContainer::signal curSignal;
            curSignal.name = signalList.at(2);
            curSignal.startBit = parseStartBit(signalList.at(4));
            curSignal.length = parseLength(signalList.at(4));
            curSignal.resolution = parseResolution(signalList.at(5));
            curSignal.offset = parseOffset(signalList.at(5));
            curSignal.minValue = parseMinValue(signalList.at(6));
            curSignal.maxValue = parseMaxValue(signalList.at(6));
            QString commentContainer = parseComment(curLine);
            curSignal.comment = commentContainer;
            curSignal.isJ1939 = commentContainer.contains(QString("j1939"),Qt::CaseInsensitive);
            addSignalToMessage (messageID,curSignal);

        }else{
            inlineOfMessage = false;
        }
        if (inlineOfMessageOld && !inlineOfMessage){
            comInterface.value(messageID)->setInserted();
        }
        inlineOfMessageOld = inlineOfMessage;
    }
    this->isAllInserted = true;

    return true;
}
//BO_ <ID> <Message_name>: <DLC> Vector__XXX -> for messages
//SG_ <Name> : <Bit order> | <Bit length>@1+ (<resolution>,<offset>) [<min_value>|max_value] "comment" -> for signals

bool DBCHandler::generateNewMessage(QString messageID, QString messageName , unsigned short messageDLC)
{
    dataContainer* newMessage = new dataContainer();
    newMessage->setName(messageName);
    newMessage->setmessageID(messageID);
    newMessage->setDLC(messageDLC);
    comInterface.insert(messageID,newMessage);
    return true;
}

bool DBCHandler::addSignalToMessage(QString messageID,dataContainer::signal curSignal)
{
    comInterface.value(messageID)->addSignal(curSignal);
    return true;
}

unsigned short DBCHandler::parseLength(QString splitedPart)
{
    splitedPart.remove("@1+");
    splitedPart.remove("@1-");
    QStringList container = splitedPart.trimmed().split("|");
    return container.at(1).toUShort();
}

unsigned short DBCHandler::parseStartBit(QString  splitedPart)
{
    splitedPart.remove("@1+");
    splitedPart.remove("@1-");
    QStringList container = splitedPart.trimmed().split("|");
    return container.at(0).toUShort();
}

double DBCHandler::parseResolution(QString  splitedPart)
{
    splitedPart.remove("(");
    splitedPart.remove(")");
    QStringList container = splitedPart.trimmed().split(",");
    return container.at(0).toDouble();
}

double DBCHandler::parseOffset(QString  splitedPart)
{
    splitedPart.remove("(");
    splitedPart.remove(")");
    QStringList container = splitedPart.trimmed().split(",");
    return container.at(1).toDouble();
}

double DBCHandler::parseMaxValue(QString  splitedPart)
{
    splitedPart.remove("[");
    splitedPart.remove("]");
    QStringList container = splitedPart.trimmed().split("|");
    return container.at(1).toDouble();
}

double DBCHandler::parseMinValue(QString  splitedPart)
{
    splitedPart.remove("[");
    splitedPart.remove("]");
    QStringList container = splitedPart.trimmed().split("|");
    return container.at(0).toDouble();
}

QString DBCHandler::parseComment(QString splitedPart)
{
    QString comment = splitedPart.mid(splitedPart.indexOf("]")+1,(splitedPart.indexOf("Vector__XXX")));
    comment.remove("\"");
    comment.remove("Vector__XXX");
    return comment.trimmed();
}


