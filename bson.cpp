#include "bson.h"

#define MAXSIZE 32768

BSON::BSON(QObject *parent) : QObject(parent)
{
}

BSON::~BSON()
{
}

void BSON::insertNULL(QByteArray *result, const QString &k)
{
    result->append( '\x0A' );
    result->append( k.toStdString().c_str(), k.length()+1 );
}

void BSON::parseVariant(const QVariant &v, QByteArray *result, int indents, const QVariant &myname)
{
    QString indentStr = QString(" ").repeated(indents);
    const char *typeName = v.typeName();

    QString keyname = myname.value<QString>();

    if( v.isNull() )
        return insertNULL(result, keyname);

    if( 0 == strcmp( "QVariantMap", typeName ) )
    {
        result->append( '\x03' );
        result->append( keyname.toStdString().c_str(), keyname.length()+1 );

        QByteArray intresult;
        qint32 doclen = 0;
        intresult.append( (const char *)&doclen, sizeof(doclen) );

        QVariantMap asmap = v.value<QVariantMap>();
        foreach( QString k, asmap.keys() )
        {
            //qDebug() << indentStr << "[M]" << k << " => ";
            parseVariant( asmap[k], &intresult, indents+1, k );
        }

        intresult.append( '\x00' );
        intresult.shrink_to_fit();
        doclen = intresult.length();
        intresult.replace( 0, sizeof(doclen), (const char *)&doclen, sizeof(doclen) );
        result->append( intresult );
    }
    else if( 0 == strcmp( "QVariantList", typeName ) )
    {
        result->append( '\x04' );
        result->append( keyname.toStdString().c_str(), keyname.length()+1 );

        QByteArray intresult;
        qint32 doclen = 0;
        intresult.append( (const char *)&doclen, sizeof(doclen) );

        int listidx = 0;
        QVariantList aslist = v.value<QVariantList>();
        foreach( QVariant ent, aslist )
        {
            //qDebug() << indentStr << "[L]" << listidx << " => ";
            parseVariant( ent, &intresult, indents+1, listidx );
            listidx++;
        }

        intresult.append( '\x00' );
        intresult.shrink_to_fit();
        doclen = intresult.length();
        intresult.replace( 0, sizeof(doclen), (const char *)&doclen, sizeof(doclen) );
        result->append( intresult );
    }
    else if( 0 == strcmp( "bool", typeName ) )
    {
        //qDebug() << indentStr << "[B]" << v.value<bool>();
        result->append( '\x08' );
        result->append( keyname.toStdString().c_str(), keyname.length()+1 );
        result->append( v.value<bool>() ? '\x01' : '\x00' );
    }
    else if( 0 == strcmp( "int", typeName ) )
    {
        qint64 asint = v.value<qint64>();
        //qDebug() << indentStr << "[I]" << asint;
        result->append( '\x12' );
        result->append( keyname.toStdString().c_str(), keyname.length()+1 );
        result->append( (const char *)&asint, sizeof(asint) );
    }
    else if( 0 == strcmp( "double", typeName ) )
    {
        double asdouble = v.value<double>();
        //qDebug() << indentStr << "[D]" << asdouble;
        result->append( '\x01' );
        result->append( keyname.toStdString().c_str(), keyname.length()+1 );
        result->append( (const char *)&asdouble, sizeof(asdouble) );
    }
    else if( 0 == strcmp( "QString", typeName ) )
    {
        QString asstr = v.value<QString>();
        if( asstr.isNull() )
            return insertNULL(result, keyname);

        //qDebug() << indentStr << "[S]" << asstr;
        result->append( '\x02' );
        result->append( keyname.toStdString().c_str(), keyname.length()+1 );

        // Special handling for UTF-8 strings:
        qint32 slen = asstr.length()+1;
        result->append( (const char *)&slen, sizeof(slen) );
        result->append( asstr.toStdString().c_str(), slen );
    }
    else if( 0 == strcmp( "QByteArray", typeName ) )
    {
        QByteArray asba = v.value<QByteArray>();
        //qDebug() << indentStr << "[BIN]" << asba.length();
        result->append( '\x05' );
        result->append( keyname.toStdString().c_str(), keyname.length()+1 );

        // Special handling for UTF-8 strings:
        qint32 slen = asba.length();
        result->append( (const char *)&slen, sizeof(slen) );
        result->append( '\x00' );
        result->append( asba.constData(), slen );
    }
    else if( 0 == strcmp( "QDateTime", typeName ) )
    {
        QDateTime asdt = v.value<QDateTime>();
        qint64 asint = asdt.toMSecsSinceEpoch();
        //qDebug() << indentStr << "[I]" << asint;
        result->append( '\x09' );
        result->append( keyname.toStdString().c_str(), keyname.length()+1 );
        result->append( (const char *)&asint, sizeof(asint) );
    }
    else
    {
        qDebug() << indentStr << "(Unknown type:" << typeName << ")";
        return insertNULL(result, keyname);
    }
}

QByteArray BSON::toBSON(const QVariantMap &asmap)
{
    QByteArray result;
    qint32 doclen = 0;
    result.append( (const char *)&doclen, sizeof(doclen) );

    foreach( QString k, asmap.keys() )
    {
        //qDebug() << "[root]" << k << " => ";
        parseVariant( asmap[k], &result, 0, k );
    }

    result.append( '\x00' );
    result.shrink_to_fit();
    doclen = result.length();
    result.replace( 0, sizeof(doclen), (const char *)&doclen, sizeof(doclen) );
    return result;
}

char *BSON::parseBSONValue( QVariantMap *dest, char *cursor, char *end )
{
    quint8 code = cursor[0];
    cursor++;

    QString elemname;
    char *cstrename = cursor;
    elemname.append( cstrename );
    cursor += elemname.length() + 1;

    if( code == '\x01' )
    {
        double val;
        memcpy( &val, cursor, sizeof(double) );
        cursor += sizeof(double);
        dest->insert( elemname, val );
    }

    else if( code == '\x02' )
    {
        qint32 len;
        memcpy( &len, cursor, sizeof(qint32) );
        cursor += sizeof(qint32);

        QByteArray buf;
        buf.append( cursor, len );
        cursor += len;
        dest->insert( elemname, QString::fromUtf8(buf) );
    }
    else if( code == '\x03' )
    {
        // Sanitize:
        QVariantMap intres;
        char *origPos = cursor;
        qint32 doclen;
        memcpy( &doclen, cursor, sizeof(doclen) );
        cursor += sizeof(doclen);

        // Make sure we don't overrun:
        if( origPos+doclen > end )
        {
            qDebug() << "Document is too long for our defined section, so sad.";
            return NULL;
        }

        char *sentinel = origPos + doclen - 1;

        if( sentinel[0] != '\x00' )
        {
            qDebug() << "Failed to find document ending signature.";
            return NULL;
        }

        while( cursor && cursor < sentinel )
        {
            cursor = parseBSONValue( &intres, cursor, sentinel );
            //fprintf(stderr, "Cursor is at %p, and we stop at %p.\n", cursor, origPos + doclen - 1 );
        }

        if( cursor )
            cursor++; // terminating \x00
        dest->insert( elemname, intres );
    }
    else if( code == '\x04' )
    {
        // Sanitize:
        QVariantMap intres;
        char *origPos = cursor;
        qint32 doclen;
        memcpy( &doclen, cursor, sizeof(doclen) );
        cursor += sizeof(doclen);

        // Make sure we don't overrun:
        if( origPos+doclen > end )
        {
            qDebug() << "List is too long for our defined section, so sad.";
            return NULL;
        }

        char *sentinel = origPos + doclen - 1;

        if( sentinel[0] != '\x00' )
        {
            qDebug() << "Failed to find document ending signature.";
            return NULL;
        }

        while( cursor && cursor < sentinel )
        {
            cursor = parseBSONValue( &intres, cursor, sentinel );
            //fprintf(stderr, "Cursor is at %p, and we stop at %p.\n", cursor, origPos + doclen - 1 );
        }

        if( cursor )
            cursor++; // terminating \x00
        dest->insert( elemname, intres.values() );
    }
    else if( code == '\x05' )
    {
        int32_t len;
        memcpy( &len, cursor, sizeof(int32_t) );
        cursor += sizeof(int32_t);

        uint8_t btype;
        memcpy( &btype, cursor, sizeof(uint8_t) );
        cursor++;

        // Make sure we don't overrun:
        if( cursor+len > end )
        {
            qDebug() << "Binary block is too long for our defined section, so sad.";
            return NULL;
        }

        QByteArray result( cursor, len );
        cursor += len;

        dest->insert( elemname, result );
        //qDebug() << "Binary: " << result;
    }
    else if( code == '\x08' )
    {
        uint8_t val;
        memcpy( &val, cursor, sizeof(uint8_t) );
        cursor += sizeof(uint8_t);
        dest->insert( elemname, val == 0 ? false : true );
        //qDebug() << "Bool: " << (val == 0 ? false : true);
    }
    else if( code == '\x09' )
    {
        int64_t val;
        memcpy( &val, cursor, sizeof(int64_t) );
        cursor += sizeof(int64_t);
        QDateTime t = QDateTime::fromMSecsSinceEpoch(val, Qt::UTC);
        dest->insert( elemname, t );
        //qDebug() << "Datetime: " << t;
    }
    else if( code == '\x0A' )
    {
        //qDebug() << "Null";
        dest->insert( elemname, QVariant() );
    }
    else if( code == '\x12' )
    {
        qint64 val;
        memcpy( &val, cursor, sizeof(qint64) );
        cursor += sizeof(qint64);
        dest->insert( elemname, val );
        //qDebug() << "Int64: " << val;
    }
    return cursor;
}

QVariantMap BSON::fromBSON(const QByteArray &ba)
{
    QVariantMap result;

    char *cursor = (char*)ba.data();
    // Sanitize:
    qint32 doclen;
    memcpy( &doclen, ba.constData(), sizeof(doclen) );
    cursor += sizeof(doclen);

    if( doclen > MAXSIZE || ba.length() > MAXSIZE )
    {
        qDebug() << "Packet length specified to be" << doclen << "which is larger than the allowed" << MAXSIZE;
        return result;
    }

    if( ba.length() != doclen )
    {
        qDebug() << "Size mismatch: Expected" << doclen << "but packet is" << ba.length() << "bytes.";
        return result;
    }

    char *sentinel = ((char*)ba.constData()) + doclen - 1;
    if( ba.length() < (sentinel-cursor) )
    {
        qDebug() << "Size mismatch: Expected" << (sentinel-cursor) << "but packet is" << ba.length() << "bytes.";
        return result;
    }

    if( ba.at( doclen-1 ) != '\x00' )
    {
        qDebug() << "Failed to find document ending signature.";
        return result;
    }

    while( cursor && cursor < sentinel )
    {
        cursor = parseBSONValue( &result, cursor, sentinel );
        //fprintf(stderr, "Cursor is at %p, and we stop at %p.\n", cursor, ba.constData() + doclen - 1 );
    }

    return result;
}
