#include "DBChandler.h"
#include "datacontainer.h"
#include "qforeach.h"
#include <QtGlobal>
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
        data.append({"Name","ID(HEX)","DLC","CycleTime[ms]","Timeout[ms]","Status"});
        foreach(dataContainer *const curValue , comInterface){
            data.append({curValue->getName(),curValue->getID(),QString::number(curValue->getDLC()),curValue->getMsCycleTime(),curValue->getMsTimeOut(),curValue->getIfSelected() ? "X" : "O" });
        }
        return data;
    }
}

QList<QList<QString> > DBCHandler::signalsList()
{
    if (isAllInserted){

        QList<QList<QString>> dataSignal;
        dataSignal.append({"Name","StartBit","Length","Resolution","Offset","MinValue","MaxValue","App. DataType","Com. DataType","Comment"});
        for ( const dataContainer::signal *data : *comInterface.value(this->displayReqSignalID)->getSignalList()){
            dataSignal.append({data->name,QString::number(data->startBit),QString::number(data->length),QString::number(data->resolution),QString::number(data->offset),QString::number(data->minValue),QString::number(data->maxValue),data->appDataType,data->comDataType,data->comment});
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
    setErrCode("Tablo temizlendi");
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
    QList<QList<QString>> msgCommentList;

    while (!lines.atEnd()) {
        QString curLine = lines.readLine();

        /*Message parse - split message line to items and rise message block flasg (inlineOfMessage)*/
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
            /*Signal parse - split signal lines to items*/
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
            /*Defualt value*/
            if(commentContainer.contains(QString("defValue="),Qt::CaseInsensitive)){
                curSignal.defValue=commentContainer.mid(commentContainer.indexOf("defValue=")+9,10).trimmed().toDouble();
            }else{
                curSignal.defValue=0.0;
            }
            addSignalToMessage (messageID,curSignal);
            /*Append message comments*/
        }else if((curLine.contains("CM_"))&& curLine.contains("BO_")){

            QStringList messageList = curLine.split(" ");
            QString commentContainer =  curLine.mid(curLine.indexOf(messageList.at(2)),curLine.length());
            QString ID = QString::number(messageList.at(2).toUInt(),16).toUpper();
            QString msTimeout = this->getBetween("timeout","ms",commentContainer);
            QString msCycleTime = this->getBetween("cycletime","ms",commentContainer);

            msgCommentList.append({ID,msTimeout,msCycleTime,commentContainer});

        }else{
            inlineOfMessage = false;
        }
        if (inlineOfMessageOld && !inlineOfMessage){
            comInterface.value(messageID)->setInserted();
        }
        inlineOfMessageOld = inlineOfMessage;
    }
    /*Assing the message comments on the list*/
    foreach(dataContainer *const curValue , comInterface){
        foreach (QList<QString> contMessage, msgCommentList){
            if(contMessage.at(0)==curValue->getID()){
                if(!contMessage.at(1).isEmpty()){
                    curValue->setMsTimeOut(contMessage.at(1));
                }
                if(!contMessage.at(2).isEmpty()){
                    curValue->setMsCycleTime(contMessage.at(2));
                }
                curValue->setComment(contMessage.at(3));
            }
        }
    }
    this->isAllInserted = true;
    const QList<QString> *data = dataContainer::getWarningList();
    foreach(QString messageWarning , *data){
        qInfo()<<messageWarning;
    }
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
    unsigned long long ms = static_cast<unsigned long long>(QDateTime::currentMSecsSinceEpoch());
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
    QString comment = splitedPart.mid(splitedPart.indexOf("]")+1,((splitedPart.indexOf("Vector__XXX")))-(splitedPart.indexOf("]")+1));
    comment.remove("\"");
    comment.remove("Vector__XXX");
    return comment.trimmed();
}

QString DBCHandler::getBetween(QString first, QString second, QString fullText)
{
    if(fullText.contains(first,Qt::CaseInsensitive)){
        qsizetype indexCont1 = fullText.indexOf(first,Qt::CaseInsensitive);
        fullText= fullText.sliced(indexCont1);
        indexCont1 = fullText.indexOf(first,Qt::CaseInsensitive);
        if(fullText.contains(QString(second),Qt::CaseInsensitive)){
            qsizetype indexCont2 = fullText.indexOf(second,Qt::CaseInsensitive);
            return fullText.mid(indexCont1,(indexCont2-indexCont1)).trimmed().remove("=").remove(":").remove(first).trimmed();
        }else
            return "(*ERROR*)";
    }return "(*ERROR*)";
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
            QDomElement addData = doc.createElement("addData");
            contentHeader.appendChild(addData);
            QDomElement data = doc.createElement("data");
            attr = doc.createAttribute("name");
            attr.setValue("http://www.3s-software.com/plcopenxml/projectinformation");
            data.setAttributeNode(attr);
            attr = doc.createAttribute("handleUnknown");
            attr.setValue("implementation");
            data.setAttributeNode(attr);
            addData.appendChild(data);
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
        QDomElement addData = doc.createElement("addData");
        QDomElement data = doc.createElement("data");
        QDomElement ObjectId = doc.createElement("ObjectId");
        attr = doc.createAttribute("name");
        attr.setValue(this->dutName);
        dataType.setAttributeNode(attr);
        this->generateVariables(&strucT,doc);
        baseType.appendChild(strucT);
        dataType.appendChild(baseType);
        attr = doc.createAttribute("name");
        attr.setValue("http://www.3s-software.com/plcopenxml/objectid");
        data.setAttributeNode(attr);
        attr = doc.createAttribute("handleUnknown");
        attr.setValue("discard");
        data.setAttributeNode(attr);
        text=doc.createTextNode(dutObjID);
        ObjectId.appendChild(text);
        data.appendChild(ObjectId);
        addData.appendChild(data);
        dataType.appendChild(addData);
        dataTypes.appendChild(dataType);
        types.appendChild(dataTypes);
        if((this->IOType == "II"))
            this->generateIIPous(&pous,doc);
        else
            this->generateIOPous(&pous,doc);
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

void DBCHandler::generateVariables(QDomElement * strucT, QDomDocument &doc)
{
    QDomAttr attr;
    QDomText text;

    foreach(dataContainer *const curValue , comInterface){
        if(curValue->getIfSelected()){
            for(const dataContainer::signal * curSignal : *curValue->getSignalList()){
                QDomElement variable = doc.createElement("variable");
                attr = doc.createAttribute("name");
                attr.setValue(curSignal->name);
                variable.setAttributeNode(attr);
                { //type and derived element with name attribut
                    QDomElement type = doc.createElement("type");
                    QDomElement derived = doc.createElement("derived");
                    attr = doc.createAttribute("name");
                    attr.setValue(curSignal->appDataType+"_T");
                    derived.setAttributeNode(attr);
                    type.appendChild(derived);
                    variable.appendChild(type);
                }
                {//initialValue
                    QDomElement initialValue = doc.createElement("initialValue");
                    QDomElement structValue = doc.createElement("structValue");
                    if((curSignal->appDataType)!="BOOL"){
                        QDomElement value = doc.createElement("value");
                        attr = doc.createAttribute("member");
                        attr.setValue("Param_Max");
                        value.setAttributeNode(attr);
                        QDomElement simpleValue = doc.createElement("simpleValue");
                        attr = doc.createAttribute("value");
                        attr.setValue(QString::number(curSignal->maxValue));
                        simpleValue.setAttributeNode(attr);
                        value.appendChild(simpleValue);
                        structValue.appendChild(value);
                    }
                    if((curSignal->appDataType)!="BOOL"){
                        QDomElement value = doc.createElement("value");
                        attr = doc.createAttribute("member");
                        attr.setValue("Param_Min");
                        value.setAttributeNode(attr);
                        QDomElement simpleValue = doc.createElement("simpleValue");
                        attr = doc.createAttribute("value");
                        attr.setValue(QString::number(curSignal->minValue));
                        simpleValue.setAttributeNode(attr);
                        value.appendChild(simpleValue);
                        structValue.appendChild(value);
                    }
                    if(((curSignal->appDataType)=="REAL")||((curSignal->appDataType)=="LREAL")){
                        QDomElement value = doc.createElement("value");
                        attr = doc.createAttribute("member");
                        attr.setValue("Param_Res");
                        value.setAttributeNode(attr);
                        QDomElement simpleValue = doc.createElement("simpleValue");
                        attr = doc.createAttribute("value");
                        attr.setValue(QString::number(curSignal->resolution));
                        simpleValue.setAttributeNode(attr);
                        value.appendChild(simpleValue);
                        structValue.appendChild(value);
                    }
                    if(((curSignal->appDataType)=="REAL")||((curSignal->appDataType)=="LREAL")){
                        QDomElement value = doc.createElement("value");
                        attr = doc.createAttribute("member");
                        attr.setValue("Param_Off");
                        value.setAttributeNode(attr);
                        QDomElement simpleValue = doc.createElement("simpleValue");
                        attr = doc.createAttribute("value");
                        attr.setValue(QString::number(curSignal->offset));
                        simpleValue.setAttributeNode(attr);
                        value.appendChild(simpleValue);
                        structValue.appendChild(value);
                    }
                    if((curSignal->appDataType)!="BOOL"){
                        QDomElement value = doc.createElement("value");
                        attr = doc.createAttribute("member");
                        attr.setValue("Param_Def");
                        value.setAttributeNode(attr);
                        QDomElement simpleValue = doc.createElement("simpleValue");
                        attr = doc.createAttribute("value");
                        attr.setValue(QString::number(curSignal->defValue));
                        simpleValue.setAttributeNode(attr);
                        value.appendChild(simpleValue);
                        structValue.appendChild(value);
                    }
                    {
                        QDomElement value = doc.createElement("value");
                        attr = doc.createAttribute("member");
                        attr.setValue("x");
                        value.setAttributeNode(attr);
                        QDomElement simpleValue = doc.createElement("simpleValue");
                        attr = doc.createAttribute("value");
                        attr.setValue(QString::number(curSignal->defValue));
                        simpleValue.setAttributeNode(attr);
                        value.appendChild(simpleValue);
                        structValue.appendChild(value);
                    }
                    initialValue.appendChild(structValue);
                    variable.appendChild(initialValue);
                }
                QDomElement documentation=doc.createElement("documentation");
                QDomElement xhtml = doc.createElement("xhtml");
                attr=doc.createAttribute("xmlns");
                attr.setValue("http://www.w3.org/1999/xhtml");
                xhtml.setAttributeNode(attr);
                text =doc.createTextNode(curSignal->comment+"/StartBit:"+QString::number(curSignal->startBit)+"/Length:"+QString::number(curSignal->length)+"/J1939:"+((curSignal->isJ1939)?"YES":"NO"));
                xhtml.appendChild(text);
                documentation.appendChild(xhtml);
                variable.appendChild(documentation);
                strucT->appendChild(variable);
            }
        }
    }
}

void DBCHandler::generateIIPous(QDomElement * pous, QDomDocument &doc)
{
    QDomAttr attr;
    QDomText text;

    foreach (dataContainer * curMessage , comInterface){
        if(curMessage->getIfSelected()){
            QDomElement pou = doc.createElement("pou");
            /*set pou name*/
            attr=doc.createAttribute("name");
            attr.setValue("_FB_"+this->dutHeader+"_"+curMessage->getName()+"_0X"+curMessage->getID());
            pou.setAttributeNode(attr);
            /*set pouType*/
            attr=doc.createAttribute("pouType");
            attr.setValue("functionBlock");
            pou.setAttributeNode(attr);
            /*Interface*/
            QDomElement interface = doc.createElement("interface");
            {
                QDomElement inputVars = doc.createElement("inputVars");

                /*Generate Input Variables - inputVars*/
                /*C_Init_Can*/
                {
                    QDomElement variable = doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("C_Init_Can");
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement BOOL = doc.createElement("BOOL");
                    type.appendChild(BOOL);
                    variable.appendChild(type);
                    inputVars.appendChild(variable);
                }
                /*C_Ptr_Obj_Can*/
                {
                    QDomElement variable = doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("Ptr_Obj_Can");
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement pointer = doc.createElement("pointer");
                    QDomElement baseType = doc.createElement("baseType");
                    QDomElement derived =doc.createElement("derived");
                    attr=doc.createAttribute("name");
                    attr.setValue("tCan");
                    derived.setAttributeNode(attr);
                    baseType.appendChild(derived);
                    pointer.appendChild(baseType);
                    type.appendChild(pointer);
                    variable.appendChild(type);
                    inputVars.appendChild(variable);
                }
                /*Start to generate variables*/
                for(const dataContainer::signal * curSignal : *curMessage->getSignalList()){
                    /*Bool signals has FrcHi and FrcLo, others FrcEn FrcVal*/
                    if(curSignal->appDataType != "BOOL"){
                        /*Create FrcEn */
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("FrcEn_"+curSignal->name);
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BOOL = doc.createElement("BOOL");
                        type.appendChild(BOOL);
                        variable.appendChild(type);
                        inputVars.appendChild(variable);
                        /*Create FrcVal */
                        variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("FrcVal_"+curSignal->name);
                        variable.setAttributeNode(attr);
                        type = doc.createElement("type");
                        QDomElement signalDataType = doc.createElement(curSignal->comDataType);
                        type.appendChild(signalDataType);
                        variable.appendChild(type);
                        inputVars.appendChild(variable);

                    }
                    else {
                        /*Create FrcHi */
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("FrcHi_"+curSignal->name);
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BOOL = doc.createElement("BOOL");
                        type.appendChild(BOOL);
                        variable.appendChild(type);
                        inputVars.appendChild(variable);
                        /*Create FrcLo */
                        variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("FrcLo_"+curSignal->name);
                        variable.setAttributeNode(attr);
                        type = doc.createElement("type");
                        BOOL = doc.createElement("BOOL");
                        type.appendChild(BOOL);
                        variable.appendChild(type);
                        inputVars.appendChild(variable);
                    }

                }
                interface.appendChild(inputVars);
            }
            /*Generate Output Variables - outputVars*/
            {
                QDomElement outputVars = doc.createElement("outputVars");;
                {
                    QDomElement variable = doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("S_Msg_TmOut");
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement BOOL = doc.createElement("BOOL");
                    type.appendChild(BOOL);
                    variable.appendChild(type);
                    outputVars.appendChild(variable);
                }
                interface.appendChild(outputVars);
            }
            /*Generate Output Input Variables - inoutVars*/
            {
                QDomElement inoutVars = doc.createElement("inOutVars");
                {
                    QDomElement variable = doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue(dutHeader);
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement dataVarType = doc.createElement("derived");
                    attr=doc.createAttribute("name");
                    attr.setValue(dutName);
                    dataVarType.setAttributeNode(attr);
                    type.appendChild(dataVarType);
                    variable.appendChild(type);
                    inoutVars.appendChild(variable);
                }
                interface.appendChild(inoutVars);
            }
            /*Generate Local Variables - inoutVars*/
            {
                QDomElement localVars= doc.createElement("localVars");
                /*Function block declaration*/
                {
                    QDomElement variable=doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("_FB_CanTx_Message_Unpack_0");
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement derived = doc.createElement("derived");
                    attr=doc.createAttribute("name");
                    attr.setValue("_FB_CanTx_Message_Unpack");
                    derived.setAttributeNode(attr);
                    type.appendChild(derived);
                    variable.appendChild(type);
                    localVars.appendChild(variable);
                }
                /*P_ID_Can*/
                {
                    QDomElement variable=doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("P_ID_Can");
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement dataVarType = doc.createElement("UDINT");
                    type.appendChild(dataVarType);
                    QDomElement initialValue = doc.createElement("initialValue");
                    QDomElement simpleValue = doc.createElement("simpleValue");
                    attr=doc.createAttribute("value");
                    attr.setValue("16#"+curMessage->getID());
                    simpleValue.setAttributeNode(attr);
                    initialValue.appendChild(simpleValue);
                    variable.appendChild(type);
                    variable.appendChild(initialValue);
                    localVars.appendChild(variable);
                }
                /*P_Tm_MsgTmOut*/
                {
                    QDomElement variable=doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("P_Tm_MsgTmOut");
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement dataVarType = doc.createElement("TIME");
                    type.appendChild(dataVarType);
                    QDomElement initialValue = doc.createElement("initialValue");
                    QDomElement simpleValue = doc.createElement("simpleValue");
                    attr=doc.createAttribute("value");
                    attr.setValue("TIME#"+curMessage->getMsTimeOut()+"ms");
                    simpleValue.setAttributeNode(attr);
                    initialValue.appendChild(simpleValue);
                    variable.appendChild(type);
                    variable.appendChild(initialValue);
                    localVars.appendChild(variable);
                }
                /*P_Msg_Extd*/
                {
                    QDomElement variable=doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("P_Msg_Extd");
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement dataVarType = doc.createElement("BOOL");
                    type.appendChild(dataVarType);
                    QDomElement initialValue = doc.createElement("initialValue");
                    QDomElement simpleValue = doc.createElement("simpleValue");
                    attr=doc.createAttribute("value");
                    attr.setValue(QString::number(curMessage->getIfExtended()).toUpper());
                    simpleValue.setAttributeNode(attr);
                    initialValue.appendChild(simpleValue);
                    variable.appendChild(type);
                    variable.appendChild(initialValue);
                    localVars.appendChild(variable);
                }

                for(unsigned i=0; i<64;i++){
                    if(i==9){
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_II_BYTE_0");
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BYTE = doc.createElement("BYTE");
                        type.appendChild(BYTE);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }else if(i==17){
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_II_BYTE_1");
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BYTE = doc.createElement("BYTE");
                        type.appendChild(BYTE);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                        variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_II_WORD_0");
                        variable.setAttributeNode(attr);
                        type = doc.createElement("type");
                        QDomElement WORD = doc.createElement("WORD");
                        type.appendChild(WORD);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }
                    else if(i==25){
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_II_BYTE_2");
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BYTE = doc.createElement("BYTE");
                        type.appendChild(BYTE);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }
                    else if(i==33){
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_II_BYTE_3");
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BYTE = doc.createElement("BYTE");
                        type.appendChild(BYTE);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                        variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_II_WORD_1");
                        variable.setAttributeNode(attr);
                        type = doc.createElement("type");
                        QDomElement WORD = doc.createElement("WORD");
                        type.appendChild(WORD);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                        variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_II_DWORD_0");
                        variable.setAttributeNode(attr);
                        type = doc.createElement("type");
                        QDomElement DWORD = doc.createElement("DWORD");
                        type.appendChild(DWORD);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }
                    else if(i==41){
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_II_BYTE_4");
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BYTE = doc.createElement("BYTE");
                        type.appendChild(BYTE);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }
                    else if(i==49){
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_II_BYTE_5");
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BYTE = doc.createElement("BYTE");
                        type.appendChild(BYTE);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                        variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_II_WORD_2");
                        variable.setAttributeNode(attr);
                        type = doc.createElement("type");
                        QDomElement WORD = doc.createElement("WORD");
                        type.appendChild(WORD);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }
                    else if(i==57){
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_II_BYTE_6");
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BYTE = doc.createElement("BYTE");
                        type.appendChild(BYTE);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }

                    {
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_II_BIT_"+QString::number(i));
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BOOL = doc.createElement("BOOL");
                        type.appendChild(BOOL);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }

                    if(i==63){
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_II_BYTE_7");
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BYTE = doc.createElement("BYTE");
                        type.appendChild(BYTE);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                        variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_II_WORD_3");
                        variable.setAttributeNode(attr);
                        type = doc.createElement("type");
                        QDomElement WORD = doc.createElement("WORD");
                        type.appendChild(WORD);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                        variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_II_DWORD_1");
                        variable.setAttributeNode(attr);
                        type = doc.createElement("type");
                        QDomElement DWORD = doc.createElement("DWORD");
                        type.appendChild(DWORD);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }
                }
                for(const dataContainer::signal * curSignal : *curMessage->getSignalList()){
                    QDomElement variable=doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("Raw_"+curSignal->name);
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement dataType = doc.createElement(curSignal->appDataType);
                    type.appendChild(dataType);
                    variable.appendChild(type);
                    localVars.appendChild(variable);
                }

                interface.appendChild(localVars);
            }
            /**
 *
 *
 *
 *
 * FUNCTION DEFINITIONS WILL BE PLACED HERE
 *
 *
 *
 *
 * /
            pou.appendChild(interface);
            /*Create Body*/
            QDomElement body = doc.createElement("body");
            QDomElement ST = doc.createElement("ST");
            QDomElement xhtml = doc.createElement("xhtml");
            attr=doc.createAttribute("xmlns");
            attr.setValue("http://www.w3.org/1999/xhtml");
            xhtml.setAttributeNode(attr);
            QString STcode;
            this->generateIIST(&STcode,curMessage);
            text=doc.createTextNode(STcode);
            xhtml.appendChild(text);
            ST.appendChild(xhtml);
            body.appendChild(ST);
            pou.appendChild(body);

            /*Create addData*/
            QDomElement addData = doc.createElement("addData");
            QDomElement data = doc.createElement("data");
            attr=doc.createAttribute("name");
            attr.setValue("http://www.3s-software.com/plcopenxml/objectid");
            data.setAttributeNode(attr);
            attr=doc.createAttribute("handleUnknown");
            attr.setValue("discard");
            data.setAttributeNode(attr);
            QDomElement ObjectId = doc.createElement("ObjectId");
            for(QList<QString> curVal : this->fbNameandObjId){
                if(curVal.at(0)== ("_FB_"+this->dutHeader+"_"+curMessage->getName()+"_0X"+curMessage->getID())){
                    text=doc.createTextNode(curVal.at(1));
                }
            }
            ObjectId.appendChild(text);
            data.appendChild(ObjectId);
            addData.appendChild(data);
            pou.appendChild(addData);
            pous->appendChild(pou);
        }

    }

}
void DBCHandler::generateIOPous(QDomElement * pous, QDomDocument &doc)
{
    QDomAttr attr;
    QDomText text;

    foreach (dataContainer * curMessage , comInterface){
        if(curMessage->getIfSelected()){
            QDomElement pou = doc.createElement("pou");
            /*set pou name*/
            attr=doc.createAttribute("name");
            attr.setValue("_FB_"+this->dutHeader+"_"+curMessage->getName()+"_0X"+curMessage->getID());
            pou.setAttributeNode(attr);
            /*set pouType*/
            attr=doc.createAttribute("pouType");
            attr.setValue("functionBlock");
            pou.setAttributeNode(attr);
            /*Interface*/
            QDomElement interface = doc.createElement("interface");
            {
                QDomElement inputVars = doc.createElement("inputVars");

                /*Generate Input Variables - inputVars*/
                /*C_Init_Can*/
                {
                    QDomElement variable = doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("C_Init_Can");
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement BOOL = doc.createElement("BOOL");
                    type.appendChild(BOOL);
                    variable.appendChild(type);
                    inputVars.appendChild(variable);
                }
                /*C_Ptr_Obj_Can*/
                {
                    QDomElement variable = doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("Ptr_Obj_Can");
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement pointer = doc.createElement("pointer");
                    QDomElement baseType = doc.createElement("baseType");
                    QDomElement derived =doc.createElement("derived");
                    attr=doc.createAttribute("name");
                    attr.setValue("tCan");
                    derived.setAttributeNode(attr);
                    baseType.appendChild(derived);
                    pointer.appendChild(baseType);
                    type.appendChild(pointer);
                    variable.appendChild(type);
                    inputVars.appendChild(variable);
                }
                /*Start to generate variables*/
                for(const dataContainer::signal * curSignal : *curMessage->getSignalList()){
                    /*Bool signals has FrcHi and FrcLo, others FrcEn FrcVal*/
                    if(curSignal->appDataType != "BOOL"){
                        /*Create FrcEn */
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("FrcEn_"+curSignal->name);
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BOOL = doc.createElement("BOOL");
                        type.appendChild(BOOL);
                        variable.appendChild(type);
                        inputVars.appendChild(variable);
                        /*Create FrcVal */
                        variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("FrcVal_"+curSignal->name);
                        variable.setAttributeNode(attr);
                        type = doc.createElement("type");
                        QDomElement signalDataType = doc.createElement(curSignal->comDataType);
                        type.appendChild(signalDataType);
                        variable.appendChild(type);
                        inputVars.appendChild(variable);

                    }
                    else {
                        /*Create FrcHi */
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("FrcHi_"+curSignal->name);
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BOOL = doc.createElement("BOOL");
                        type.appendChild(BOOL);
                        variable.appendChild(type);
                        inputVars.appendChild(variable);
                        /*Create FrcLo */
                        variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("FrcLo_"+curSignal->name);
                        variable.setAttributeNode(attr);
                        type = doc.createElement("type");
                        BOOL = doc.createElement("BOOL");
                        type.appendChild(BOOL);
                        variable.appendChild(type);
                        inputVars.appendChild(variable);
                    }

                }
                interface.appendChild(inputVars);
            }
            /*Generate Output Variables - outputVars*/
            {
                QDomElement outputVars = doc.createElement("outputVars");;
                {
                    QDomElement variable = doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("S_Msg_Snt_Ok");
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement BOOL = doc.createElement("BOOL");
                    type.appendChild(BOOL);
                    variable.appendChild(type);
                    outputVars.appendChild(variable);
                }
                interface.appendChild(outputVars);
            }
            /*Generate Output Input Variables - inoutVars*/
            {
                QDomElement inoutVars = doc.createElement("inOutVars");
                {
                    QDomElement variable = doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue(dutHeader);
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement dataVarType = doc.createElement("derived");
                    attr=doc.createAttribute("name");
                    attr.setValue(dutName);
                    dataVarType.setAttributeNode(attr);
                    type.appendChild(dataVarType);
                    variable.appendChild(type);
                    inoutVars.appendChild(variable);
                }
                interface.appendChild(inoutVars);
            }
            /*Generate Local Variables - inoutVars*/
            {
                QDomElement localVars= doc.createElement("localVars");
                /*Function block declaration*/
                {
                    QDomElement variable=doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("_FB_CanRx_Message_Unpack_0");
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement derived = doc.createElement("derived");
                    attr=doc.createAttribute("name");
                    attr.setValue("_FB_CanRx_Message_Unpack");
                    derived.setAttributeNode(attr);
                    type.appendChild(derived);
                    variable.appendChild(type);
                    localVars.appendChild(variable);
                }
                /*P_ID_Can*/
                {
                    QDomElement variable=doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("P_ID_Can");
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement dataVarType = doc.createElement("UDINT");
                    type.appendChild(dataVarType);
                    QDomElement initialValue = doc.createElement("initialValue");
                    QDomElement simpleValue = doc.createElement("simpleValue");
                    attr=doc.createAttribute("value");
                    attr.setValue("16#"+curMessage->getID());
                    simpleValue.setAttributeNode(attr);
                    initialValue.appendChild(simpleValue);
                    variable.appendChild(type);
                    variable.appendChild(initialValue);
                    localVars.appendChild(variable);
                }
                /*P_Tm_CycleTime*/
                {
                    QDomElement variable=doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("P_Tm_CycleTime");
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement dataVarType = doc.createElement("TIME");
                    type.appendChild(dataVarType);
                    QDomElement initialValue = doc.createElement("initialValue");
                    QDomElement simpleValue = doc.createElement("simpleValue");
                    attr=doc.createAttribute("value");
                    attr.setValue("TIME#"+curMessage->getMsCycleTime()+"ms");
                    simpleValue.setAttributeNode(attr);
                    initialValue.appendChild(simpleValue);
                    variable.appendChild(type);
                    variable.appendChild(initialValue);
                    localVars.appendChild(variable);
                }
                /*P_Msg_Dlc*/
                {
                    QDomElement variable=doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("P_Msg_Dlc");
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement dataVarType = doc.createElement("TIME");
                    type.appendChild(dataVarType);
                    QDomElement initialValue = doc.createElement("initialValue");
                    QDomElement simpleValue = doc.createElement("simpleValue");
                    attr=doc.createAttribute("value");
                    attr.setValue(QString::number(curMessage->getDLC()));
                    simpleValue.setAttributeNode(attr);
                    initialValue.appendChild(simpleValue);
                    variable.appendChild(type);
                    variable.appendChild(initialValue);
                    localVars.appendChild(variable);
                }
                /*P_Msg_Extd*/
                {
                    QDomElement variable=doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("P_Msg_Extd");
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement dataVarType = doc.createElement("BOOL");
                    type.appendChild(dataVarType);
                    QDomElement initialValue = doc.createElement("initialValue");
                    QDomElement simpleValue = doc.createElement("simpleValue");
                    attr=doc.createAttribute("value");
                    attr.setValue(QString::number(curMessage->getIfExtended()).toUpper());
                    simpleValue.setAttributeNode(attr);
                    initialValue.appendChild(simpleValue);
                    variable.appendChild(type);
                    variable.appendChild(initialValue);
                    localVars.appendChild(variable);
                }

                for(unsigned i=0; i<64;i++){
                    if(i==9){
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_IO_BYTE_0");
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BYTE = doc.createElement("BYTE");
                        type.appendChild(BYTE);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }else if(i==17){
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_IO_BYTE_1");
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BYTE = doc.createElement("BYTE");
                        type.appendChild(BYTE);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                        variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_IO_WORD_0");
                        variable.setAttributeNode(attr);
                        type = doc.createElement("type");
                        QDomElement WORD = doc.createElement("WORD");
                        type.appendChild(WORD);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }
                    else if(i==25){
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_IO_BYTE_2");
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BYTE = doc.createElement("BYTE");
                        type.appendChild(BYTE);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }
                    else if(i==33){
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_IO_BYTE_3");
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BYTE = doc.createElement("BYTE");
                        type.appendChild(BYTE);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                        variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_IO_WORD_1");
                        variable.setAttributeNode(attr);
                        type = doc.createElement("type");
                        QDomElement WORD = doc.createElement("WORD");
                        type.appendChild(WORD);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                        variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_IO_DWORD_0");
                        variable.setAttributeNode(attr);
                        type = doc.createElement("type");
                        QDomElement DWORD = doc.createElement("DWORD");
                        type.appendChild(DWORD);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }
                    else if(i==41){
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_IO_BYTE_4");
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BYTE = doc.createElement("BYTE");
                        type.appendChild(BYTE);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }
                    else if(i==49){
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_IO_BYTE_5");
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BYTE = doc.createElement("BYTE");
                        type.appendChild(BYTE);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                        variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_IO_WORD_2");
                        variable.setAttributeNode(attr);
                        type = doc.createElement("type");
                        QDomElement WORD = doc.createElement("WORD");
                        type.appendChild(WORD);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }
                    else if(i==57){
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_IO_BYTE_6");
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BYTE = doc.createElement("BYTE");
                        type.appendChild(BYTE);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }

                    {
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_IO_BIT_"+QString::number(i));
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BOOL = doc.createElement("BOOL");
                        type.appendChild(BOOL);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }

                    if(i==63){
                        QDomElement variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_IO_BYTE_7");
                        variable.setAttributeNode(attr);
                        QDomElement type = doc.createElement("type");
                        QDomElement BYTE = doc.createElement("BYTE");
                        type.appendChild(BYTE);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                        variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_IO_WORD_3");
                        variable.setAttributeNode(attr);
                        type = doc.createElement("type");
                        QDomElement WORD = doc.createElement("WORD");
                        type.appendChild(WORD);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                        variable = doc.createElement("variable");
                        attr=doc.createAttribute("name");
                        attr.setValue("S_IO_DWORD_1");
                        variable.setAttributeNode(attr);
                        type = doc.createElement("type");
                        QDomElement DWORD = doc.createElement("DWORD");
                        type.appendChild(DWORD);
                        variable.appendChild(type);
                        localVars.appendChild(variable);
                    }
                }
                for(const dataContainer::signal * curSignal : *curMessage->getSignalList()){
                    QDomElement variable=doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("Raw_"+curSignal->name);
                    variable.setAttributeNode(attr);
                    QDomElement type = doc.createElement("type");
                    QDomElement dataType = doc.createElement(curSignal->comDataType);
                    type.appendChild(dataType);
                    variable.appendChild(type);
                    localVars.appendChild(variable);
                    variable=doc.createElement("variable");
                    attr=doc.createAttribute("name");
                    attr.setValue("Cont_"+curSignal->name);
                    variable.setAttributeNode(attr);
                    type = doc.createElement("type");
                    dataType = doc.createElement(curSignal->appDataType);
                    type.appendChild(dataType);
                    variable.appendChild(type);
                    localVars.appendChild(variable);
                }

                interface.appendChild(localVars);
            }

            pou.appendChild(interface);
            /*Create Body*/
            QDomElement body = doc.createElement("body");
            QDomElement ST = doc.createElement("ST");
            QDomElement xhtml = doc.createElement("xhtml");
            attr=doc.createAttribute("xmlns");
            attr.setValue("http://www.w3.org/1999/xhtml");
            xhtml.setAttributeNode(attr);
            QString STcode;
            this->generateIOST(&STcode,curMessage);
            text=doc.createTextNode(STcode);
            xhtml.appendChild(text);
            ST.appendChild(xhtml);
            body.appendChild(ST);
            pou.appendChild(body);
            /**
 *
 *
 *
 *
 * FUNCTION DEFINITIONS WILL BE PLACED HERE
 *
 *
 *
 *
 * /
            /*Create addData*/
            QDomElement addData = doc.createElement("addData");
            QDomElement data = doc.createElement("data");
            attr=doc.createAttribute("name");
            attr.setValue("http://www.3s-software.com/plcopenxml/objectid");
            data.setAttributeNode(attr);
            attr=doc.createAttribute("handleUnknown");
            attr.setValue("discard");
            data.setAttributeNode(attr);
            QDomElement ObjectId = doc.createElement("ObjectId");
            for(QList<QString> curVal : this->fbNameandObjId){
                if(curVal.at(0)== ("_FB_"+this->dutHeader+"_"+curMessage->getName()+"_0X"+curMessage->getID())){
                    text=doc.createTextNode(curVal.at(1));
                }
            }
            ObjectId.appendChild(text);
            data.appendChild(ObjectId);
            addData.appendChild(data);
            pou.appendChild(addData);
            pous->appendChild(pou);
        }

    }


}

void DBCHandler::generateIOST(QString *const ST,dataContainer *const curMessage)
{
    ST->append(    "_FB_CanTx_Message_Unpack(\n"
                   "C_Enable:= C_Init_Can,\n"
                   "Obj_CAN:= Ptr_Obj_Can,\n"
                   "X_MsgID:= P_ID_Can,\n"
                   "Tm_CycleTime:= P_Tm_MsgCyc,\n"
                   "Msg_Dlc:=P_Msg_Dlc,\n"
                   "S_Extended:= P_Msg_Extd,\n"
                   "S_Bit_0:=	S_IO_BIT_0	,\n"
                   "S_Bit_1:=	S_IO_BIT_1	,\n"
                   "S_Bit_2:=	S_IO_BIT_2	,\n"
                   "S_Bit_3:=	S_IO_BIT_3	,\n"
                   "S_Bit_4:=	S_IO_BIT_4	,\n"
                   "S_Bit_5:=	S_IO_BIT_5	,\n"
                   "S_Bit_6:=	S_IO_BIT_6	,\n"
                   "S_Bit_7:=	S_IO_BIT_7	,\n"
                   "X_Byte_0:= 	X_IO_BYTE_0	,\n"
                   "S_Bit_8:= 	S_IO_BIT_8	,\n"
                   "S_Bit_9:= 	S_IO_BIT_9	,\n"
                   "S_Bit_10:= 	S_IO_BIT_10	,\n"
                   "S_Bit_11:= 	S_IO_BIT_11	,\n"
                   "S_Bit_12:= 	S_IO_BIT_12	,\n"
                   "S_Bit_13:= 	S_IO_BIT_13	,\n"
                   "S_Bit_14:= 	S_IO_BIT_14	,\n"
                   "S_Bit_15:= 	S_IO_BIT_15	,\n"
                   "X_Byte_1:= 	X_IO_BYTE_1	,\n"
                   "X_Word_0:= 	X_IO_WORD_0	,\n"
                   "S_Bit_16:= 	S_IO_BIT_16	,\n"
                   "S_Bit_17:= 	S_IO_BIT_17	,\n"
                   "S_Bit_18:= 	S_IO_BIT_18	,\n"
                   "S_Bit_19:= 	S_IO_BIT_19	,\n"
                   "S_Bit_20:= 	S_IO_BIT_20	,\n"
                   "S_Bit_21:= 	S_IO_BIT_21	,\n"
                   "S_Bit_22:= 	S_IO_BIT_22	,\n"
                   "S_Bit_23:= 	S_IO_BIT_23	,\n"
                   "X_Byte_2:= 	X_IO_BYTE_2	,\n"
                   "S_Bit_24:= 	S_IO_BIT_24	,\n"
                   "S_Bit_25:= 	S_IO_BIT_25	,\n"
                   "S_Bit_26:= 	S_IO_BIT_26	,\n"
                   "S_Bit_27:= 	S_IO_BIT_27	,\n"
                   "S_Bit_28:= 	S_IO_BIT_28	,\n"
                   "S_Bit_29:= 	S_IO_BIT_29	,\n"
                   "S_Bit_30:= 	S_IO_BIT_30	,\n"
                   "S_Bit_31:= 	S_IO_BIT_31	,\n"
                   "X_Byte_3:= 	X_IO_BYTE_3	,\n"
                   "X_Word_1:= 	X_IO_WORD_1	,\n"
                   "X_DWord_0:= X_IO_DWORD_0,\n"
                   "S_Bit_32:= 	S_IO_BIT_32	,\n"
                   "S_Bit_33:= 	S_IO_BIT_33	,\n"
                   "S_Bit_34:= 	S_IO_BIT_34	,\n"
                   "S_Bit_35:= 	S_IO_BIT_35	,\n"
                   "S_Bit_36:= 	S_IO_BIT_36	,\n"
                   "S_Bit_37:= 	S_IO_BIT_37	,\n"
                   "S_Bit_38:= 	S_IO_BIT_38	,\n"
                   "S_Bit_39:= 	S_IO_BIT_39	,\n"
                   "X_Byte_4:= 	X_IO_BYTE_4	,\n"
                   "S_Bit_40:= 	S_IO_BIT_40	,\n"
                   "S_Bit_41:= 	S_IO_BIT_41	,\n"
                   "S_Bit_42:= 	S_IO_BIT_42	,\n"
                   "S_Bit_43:= 	S_IO_BIT_43	,\n"
                   "S_Bit_44:= 	S_IO_BIT_44	,\n"
                   "S_Bit_45:= 	S_IO_BIT_45	,\n"
                   "S_Bit_46:= 	S_IO_BIT_46	,\n"
                   "S_Bit_47:= 	S_IO_BIT_47	,\n"
                   "X_Byte_5:= 	X_IO_BYTE_5	,\n"
                   "X_Word_2:= 	X_IO_WORD_2	,\n"
                   "S_Bit_48:= 	S_IO_BIT_48	,\n"
                   "S_Bit_49:= 	S_IO_BIT_49	,\n"
                   "S_Bit_50:= 	S_IO_BIT_50	,\n"
                   "S_Bit_51:= 	S_IO_BIT_51	,\n"
                   "S_Bit_52:= 	S_IO_BIT_52	,\n"
                   "S_Bit_53:= 	S_IO_BIT_53	,\n"
                   "S_Bit_54:= 	S_IO_BIT_54	,\n"
                   "S_Bit_55:= 	S_IO_BIT_55	,\n"
                   "X_Byte_6:= 	X_IO_BYTE_6	,\n"
                   "S_Bit_56:= 	S_IO_BIT_56	,\n"
                   "S_Bit_57:= 	S_IO_BIT_57	,\n"
                   "S_Bit_58:= 	S_IO_BIT_58	,\n"
                   "S_Bit_59:= 	S_IO_BIT_59	,\n"
                   "S_Bit_60:= 	S_IO_BIT_60	,\n"
                   "S_Bit_61:= 	S_IO_BIT_61	,\n"
                   "S_Bit_62:= 	S_IO_BIT_62	,\n"
                   "S_Bit_63:= 	S_IO_BIT_63	,\n"
                   "X_Byte_7:= 	X_IO_BYTE_7	,\n"
                   "X_Word_3:= 	X_IO_WORD_3	,\n"
                   "X_DWord_1:= X_IO_DWORD_1,\n"
                   "S_Sent_Ok=&gt; S_Msg_Snt_Ok);\n" );
}
QString DBCHandler::convTypeApptoCom (QString signalName, unsigned short starbit,  QString converType)
{

}
void DBCHandler::generateIIST(QString *const ST,dataContainer *const curMessage)
{
    ST->append( "_FB_CanRx_Message_Unpack_0(\n"
                "C_Enable:= C_Init_Can,\n"
                "Obj_CAN:= Ptr_Obj_Can ,\n"
                "X_MsgID:= P_ID_Can,\n"
                "Tm_MsgTmOut:= P_Tm_MsgTmOut,\n"
                "S_Extended:= P_Msg_Extd,\n"
                "S_ER_TmOut=&gt; S_Msg_TmOut,\n"
                "S_Bit_0=&gt;  S_II_BIT_0	 ,\n"
                "S_Bit_1=&gt;  S_II_BIT_1	 ,\n"
                "S_Bit_2=&gt;  S_II_BIT_2	 ,\n"
                "S_Bit_3=&gt;  S_II_BIT_3	 ,\n"
                "S_Bit_4=&gt;  S_II_BIT_4	 ,\n"
                "S_Bit_5=&gt;  S_II_BIT_5	 ,\n"
                "S_Bit_6=&gt;  S_II_BIT_6	 ,\n"
                "S_Bit_7=&gt;  S_II_BIT_7	 ,\n"
                "X_Byte_0=&gt; X_II_BYTE_0	 ,\n"
                "S_Bit_8=&gt;  S_II_BIT_8	 ,\n"
                "S_Bit_9=&gt;  S_II_BIT_9	 ,\n"
                "S_Bit_10=&gt; S_II_BIT_10	 ,\n"
                "S_Bit_11=&gt; S_II_BIT_11	 ,\n"
                "S_Bit_12=&gt; S_II_BIT_12	 ,\n"
                "S_Bit_13=&gt; S_II_BIT_13	 ,\n"
                "S_Bit_14=&gt; S_II_BIT_14	 ,\n"
                "S_Bit_15=&gt; S_II_BIT_15	 ,\n"
                "X_Byte_1=&gt; X_II_BYTE_1	 ,\n"
                "X_Word_0=&gt; X_II_WORD_0	 ,\n"
                "S_Bit_16=&gt; S_II_BIT_16	 ,\n"
                "S_Bit_17=&gt; S_II_BIT_17	 ,\n"
                "S_Bit_18=&gt; S_II_BIT_18	 ,\n"
                "S_Bit_19=&gt; S_II_BIT_19	 ,\n"
                "S_Bit_20=&gt; S_II_BIT_20	 ,\n"
                "S_Bit_21=&gt; S_II_BIT_21	 ,\n"
                "S_Bit_22=&gt; S_II_BIT_22	 ,\n"
                "S_Bit_23=&gt; S_II_BIT_23	 ,\n"
                "X_Byte_2=&gt; X_II_BYTE_2	 ,\n"
                "S_Bit_24=&gt; S_II_BIT_24	 ,\n"
                "S_Bit_25=&gt; S_II_BIT_25	 ,\n"
                "S_Bit_26=&gt; S_II_BIT_26	 ,\n"
                "S_Bit_27=&gt; S_II_BIT_27	 ,\n"
                "S_Bit_28=&gt; S_II_BIT_28	 ,\n"
                "S_Bit_29=&gt; S_II_BIT_29	 ,\n"
                "S_Bit_30=&gt; S_II_BIT_30	 ,\n"
                "S_Bit_31=&gt; S_II_BIT_31	 ,\n"
                "X_Byte_3=&gt; X_II_BYTE_3	 ,\n"
                "X_Word_1=&gt; X_II_WORD_1	 ,\n"
                "X_DWord_0=&gt; X_II_DWORD_0 ,\n"
                "S_Bit_32=&gt; S_II_BIT_32	 ,\n"
                "S_Bit_33=&gt; S_II_BIT_33	 ,\n"
                "S_Bit_34=&gt; S_II_BIT_34	 ,\n"
                "S_Bit_35=&gt; S_II_BIT_35	 ,\n"
                "S_Bit_36=&gt; S_II_BIT_36	 ,\n"
                "S_Bit_37=&gt; S_II_BIT_37	 ,\n"
                "S_Bit_38=&gt; S_II_BIT_38	 ,\n"
                "S_Bit_39=&gt; S_II_BIT_39	 ,\n"
                "X_Byte_4=&gt; X_II_BYTE_4	 ,\n"
                "S_Bit_40=&gt; S_II_BIT_40	 ,\n"
                "S_Bit_41=&gt; S_II_BIT_41	 ,\n"
                "S_Bit_42=&gt; S_II_BIT_42	 ,\n"
                "S_Bit_43=&gt; S_II_BIT_43	 ,\n"
                "S_Bit_44=&gt; S_II_BIT_44	 ,\n"
                "S_Bit_45=&gt; S_II_BIT_45	 ,\n"
                "S_Bit_46=&gt; S_II_BIT_46	 ,\n"
                "S_Bit_47=&gt; S_II_BIT_47	 ,\n"
                "X_Byte_5=&gt; X_II_BYTE_5	 ,\n"
                "X_Word_2=&gt; X_II_WORD_2	 ,\n"
                "S_Bit_48=&gt; S_II_BIT_48	 ,\n"
                "S_Bit_49=&gt; S_II_BIT_49	 ,\n"
                "S_Bit_50=&gt; S_II_BIT_50	 ,\n"
                "S_Bit_51=&gt; S_II_BIT_51	 ,\n"
                "S_Bit_52=&gt; S_II_BIT_52	 ,\n"
                "S_Bit_53=&gt; S_II_BIT_53	 ,\n"
                "S_Bit_54=&gt; S_II_BIT_54	 ,\n"
                "S_Bit_55=&gt; S_II_BIT_55	 ,\n"
                "X_Byte_6=&gt; X_II_BYTE_6	 ,\n"
                "S_Bit_56=&gt; S_II_BIT_56	 ,\n"
                "S_Bit_57=&gt; S_II_BIT_57	 ,\n"
                "S_Bit_58=&gt; S_II_BIT_58	 ,\n"
                "S_Bit_59=&gt; S_II_BIT_59	 ,\n"
                "S_Bit_60=&gt; S_II_BIT_60	 ,\n"
                "S_Bit_61=&gt; S_II_BIT_61	 ,\n"
                "S_Bit_62=&gt; S_II_BIT_62	 ,\n"
                "S_Bit_63=&gt; S_II_BIT_63	 ,\n"
                "X_Byte_7=&gt; X_II_BYTE_7	 ,\n"
                "X_Word_3=&gt; X_II_WORD_3	 ,\n"
                "X_DWord_1=&gt;X_II_DWORD_1   );\n");

    for( const dataContainer::signal * curSignal : *curMessage->getSignalList()){
        ST->append(convTypeComtoApp(curSignal->name,curSignal->startBit,curSignal->length,curSignal->convMethod));
    }
}
QString DBCHandler::convTypeComtoApp(QString signalName, unsigned short startbit,unsigned short length,  QString converType)
{
    QString ST="\n\n\n(*THIS AREA RESERVED FOR "+signalName+"*)\n\n\n";
    if(converType=="BOOL:BOOL"){
        ST.append("\n"+this->dutHeader+"."+signalName+".v               := NOT S_Msg_TmOut OR FrcHi_"+signalName+"OR FrcLo_"+signalName+" ;"
                  "\n"+this->dutHeader+"."+signalName+".x                := "+this->dutHeader+"."+signalName+".v AND (S_II_BIT_"+QString::number(startbit)+" OR FrcHi_"+signalName+") AND NOT FrcLo_"+signalName+" ;");


    }else if(converType=="2BOOL:BOOL"){
        ST.append("\n"+this->dutHeader+"."+signalName+".v               := ((NOT S_Msg_TmOut AND NOT S_II_BIT_"+QString::number(startbit+1)+") OR FrcHi_"+signalName+" OR FrcLo_"+signalName+") ;"
                   "\n"+this->dutHeader+"."+signalName+".x               := "+this->dutHeader+"."+signalName+".v AND (S_II_BIT_"+QString::number(startbit+1)+" OR FrcHi_"+signalName+") AND NOT FrcLo_"+signalName+";");
    }/*else if(converType=="BYTE:REAL"){

        }else if(converType=="BYTE:USINT"){

        }else if(converType=="BYTE:BYTE"){

        }else if(converType=="WORD:REAL"){

        }else if(converType=="WORD:UINT"){

        }else if(converType=="WORD:WORD"){

        }else if(converType=="DWORD:REAL"){

        }else if(converType=="DWORD:UDINT"){

        }else if(converType=="DWORD:DWORD"){

        }else if(converType=="2DWORD:LREAL"){

        }else if(converType=="2DWORD:ULINT"){

        }else if(converType=="2DWORD:LWORD"){

        }*/else{
        bool flagPack = false;
        if((length>7)){
            if(length<17){
                ST.append("\n_FB_PACK_BYTE_TO_WORD_0();");
            }else if(length <33){
                ST.append("\n_FB_PACK_BYTE_TO_DWORD_0();");
            }else {
                ST.append("\n_FB_PACK_BYTE_TO_DWORD_0();");
                ST.append("\n_FB_PACK_BYTE_TO_DWORD_1();\n");
            }
        }
        unsigned packID=0;
        unsigned packByteID=0;
        for(unsigned i =0; i<=length;i++){



            if(flagPack){
                if((i%8 == 0) && (i>0)){
                    ST.append(" ,X_BYTE_0=>"+ ((length<8) ? ("Raw_"+signalName) : ((length<16) ? "_FB_PACK_BYTE_TO_WORD_" :"_FB_PACK_BYTE_TO_DWORD_"+QString::number(packID)+".X_BYTE_"+QString::number(packByteID)))+");");
                    flagPack=false;
                    packByteID++;
                }
            }
            if((i%32 == 0) && (i>0) && length>16){
                packID++;
                packByteID=0;
            }
            if((i%16 == 0) && (i>0) && length<=16){
                packID++;
                packByteID=0;
            }

            if(((i%8 == 0) && ((i/8)<8) )&& (!flagPack)){
                ST.append("\n_FB_PACK_8BIT_TO_BYTE_"+QString::number(qFloor(i/8.0))+"(");
                flagPack=true;
            }
            if(flagPack){
                if((i%8 != 0)){
                    ST.append("S_BIT_"+QString::number(i%8)+":= S_II_BIT_"+QString::number(startbit+i)+"");
                    if(i%8 != (qMin(unsigned(7),length-unsigned(1)))){
                        ST.append(",");
                    }
                }
            }

        }
        if((length>7)){
            if(length<17){
                ST.append("\nRaw_"+signalName+"            :=_FB_PACK_BYTE_TO_WORD_0.X_WORD_0;");
            }else if(length <33){
                ST.append("\nRaw_"+signalName+"            :=_FB_PACK_BYTE_TO_DWORD_0.X_DWORD_0;");
            }else {
                ST.append("\n_FB_PACK_DWORD_TO_LWORD_0(X_DWORD_0:=_FB_PACK_BYTE_TO_DWORD_0.X_DWORD_0,X_DWORD_1:=_FB_PACK_BYTE_TO_DWORD_1.X_DWORD_0,X_LWORD_0=> Raw_"+signalName+");");
            }
        }
    }
    if(length==1){
        ST.append("\n"+this->dutHeader+"."+signalName+".e				:= FALSE;");
        ST.append("\n"+this->dutHeader+"."+signalName+".na				:= FALSE;");
    }
    else if((length==2) && (converType != "toBYTE")){

        ST.append("\n"+this->dutHeader+"."+signalName+".e				:= X_II_BIT_"+QString::number(startbit+1)+" AND NOT X_II_BIT_"+QString::number(startbit)+" ;");
        ST.append("\n"+this->dutHeader+"."+signalName+".na				:= X_II_BIT_"+QString::number(startbit+1)+" AND X_II_BIT_"+QString::number(startbit)+" ;");

    }
    else if((length<9)){
        if(converType == "toBYTE"){
            ST.append("\nCont_"+signalName+"				:= Raw_"+signalName+" ;");
        }
        else if (converType == "toUSINT"){
            ST.append("\nCont_"+signalName+"				:= BYTE_TO_USINT(Raw_"+signalName+") ;");
        }
        else if (converType == "toREAL"){
            ST.append("\nCont_"+signalName+"				:= USINT_TO_REAL(BYTE_TO_USINT(Raw_"+signalName+"))*"+this->dutHeader+"."+signalName+".Param_Res+"+this->dutHeader+"."+signalName+".Param_Off ;");
        }
        ST.append( "\n"+this->dutHeader+"."+signalName+".e              :=  (Raw_"+signalName+" > 16#FDFF) AND (Raw_"+signalName+" < 16#FF00) ;");
        ST.append("\n"+this->dutHeader+"."+signalName+".na              := (Raw_"+signalName+" > 16#FEFF) ;");

    }else if((length<17)){
        if(converType == "toBYTE"){
            ST.append("\nCont_"+signalName+"				:= Raw_"+signalName+" ;");
        }
        else if (converType == "toUSINT"){
            ST.append("\nCont_"+signalName+"				:= WORD_TO_UINT(Raw_"+signalName+") ;");
        }
        else if (converType == "toREAL"){
            ST.append("\n Cont_"+signalName+"				:= UINT_TO_REAL(WORD_TO_UINT(Raw_"+signalName+"))*"+this->dutHeader+"."+signalName+".Param_Res+"+this->dutHeader+"."+signalName+".Param_Off ;");
        }
        ST.append("\n"+this->dutHeader+"."+signalName+".e              := (Raw_"+signalName+" > 16#FDFFFFFF) AND (Raw_"+signalName+" < 16#FF000000) ;");
        ST.append("\n"+this->dutHeader+"."+signalName+".na              := (Raw_"+signalName+" > 16#FEFFFFFF) ;");

    }else if((length<33)){
        if(converType == "toBYTE"){
            ST.append("\nCont_"+signalName+"               := Raw_"+signalName+" ;");
        }
        else if (converType == "toUSINT"){
            ST.append("\nCont_"+signalName+"				:= DWORD_TO_UDINT(Raw_"+signalName+") ;");
        }
        else if (converType == "toREAL"){
            ST.append("\nCont_"+signalName+"				:= UDINT_TO_REAL(DWORD_TO_UDINT(Raw_"+signalName+"))*"+this->dutHeader+"."+signalName+".Param_Res+"+this->dutHeader+"."+signalName+".Param_Off ;");
        }
        ST.append("\n"+this->dutHeader+"."+signalName+".e              := (Raw_"+signalName+" > 16#FDFFFFFFFFFFFFFF) AND (Raw_"+signalName+" < 16#FF00000000000000) ;");
        ST.append("\n"+this->dutHeader+"."+signalName+".na              := (Raw_"+signalName+" > 16#FEFFFFFFFFFFFFFF) ;");

    }else if((length<65)){
        if(converType == "toBYTE"){
            ST.append("\nCont_"+signalName+"               := Raw_"+signalName+" ;");
        }
        else if (converType == "toUSINT"){
            ST.append("\nCont_"+signalName+"				:= LWORD_TO_ULINT(Raw_"+signalName+") ;");
        }
        else if (converType == "toREAL"){
            ST.append("\nCont_"+signalName+"				:= ULINT_TO_REAL(LWORD_TO_ULINT(Raw_"+signalName+"))*"+this->dutHeader+"."+signalName+".Param_Res+"+this->dutHeader+"."+signalName+".Param_Off ;");
        }
        ST.append("\n"+this->dutHeader+"."+signalName+".e              := (Raw_"+signalName+" > 16#FDFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF) AND (Raw_"+signalName+" < 16#FF000000000000000000000000000000) ;");
        ST.append("\n"+this->dutHeader+"."+signalName+".na              := (Raw_"+signalName+" > 16#FEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF) ;");

    }

    if((length!= 1) && (!(length==2 && (converType !="toByte")))){
        ST.append("\n"+this->dutHeader+"."+signalName+".RangeExcd 		:= NOT ((Cont_"+signalName+" >= "+this->dutHeader+"."+signalName+".Param_Min) AND ("+this->dutHeader+"."+signalName+".Param_Min <= "+this->dutHeader+"."+signalName+".Param_Max));"
        "\n"+this->dutHeader+"."+signalName+".v				:= NOT( S_Msg_TmOut OR "+this->dutHeader+"."+signalName+".RangeExcd OR (( "+this->dutHeader+"."+signalName+".e OR "+this->dutHeader+"."+signalName+".na) AND IIDUT_NAME>."+signalName+".J1939)) OR FrcEn_"+signalName+";"
        "\n"
        "\nIF FrcEn_"+signalName+" THEN"
        "\n	"+this->dutHeader+"."+signalName+".x 		   	:= FrcVal_"+signalName+";"
        "\nELSIF "+this->dutHeader+"."+signalName+".v THEN"
        "\n	"+this->dutHeader+"."+signalName+".x 		   	:= Cont_"+signalName+";"
        "\nELSE"
        "\n	"+this->dutHeader+"."+signalName+".x 		   	:= <IODUT_NAME>."+signalName+".Param_Def;"
        "\nEND_IF;");
    }
    return ST;
}






