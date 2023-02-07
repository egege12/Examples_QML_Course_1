#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

class tablemodel : public QAbstractTableModel
{

    enum TableRoles{
        TableDataRole = Qt::UserRole+1,
        HeadingRole
    };
public:

Q_INVOKABLE void setTable(const QVariant& value)
        { //inline for briefity
            beginResetModel();
            table = value.value<QVector<QVector<QString>>>();
            endResetModel();
        }


     QList<QList<QString>> table;
    explicit tablemodel(QAbstractTableModel *parent = nullptr);

    int rowCount(const QModelIndex & = QModelIndex()) const override;

    int columnCount(const QModelIndex & = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;
public slots:

};

#endif // TABLEMODEL_H
