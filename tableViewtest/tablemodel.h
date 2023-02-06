#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

class tablemodel : public QAbstractTableModel
{
    Q_OBJECT

    enum TableRoles{
        TableDataRole = Qt::UserRole+1,
        HeadingRole
    };

    QList<QList<QString>> table;


public:
    explicit tablemodel(QAbstractTableModel *parent = nullptr);

    int rowCount(const QModelIndex & = QModelIndex()) const override;

    int columnCount(const QModelIndex & = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    void generateTable(QList<QList<QString>> messages);
    void updateTable();
};

#endif // TABLEMODEL_H
