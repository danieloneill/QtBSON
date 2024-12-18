#include <QDateTime>
#include <QDebug>
#include <QFile>

#include "bson.h"

void runtests()
{
    BSON bson;

    QByteArray bindata;
    bindata.append('\x01');
    bindata.append('\x02');
    bindata.append('\x03');
    bindata.append('\x04');
    bindata.append('\x05');

    QVariantMap lsent;
    lsent.insert("DateTime", QDateTime::currentDateTimeUtc());
    lsent.insert("bindata", bindata);

    QVariantList l;
    l.append(QVariant());
    l.append(QString());
    l.append("3rd element");
    l.append(lsent);

    QVariantMap m;
    m.insert("doubleval", (double)1.23);
    m.insert("intval", 5);
    m.insert("boolval1", true);
    m.insert("boolval2", false);
    m.insert("stringval", "I am a string.");
    m.insert("list", l);

    qDebug() << m;
    QByteArray result = bson.toBSON(m);

    QFile f("test.bson");
    f.open(QIODevice::ReadWrite|QIODevice::Truncate);
    f.write(result);
    f.close();

    BSON::Error err;

    QVariantMap rebuilt = bson.fromBSON(result, &err);
    qDebug() << err;
    qDebug() << rebuilt;
}

int main(int argc, char **argv)
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)

    runtests();

    return 0;
}

