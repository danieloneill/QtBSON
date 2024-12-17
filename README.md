# QtBSON
Quick and dirty implementation to serialise QVariantMap into BSON and vice-versa

Some caveats:
 * The only top-level container allowed is a QVariantMap in either direction.
 * Supported types are QVariantMap, QVariantList, Null (undefined QVariant or QString), bool, int, double, QString, QByteArray, and QDateTime. Other types are generally unimplemented. (QJSValue support exists, but is masked by "#ifdef", if you need it. It just bullies the value into a string.)

A test application is included which demonstrates how the class is interacted with.

Enjoy.

