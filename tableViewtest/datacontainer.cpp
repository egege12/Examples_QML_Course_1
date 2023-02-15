#include "datacontainer.h"
#include <QDebug>

unsigned int dataContainer::messageCounter = 0;
unsigned int dataContainer::signalCounter = 0;
QList<QString> dataContainer::warningMessages ={};
dataContainer::dataContainer(QObject *parent)
{
    ++dataContainer::messageCounter;
    this->isInserted=false;
    this->isSelected = false;
    this->msTimeout ="2500";
    this->msCycleTime ="100";
    this->comment="No Comment";
}

bool dataContainer::addSignal(signal newSignal)
{
    ++signalCounter;
    signal *Ptr = new signal;
    *Ptr = newSignal;
    dataTypeAss(Ptr);
    this->signalList.append(Ptr);
    return true;
}

const QList<dataContainer::signal *> *dataContainer::getSignalList()
{
    return &signalList;
}

QString dataContainer::getName()
{
    return this->Name;
}

QString dataContainer::getID()
{
    return this->messageID;
}

QString dataContainer::getMsTimeOut()
{
    return this->msTimeout;
}

QString dataContainer::getMsCycleTime()
{
    return this->msCycleTime;
}

QString dataContainer::getComment()
{
    return this->comment;
}



bool dataContainer::getIfSelected()
{
    return this->isSelected;
}

bool dataContainer::getIfExtended()
{
    return this->isExtended;
}

unsigned short dataContainer::getDLC()
{
    return this->dlc;
}

const QList<QString> *dataContainer::getWarningList()
{
    return &dataContainer::warningMessages;
}

void dataContainer::setName(QString Name)
{
    this->Name = Name;

}

void dataContainer::setmessageID(QString messageID)
{

    bool isExtended = (messageID.toULong()> 1000);
    this->messageID = messageID;
    this->isExtended = isExtended;

}

void dataContainer::setDLC(unsigned short DLC)
{
    this->dlc= DLC;
}

void dataContainer::setSelected()
{
    this->isSelected = !this->isSelected;
}

void dataContainer::setInserted()
{
    this->isInserted = true;
}

void dataContainer::setMsTimeOut(QString msTimeout)
{
    this->msTimeout = msTimeout;
}

void dataContainer::setMsCycleTime(QString msCycleTime)
{
    this->msCycleTime = msCycleTime;
}

void dataContainer::setComment(QString comment)
{
    this->comment = comment;
}

void dataContainer::setWarning(const QString &warningCode)
{
    dataContainer::warningMessages.append(warningCode);
}

void dataContainer::dataTypeAss(signal *signalPtr)
{
    if(signalPtr->length == 1 ){
        signalPtr->appDataType = "BOOL";
        signalPtr->comDataType = "BOOL";
    }else if (signalPtr->length == 2){
        signalPtr->comDataType = "2XBOOL";
        if((signalPtr->isJ1939) || signalPtr->name.contains("C_") || signalPtr->name.contains("S_")){
            signalPtr->appDataType = "BOOL";
        }else if (signalPtr->name.contains("Z_")){
            signalPtr->appDataType = "BYTE";
        }else{
            signalPtr->appDataType="BYTE";
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde C_ S_ Z_ işareti bulunmuyor");
        }
    }else if (signalPtr->length < 8){
        signalPtr->comDataType = QString::number(signalPtr->length) + QString("XBOOL");
        if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
            signalPtr->appDataType = "REAL";
        }else if(signalPtr->name.contains("N_")){
            signalPtr->appDataType = "USINT";
        }else if(signalPtr->name.contains("Z_")){
            signalPtr->appDataType = "BYTE";
        }else{
            signalPtr->appDataType="BYTE";
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");
        }

    }else if (signalPtr->length == 8){
        signalPtr->comDataType = "BYTE";
        if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
            signalPtr->appDataType = "REAL";
        }else if(signalPtr->name.contains("N_")){
            signalPtr->appDataType = "USINT";
        }else if(signalPtr->name.contains("Z_")){
            signalPtr->appDataType = "BYTE";
        }else{
            signalPtr->appDataType="BYTE";
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");
        }

    }else if (signalPtr->length <  16){
        signalPtr->comDataType = QString::number(signalPtr->length) + QString("XBOOL");
        if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
            signalPtr->appDataType = "REAL";
        }else if(signalPtr->name.contains("N_")){
            signalPtr->appDataType = "UINT";
        }else if(signalPtr->name.contains("Z_")){
            signalPtr->appDataType = "WORD";
        }else{
            signalPtr->appDataType="WORD";
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");
        }

    }else if (signalPtr->length == 16){
        signalPtr->comDataType = "WORD";
        if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
            signalPtr->appDataType = "REAL";
        }else if(signalPtr->name.contains("N_")){
            signalPtr->appDataType = "UINT";
        }else if(signalPtr->name.contains("Z_")){
            signalPtr->appDataType = "WORD";
        }else{
            signalPtr->appDataType="WORD";
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");
        }
    }else if (signalPtr->length < 32){
        signalPtr->comDataType = QString::number(signalPtr->length) + QString("XBOOL");
        if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
            signalPtr->appDataType = "REAL";
        }else if(signalPtr->name.contains("N_")){
            signalPtr->appDataType = "UDINT";
        }else if(signalPtr->name.contains("Z_")){
            signalPtr->appDataType = "DWORD";
        }else{
            signalPtr->appDataType="DWORD";
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");
        }
    }else if (signalPtr->length == 32){
        signalPtr->comDataType = "DWORD";
        if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
            signalPtr->appDataType = "REAL";
        }else if(signalPtr->name.contains("N_")){
            signalPtr->appDataType = "UDINT";
        }else if(signalPtr->name.contains("Z_")){
            signalPtr->appDataType = "DWORD";
        }else{
            signalPtr->appDataType="DWORD";
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");
        }
    }else if (signalPtr->length < 64){
        signalPtr->comDataType = QString::number(signalPtr->length) + QString("XBOOL");
        if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
            signalPtr->appDataType = "LREAL";
        }else if(signalPtr->name.contains("N_")){
            signalPtr->appDataType = "ULINT";
        }else if(signalPtr->name.contains("Z_")){
            signalPtr->appDataType = "LWORD";
        }else{
            signalPtr->appDataType="LWORD";
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");
        }
    }else if (signalPtr->length == 64){
        signalPtr->comDataType = "LWORD";
        if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
            signalPtr->appDataType = "LREAL";
        }else if(signalPtr->name.contains("N_")){
            signalPtr->appDataType = "ULINT";
        }else if(signalPtr->name.contains("Z_")){
            signalPtr->appDataType = "LWORD";
        }else{
            signalPtr->appDataType = "LWORD";
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");
        }
    }
}


dataContainer::~dataContainer()
{
    --dataContainer::messageCounter;
    signalCounter = 0;
    for(signal * curSignal : signalList){
        delete curSignal;
    }
    signalList.clear();
}
