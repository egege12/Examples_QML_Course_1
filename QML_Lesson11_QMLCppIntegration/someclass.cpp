#include "someclass.h"

SomeClass::SomeClass(QObject *parent)
    : QObject{parent}
{

}

void SomeClass::callMe()
{
    qInfo()<<"call me called";
}
