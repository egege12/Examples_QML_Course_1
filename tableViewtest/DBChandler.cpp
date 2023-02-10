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
    this->dutHeader = dutName.remove("_T");

    qInfo()<<"Dut name set to :"<<dutName;
}

void DBCHandler::setIOType(QString setIOType)
{
    this->IOType = setIOType;
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

bool DBCHandler::createObjIds()
{
    QString name,id;
    foreach(dataContainer *const curValue , comInterface){
        if(curValue->getIfSelected()){
        name = "_FB_"+this->dutHeader+"_"+curValue->getName()+"_0X"+curValue->getID();
        id = getObjID();
        this->fbNameandObjId.append({name,id});
        }
    }
    if(this->IOType == "IO"){
    name = "_FB_"+this->dutHeader+"_NA_Handler";
    this->fbNameandObjId.append({name,getObjID()});
    name = "_FB_"+this->dutHeader+"_Err_Handler";
    this->fbNameandObjId.append({name,getObjID()});
    name = "_FB_"+this->dutHeader+"_Validty_Handler";
    this->fbNameandObjId.append({name,getObjID()});
    }
    this->dutObjID = this->getObjID();
    this->pouObjID = this->getObjID();

    return true;
}

QString DBCHandler::getObjID()
{
    return QString((getrand(281474976710655))+"-"+getrand(65535)+"-"+getrand(65535)+"-"+getrand(281474976710655));
}

QString DBCHandler::getrand(unsigned long long max)
{
        unsigned long ms = static_cast<unsigned>(QDateTime::currentMSecsSinceEpoch());
        std::mt19937 gen(ms);
        std::uniform_int_distribution<> uid(0, max);
        bool ok;
        return QString::number((QString::number(uid(gen))).toULongLong(),16);

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

//XML Manpulation
void DBCHandler::startToGenerate()
{
    if(!DBCHandler::selectedMessageCounter){
        this->setErrCode("En az bir mesaj seçmelisin");
    }else if(!isAllInserted){
        this->setErrCode("Veri tabanı okunamadı");
    }else{
        emit this->procesStarted();
        try {
            if (this->folderLoc.isEmpty()){
                throw QString("Dosya konumu boş olamaz!");
            }
            else{
                QFile *xmlFile = new QFile(QString(this->folderLoc+"/"+(this->dutHeader)+"_Chain.xml"));
                if(!xmlFile->open(QIODevice::WriteOnly | QIODevice::Truncate)){
                    throw QString("Dosya açılamadı, lütfen konumu kontrol edin!");
                }
                else{
                    if (!createXml_STG1(xmlFile)){
                        throw QString("XML oluşturulurken bir şeyler yanlış gitti!");
                    }else
                        xmlFile->close();
                }

            }
        } catch (QString text) {
            this->setErrCode(text);
        }
    }
    //do here
}

bool DBCHandler::createXml_STG1(QFile *xmlFile)
{
    QTextStream out(xmlFile);
    QDomDocument doc;
    QDomText text;
    QDomElement element;
    QDomAttr attr;
    //Generate obj ids
    this->createObjIds();
    //<?xml version="1.0" encoding="utf-8"?>
    QDomProcessingInstruction instruction;
    instruction = doc.createProcessingInstruction( "xml", "version = \'1.0\' encoding=\'utf-8\'" );
    doc.appendChild( instruction );
    //****************************************************

    //<project xmlns="http://www.plcopen.org/xml/tc6_0200">
    QDomElement elemProject = doc.createElement( "project" );
    attr = doc.createAttribute( "xmlns" );//Add the text NAME in the USERINFO element
    attr.setValue("http://www.plcopen.org/xml/tc6_0200");//Add value for text NAME
    elemProject.setAttributeNode(attr);
    //Append child to document
    doc.appendChild(elemProject);
    //****************************************************
//START OF "fileHeader"
    //<fileHeader companyName="" productName="" productVersion="" creationDateTime="" />
    {
    QDomElement fileHeader = doc.createElement( "fileHeader" );
    attr =doc.createAttribute("companyName");
    attr.setValue("BZK");
    fileHeader.setAttributeNode(attr);
    attr =doc.createAttribute("productName");
    attr.setValue("CODESYS");
    fileHeader.setAttributeNode(attr);
    attr =doc.createAttribute("productVersion");
    attr.setValue("CODESYS V3.5 SP17");
    fileHeader.setAttributeNode(attr);
    attr =doc.createAttribute("creationDateTime");
    attr.setValue("2023-01-01T01:01:01.0000001");
    fileHeader.setAttributeNode(attr);
    //Append child to project
    elemProject.appendChild(fileHeader);
    }
    //****************************************************
//END OF "fileHeader"
//START OF "ContentHeader"
    //<contentHeader name="" modificationDateTime="">
    {
    QDomElement contentHeader = doc.createElement( "contentHeader" );
    attr =doc.createAttribute("name");
    attr.setValue("AUTOMATIC_INTERFACE_GENERATOR.project");
    contentHeader.setAttributeNode(attr);
    attr =doc.createAttribute("modificationDateTime");
    attr.setValue("2023-01-01T01:01:01.0000001");
    contentHeader.setAttributeNode(attr);
    //Append child to project
    elemProject.appendChild(contentHeader);
    //****************************************************
    //<coordinateInfo>
    QDomElement coordinateInfo = doc.createElement("coordinateInfo");
    contentHeader.appendChild(coordinateInfo);
    QDomElement scaling1 = doc.createElement("scaling"); // Make Child4 "scaling"
    attr =doc.createAttribute("y");
    attr.setValue("1");
    scaling1.setAttributeNode(attr);         //<scaling x="1"/>
    attr =doc.createAttribute("x");
    attr.setValue("1");
    scaling1.setAttributeNode(attr);         //<scaling Y="1"/>

    QDomElement fbd = doc.createElement("fbd");
    fbd.appendChild(scaling1);       //Append "scaling" to "fbd"
    coordinateInfo.appendChild(fbd);       //Append "fpd" to "coordinateInfo"
    scaling1 = doc.createElement("scaling"); // Make Child4 "scaling"
    attr =doc.createAttribute("y");
    attr.setValue("1");
    scaling1.setAttributeNode(attr);         //<scaling x="1"/>
    attr =doc.createAttribute("x");
    attr.setValue("1");
    scaling1.setAttributeNode(attr);
    QDomElement ld = doc.createElement("ld");      //Make Child3 "ld"
    ld.appendChild(scaling1);       //Append "scaling" to "ld"
    coordinateInfo.appendChild(ld);       //Append "ld" to "coordinateInfo"
    scaling1 = doc.createElement("scaling"); // Make Child4 "scaling"
    attr =doc.createAttribute("y");
    attr.setValue("1");
    scaling1.setAttributeNode(attr);         //<scaling x="1"/>
    attr =doc.createAttribute("x");
    attr.setValue("1");
    scaling1.setAttributeNode(attr);
    QDomElement sfc = doc.createElement("sfc");     //Make Child3 "sfc"
    sfc.appendChild(scaling1);       //Append "scaling" to "sfc"
    coordinateInfo.appendChild(sfc);       //Append "sfc" to "coordinateInfo"

    {
    //****************************************************
    //<addData>
    QDomElement appData = doc.createElement("appData");
    contentHeader.appendChild(appData);
    QDomElement data = doc.createElement("data");
    attr = doc.createAttribute("name");
    attr.setValue("http://www.3s-software.com/plcopenxml/projectinformation");
    data.setAttributeNode(attr);
    appData.appendChild(data);
    QDomElement ProjectInformation = doc.createElement("ProjectInformation");
    data.appendChild(ProjectInformation);
}
//END OF "ContentHeader"
}
{
//START OF "types"
    QDomElement types = doc.createElement("types");
    QDomElement pous = doc.createElement("pous");
    QDomElement dataTypes = doc.createElement("dataTypes");
    QDomElement dataType = doc.createElement("dataType");
    QDomElement baseType = doc.createElement("baseType");
    QDomElement strucT = doc.createElement("struct");
    attr = doc.createAttribute("name");
    attr.setValue(this->dutName);
    dataType.setAttributeNode(attr);
    this->generateDataTypes(&strucT);
    baseType.appendChild(strucT);
    dataType.appendChild(baseType);
    dataTypes.appendChild(dataType);
    types.appendChild(dataTypes);
    this->generatePous(&pous);
    types.appendChild(pous);
    elemProject.appendChild(types);

//END OF "types"
}
{
//START OF "instances"
    QDomElement instances = doc.createElement("instances");
    elemProject.appendChild(instances);
    QDomElement configurations= doc.createElement("configurations");
    instances.appendChild(configurations);
//END OF "instances"
}
{
//START OF "addData"
    QDomElement addData = doc.createElement("addData");
    elemProject.appendChild(addData);
    QDomElement data= doc.createElement("data");
    attr = doc.createAttribute("name");
    attr.setValue("http://www.3s-software.com/plcopenxml/projectstructure");
    data.setAttributeNode(attr);
    attr = doc.createAttribute("handleUnknown");
    attr.setValue("discard");
    data.setAttributeNode(attr);
    addData.appendChild(data);
    //fbs
    QDomElement ProjectStructure = doc.createElement("ProjectStructure");
    data.appendChild(ProjectStructure);
    QDomElement folder1 = doc.createElement("Folder");
    attr= doc.createAttribute("Name");
    attr.setValue("FD_Library_Application"); // Folder name for Function blocks
    folder1.setAttributeNode(attr);
    QDomElement folder2 = doc.createElement("Folder");
    attr= doc.createAttribute("Name");
    attr.setValue("_FB_"+this->dutHeader+"_MessageBlocks"); // Folder name for Function blocks
    folder2.setAttributeNode(attr);
    folder1.appendChild(folder2);

    for(QList<QString> const &data : fbNameandObjId ){
        QDomElement object = doc.createElement("Object");
        attr=doc.createAttribute("Name");
        attr.setValue(data.at(0));
        object.setAttributeNode(attr);
        attr=doc.createAttribute("ObjectId");
        attr.setValue(data.at(1));
        object.setAttributeNode(attr);
        folder2.appendChild(object);
    }
    //DUTs
    ProjectStructure.appendChild(folder1);
    folder1 = doc.createElement("Folder");
    attr= doc.createAttribute("Name");
    attr.setValue("FD_COMDUT"); // Folder name for Function blocks
    folder1.setAttributeNode(attr);
    folder2 = doc.createElement("Folder");
    attr = doc.createAttribute("Name");
    attr.setValue(this->dutName);
    folder2.setAttributeNode(attr);
    attr = doc.createAttribute("ObjectId");
    attr.setValue(this->dutObjID);
    folder2.setAttributeNode(attr);
    folder1.appendChild(folder2);
    ProjectStructure.appendChild(folder1);
    //Pous
    folder1 = doc.createElement("Folder");
    attr= doc.createAttribute("Name");
    attr.setValue("FD_POUs"); // Folder name for Function blocks
    folder1.setAttributeNode(attr);
    folder2 = doc.createElement("Folder");
    attr = doc.createAttribute("Name");
    attr.setValue("P_"+this->dutHeader);
    folder2.setAttributeNode(attr);
    attr = doc.createAttribute("ObjectId");
    attr.setValue(this->dutObjID);
    folder2.setAttributeNode(attr);
    folder1.appendChild(folder2);
    ProjectStructure.appendChild(folder1);

//END OF "addData"
}

     doc.save(out, 4);
     return true;
}

void DBCHandler::generateVariables(QDomElement * strucT)
{
    QDomAttr attr;
}

void DBCHandler::generatePous(QDomElement * pous)
{
    QDomAttr attr;
}
