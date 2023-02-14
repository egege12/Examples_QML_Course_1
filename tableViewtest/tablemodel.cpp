#include "tablemodel.h"
#include <QtAlgorithms>
unsigned int tablemodel::scolumnID = 1;
QVector<bool> tablemodel::columnNumber ={false,false,false,false,false,false,false,false};
bool tablemodel::variableLessThan(const QList<QString>& a, const QList<QString>& b)
{
    bool isNumber1,isNumber2;
    a.at(tablemodel::scolumnID).toDouble(&isNumber1);
    b.at(tablemodel::scolumnID).toDouble(&isNumber2);
    if(isNumber1 && isNumber2)
        return a.at(tablemodel::scolumnID).toDouble()< b.at(tablemodel::scolumnID).toDouble();
    else
        return a.at(tablemodel::scolumnID)< b.at(tablemodel::scolumnID);
}

bool tablemodel::variableHigherThan(const QList<QString> &a, const QList<QString> &b)
{
    bool isNumber1,isNumber2;
    a.at(tablemodel::scolumnID).toDouble(&isNumber1);
    b.at(tablemodel::scolumnID).toDouble(&isNumber2);
    if(isNumber1 && isNumber2)
        return a.at(tablemodel::scolumnID).toDouble()> b.at(tablemodel::scolumnID).toDouble();
    else
        return a.at(tablemodel::scolumnID)> b.at(tablemodel::scolumnID);
}

tablemodel::tablemodel(QAbstractTableModel *parent)
    : QAbstractTableModel{parent}
{
    table.append({" "," "," "," "});
}

int tablemodel::rowCount(const QModelIndex &) const
{
    return table.size();
}

int tablemodel::columnCount(const QModelIndex &) const
{
    return table.at(0).size();
}

QVariant tablemodel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case TableDataRole:
        return table.at(index.row()).at(index.column());
    case MessageID:
        return table.at(index.row()).at(1);
    case MessageName:
        return table.at(index.row()).at(0);
    case Selected:
        return (table.at(index.row()).at(3)== "X");
    case HeadingRole:
        if (index.row() ==0 ){
                tablemodel::scolumnID=index.column();
                return true;
        }else{
                return false;
        }
        break;

     default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> tablemodel::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[TableDataRole] = "tabledata";
    roles[HeadingRole] = "heading";
    roles[MessageID] = "messageid";
    roles[MessageName] = "messagename";
    roles[Selected] = "selected";
    return roles;
}

QList<QList<QString> > tablemodel::getTable()
{
    return table;
}

void tablemodel::setTable(QList<QList<QString> > table)
{
    beginResetModel();
    this->table.clear();
    qInfo()<<"table appended";
    this->table.append(table);
    endResetModel();
    emit this->tableChanged();
}

void tablemodel::sortColumn()
{
    beginResetModel();
    if(tablemodel::columnNumber.at(tablemodel::scolumnID)){
        std::sort(table.begin()+1,table.end(),variableLessThan);
    }else{
        std::sort(table.begin()+1,table.end(),variableHigherThan);
    }
    tablemodel::columnNumber[tablemodel::scolumnID]= !tablemodel::columnNumber[tablemodel::scolumnID];
    endResetModel();
}
void tablemodel::sortColumnPrivate()
{
    beginResetModel();
    if(tablemodel::columnNumber.at(tablemodel::scolumnID)){
        std::sort(table.begin()+1,table.end(),variableLessThan);
    }else{
        std::sort(table.begin()+1,table.end(),variableHigherThan);
    }
    endResetModel();
}

