#ifndef BSON_H
#define BSON_H

#include <QByteArray>
#include <QJsonParseError>
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

    enum Error { NoError, TooLarge, LengthMismatch, ContentTruncated };
    Q_ENUM(Error)

    Q_INVOKABLE QByteArray toBSON(const QVariantMap &value);
    Q_INVOKABLE QVariantMap fromBSON(const QByteArray &buffer, BSON::Error *errPtr=nullptr);

protected:
    void insertNULL(QByteArray *result, const QString &k);
    void parseVariant(const QVariant &v, QByteArray *result, int indents, const QVariant &myname);
    char *parseBSONValue( QVariantMap *dest, char *cursor, char *end, BSON::Error *errPtr=nullptr );
};

#endif // BSON_H
