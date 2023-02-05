#include "tablemodel.h"
#include "aschandler.h"
tablemodel::tablemodel(QAbstractTableModel *parent)
    : QAbstractTableModel{parent}
{
    ASCHandler messageTable(nullptr,"C:/Users/egementurker/Desktop/PR1003_DBC_ECU_Interface_v33.dbc");

}

int tablemodel::rowCount(const QModelIndex &) const
{
    return 200;
}

int tablemodel::columnCount(const QModelIndex &) const
{
    return 200;
}

QVariant tablemodel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        return QString("%1, %2").arg(index.column()).arg(index.row());
    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> tablemodel::roleNames() const
{
    return { {Qt::DisplayRole, "display"} };
}
