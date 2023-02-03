#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

class tablemodel : public QAbstractTableModel
{
    Q_OBJECT

    QVector<QVector<QString>> table;
public:
    explicit tablemodel(QAbstractTableModel *parent = nullptr);
    int rowCount(const QModelIndex & = QModelIndex()) const override;

    int columnCount(const QModelIndex & = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;
signals:

};

#endif // TABLEMODEL_H
