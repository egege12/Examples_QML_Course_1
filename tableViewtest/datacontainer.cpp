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
        signalPtr->convMethod="BOOL:BOOL";
    }else if (signalPtr->length == 2){ 
        if((signalPtr->isJ1939) || signalPtr->name.contains("C_") || signalPtr->name.contains("S_")){
            signalPtr->appDataType = "BOOL";
            signalPtr->convMethod="2BOOL:BOOL";
            signalPtr->comDataType = "BOOL";
        }else if (signalPtr->name.contains("Z_")){
            signalPtr->appDataType = "BYTE";
            signalPtr->convMethod="toBYTE";
            signalPtr->comDataType = "BYTE";
        }else{
            signalPtr->appDataType="BYTE";
            signalPtr->convMethod="toBYTE";
            signalPtr->comDataType = "BYTE";
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde C_ S_ Z_ işareti bulunmuyor");
        }
    }else if (signalPtr->length < 8){
        signalPtr->comDataType = "BYTE";
        if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
            signalPtr->appDataType = "REAL";
            signalPtr->convMethod="toREAL";
        }else if(signalPtr->name.contains("N_")){
            signalPtr->appDataType = "USINT";
            signalPtr->convMethod="toUSINT";
        }else if(signalPtr->name.contains("Z_")){
            signalPtr->appDataType = "BYTE";
            signalPtr->convMethod="toBYTE";
        }else{
            signalPtr->appDataType="BYTE";
            signalPtr->convMethod="toBYTE";
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");
        }

    }else if (signalPtr->length == 8){
        if((signalPtr->startBit == 0 )||(signalPtr->startBit == 8 )||(signalPtr->startBit == 16 )||(signalPtr->startBit == 24 )||(signalPtr->startBit == 32 )||(signalPtr->startBit == 40 )||(signalPtr->startBit == 48 )||(signalPtr->startBit == 56 )){
            signalPtr->comDataType = "BYTE";
            if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
                signalPtr->appDataType = "REAL";
                signalPtr->convMethod="BYTE:REAL";
            }else if(signalPtr->name.contains("N_")){
                signalPtr->appDataType = "USINT";
                signalPtr->convMethod="BYTE:USINT";
            }else if(signalPtr->name.contains("Z_")){
                signalPtr->appDataType = "BYTE";
                signalPtr->convMethod="BYTE:BYTE";
            }else{
                signalPtr->appDataType="BYTE";
                signalPtr->convMethod="BYTE:BYTE";
                this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");
            }
        }else{
            signalPtr->comDataType = "BYTE";
            if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
                signalPtr->appDataType = "REAL";
                signalPtr->convMethod="toREAL";
            }else if(signalPtr->name.contains("N_")){
                signalPtr->appDataType = "USINT";
                signalPtr->convMethod="toUSINT";
            }else if(signalPtr->name.contains("Z_")){
                signalPtr->appDataType = "BYTE";
                signalPtr->convMethod="toBYTE";
            }else{
                signalPtr->appDataType="BYTE";
                signalPtr->convMethod="toBYTE";
                this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");

            }
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali başlangıç biti 8 ve katları değil, düşük performans");
        }
    }else if (signalPtr->length <  16){
        if((signalPtr->startBit == 0 )||(signalPtr->startBit == 8 )||(signalPtr->startBit == 16 )||(signalPtr->startBit == 24 )||(signalPtr->startBit == 32 )||(signalPtr->startBit == 40 )||(signalPtr->startBit == 48 )){
            signalPtr->comDataType = "WORD";
            if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
                signalPtr->appDataType = "REAL";
                signalPtr->convMethod="toREAL";
            }else if(signalPtr->name.contains("N_")){
                signalPtr->appDataType = "UINT";
                signalPtr->convMethod="toUINT";
            }else if(signalPtr->name.contains("Z_")){
                signalPtr->appDataType = "WORD";
                signalPtr->convMethod="toWORD";
            }else{
                signalPtr->appDataType="WORD";
                signalPtr->convMethod="toWORD";
                this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");

            }
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali veri boyutu 8 ve katları değil,standart olmayan veri transferi");
        }else{
            signalPtr->comDataType = "WORD";
            if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
                signalPtr->appDataType = "REAL";
                signalPtr->convMethod="toREAL";
            }else if(signalPtr->name.contains("N_")){
                signalPtr->appDataType = "UINT";
                signalPtr->convMethod="toUINT";
            }else if(signalPtr->name.contains("Z_")){
                signalPtr->appDataType = "WORD";
                signalPtr->convMethod="toWORD";
            }else{
                signalPtr->appDataType="WORD";
                signalPtr->convMethod="toWORD";
                this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");

            }
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali veri boyutu 8 ve katları değil,standart olmayan veri transferi");
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali başlangıç biti 8 ve katları değil,düşük perfomans");
        }
    }else if (signalPtr->length == 16){
        if((signalPtr->startBit == 0 )||(signalPtr->startBit == 8 )||(signalPtr->startBit == 16 )||(signalPtr->startBit == 24 )||(signalPtr->startBit == 32 )||(signalPtr->startBit == 40 )||(signalPtr->startBit == 48 )){
            signalPtr->comDataType = "WORD";
            if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
                signalPtr->appDataType = "REAL";
                signalPtr->convMethod="WORD:REAL";
            }else if(signalPtr->name.contains("N_")){
                signalPtr->appDataType = "UINT";
                signalPtr->convMethod="WORD:UINT";
            }else if(signalPtr->name.contains("Z_")){
                signalPtr->convMethod="WORD:WORD";
            }else{
                signalPtr->appDataType="WORD";
                signalPtr->convMethod="WORD:WORD";
                this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");
            }
        }else{
            signalPtr->comDataType = "WORD";
            if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
                signalPtr->appDataType = "REAL";
                signalPtr->convMethod="toREAL";
            }else if(signalPtr->name.contains("N_")){
                signalPtr->appDataType = "UINT";
                signalPtr->convMethod="toUINT";
            }else if(signalPtr->name.contains("Z_")){
                signalPtr->appDataType = "WORD";
                signalPtr->convMethod="toWORD";
            }else{
                signalPtr->appDataType="WORD";
                signalPtr->convMethod="toWORD";
                this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");

            }
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali başlangıç biti 16 ve katları değil, düşük performans");
        }
    }else if (signalPtr->length < 32){
        if((signalPtr->startBit == 0 )||(signalPtr->startBit == 8 )||(signalPtr->startBit == 16 )||(signalPtr->startBit == 24 )||(signalPtr->startBit == 32 )){
            signalPtr->comDataType = "DWORD";
            if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
                signalPtr->appDataType = "REAL";
                signalPtr->convMethod="toREAL";
            }else if(signalPtr->name.contains("N_")){
                signalPtr->appDataType = "UDINT";
                signalPtr->convMethod="toUDINT";
            }else if(signalPtr->name.contains("Z_")){
                signalPtr->appDataType = "DWORD";
                signalPtr->convMethod="toDWORD";
            }else{
                signalPtr->appDataType="DWORD";
                signalPtr->convMethod="toDWORD";
                this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");

            }
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali veri boyutu 8 ve katları değil,standart olmayan veri transferi");
        }else{
            signalPtr->comDataType = "DWORD";
            if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
                signalPtr->appDataType = "REAL";
                signalPtr->convMethod="toREAL";
            }else if(signalPtr->name.contains("N_")){
                signalPtr->appDataType = "UDINT";
                signalPtr->convMethod="toUDINT";
            }else if(signalPtr->name.contains("Z_")){
                signalPtr->appDataType = "DWORD";
                signalPtr->convMethod="toDWORD";
            }else{
                signalPtr->appDataType="DWORD";
                signalPtr->convMethod="toDWORD";
                this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");

            }
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali veri boyutu 8 ve katları değil,standart olmayan veri transferi");
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali başlangıç biti 8 ve katları değil,düşük perfomans");
        }

    }else if (signalPtr->length == 32){
        if((signalPtr->startBit == 0 )||(signalPtr->startBit == 8 )||(signalPtr->startBit == 16 )||(signalPtr->startBit == 24 )||(signalPtr->startBit == 32 )){
            signalPtr->comDataType = "DWORD";
            if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
                signalPtr->appDataType = "REAL";
                signalPtr->convMethod="DWORD:REAL";
            }else if(signalPtr->name.contains("N_")){
                signalPtr->appDataType = "UDINT";
                signalPtr->convMethod="DWORD:UDINT";
            }else if(signalPtr->name.contains("Z_")){
                signalPtr->appDataType = "DWORD";
                signalPtr->convMethod="DWORD:DWORD";
            }else{
                signalPtr->appDataType="DWORD";
                signalPtr->convMethod="DWORD:DWORD";
                this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");
            }
        }else{
            signalPtr->comDataType = "DWORD";
            if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
                signalPtr->appDataType = "REAL";
                signalPtr->convMethod="toREAL";
            }else if(signalPtr->name.contains("N_")){
                signalPtr->appDataType = "UDINT";
                signalPtr->convMethod="toUDINT";
            }else if(signalPtr->name.contains("Z_")){
                signalPtr->appDataType = "DWORD";
                signalPtr->convMethod="toDWORD";
            }else{
                signalPtr->appDataType="DWORD";
                signalPtr->convMethod="toDWORD";
                this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");

            }
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali başlangıç biti 8 ve katları değil,düşük performans");
        }
    }else if (signalPtr->length < 64){
        if((signalPtr->startBit == 0 )||(signalPtr->startBit == 8 )||(signalPtr->startBit == 16 )||(signalPtr->startBit == 24 )){
            signalPtr->comDataType = "LWORD";
            if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
                signalPtr->appDataType = "LREAL";
                signalPtr->convMethod="toLREAL";
            }else if(signalPtr->name.contains("N_")){
                signalPtr->appDataType = "ULINT";
                signalPtr->convMethod="toULINT";
            }else if(signalPtr->name.contains("Z_")){
                signalPtr->appDataType = "LWORD";
                signalPtr->convMethod="toLWORD";
            }else{
                signalPtr->appDataType="LWORD";
                signalPtr->convMethod="toLWORD";
                this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");
            }
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali veri boyutu 8 ve katları değil,standart olmayan veri transferi");
        }else{
            signalPtr->comDataType = "LWORD";
            if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
                signalPtr->appDataType = "LREAL";
                signalPtr->convMethod="toLREAL";
            }else if(signalPtr->name.contains("N_")){
                signalPtr->appDataType = "ULINT";
                signalPtr->convMethod="toULINT";
            }else if(signalPtr->name.contains("Z_")){
                signalPtr->appDataType = "LWORD";
                signalPtr->convMethod="toLWORD";
            }else{
                signalPtr->appDataType="LWORD";
                signalPtr->convMethod="toLWORD";
                this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali isimlendirmesinde X_ W_ N_ Z_ işareti bulunmuyor");
            }
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali veri boyutu 8 ve katları değil,standart olmayan veri transferi");
            this->setWarning(this->messageID+" mesajında yer alan "+signalPtr->name+" sinyali başlangıç biti 8 ve katları değil,düşük perfomans");
        }
    }else if (signalPtr->length == 64){
        signalPtr->comDataType = "LWORD";
        if(signalPtr->name.contains("X_") || signalPtr->name.contains("W_")){
            signalPtr->appDataType = "LREAL";
            signalPtr->convMethod="2DWORD:LREAL";
        }else if(signalPtr->name.contains("N_")){
            signalPtr->appDataType = "ULINT";
            signalPtr->convMethod="2DWORD:ULINT";
        }else if(signalPtr->name.contains("Z_")){
            signalPtr->appDataType = "LWORD";
            signalPtr->convMethod="2DWORD:LWORD";
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
