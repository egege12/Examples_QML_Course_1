#include "tablemodel.h"

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
    case HeadingRole:
        if (index.row() ==0 ){
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
    emit this->tableChanged();
    endResetModel();
}


