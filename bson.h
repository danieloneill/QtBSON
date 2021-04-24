#ifndef BSON_H
#define BSON_H

#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QList>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class BSON : public QObject
{
    Q_OBJECT

public:
    explicit BSON(QObject *parent = nullptr);
    ~BSON();

public:
    Q_INVOKABLE QByteArray toBSON(const QVariantMap &value);
    Q_INVOKABLE QVariantMap fromBSON(const QByteArray &buffer);

protected:
    void insertNULL(QByteArray *result, const QString &k);
    void parseVariant(const QVariant &v, QByteArray *result, int indents, const QVariant &myname);
    char *parseBSONValue( QVariantMap *dest, char *cursor, char *end );
};

#endif // BSON_H
