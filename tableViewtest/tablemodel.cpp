#include "tablemodel.h"
#include "DBChandler.h"
tablemodel::tablemodel(QAbstractTableModel *parent)
    : QAbstractTableModel{parent}
{
    interface.readFile("C:/Users/ege-t/Desktop/diagnose_dbc_v.1.9.dbc");

    connect(&timer, SIGNAL(timeout()), this, SLOT(updateTable()),Qt::QueuedConnection);
    timer.setInterval(1000);
    timer.start(1000);
    table = interface.messagesVector();

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
    return roles;
}

void tablemodel::updateTable()
{
    table = interface.messagesVector();
}
