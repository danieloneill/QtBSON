# QtBSON
Quick and dirty implementation to serialise QVariantMap into BSON and vice-versa

Some caveats:
 * It was written in a couple hours and really isn't polished at all.
 * The only top-level container allowed is a QVariantMap in either direction.
 * Supported types are QVariantMap, QVariantList, Null (undefined QVariant or QString), bool, int, double, QString, QByteArray, and QDateTime. Other types are unimplemented.

A test application is included which demonstrates how the class is interacted with.

Enjoy.

