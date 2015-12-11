#include <QString>
#include <QDebug>
#include <QtTest>
#include "../../libcore/sqlite.h"

class GrumpydTest : public QObject
{
        Q_OBJECT

    public:
        GrumpydTest();

    private Q_SLOTS:
        void initTestCase();
        void cleanupTestCase();
        void testCaseTestSQL();
};

GrumpydTest::GrumpydTest()
{

}

void GrumpydTest::initTestCase()
{

}

void GrumpydTest::cleanupTestCase()
{

}

void GrumpydTest::testCaseTestSQL()
{
#ifndef GRUMPY_SQLITE
    qDebug() << "Sqlite support is not enabled";
#else
    qDebug() << "Trying to load the sql lite database";
    GrumpyIRC::SQLite sqlite(":memory:");
    qDebug() << "Trying to load the install.sql script";
    QFile file(":/sql/install.sql");
    if (!file.open(QIODevice::ReadOnly))
    {
        QVERIFY2(false, "Failed to load the install.sql script");
        return;
    }
    QString input = QString(file.readAll());
    QVERIFY2(sqlite.ExecuteNonQuery(input), "Loading install.sql to sqlite");
#endif
}

QTEST_APPLESS_MAIN(GrumpydTest)

#include "tst_grumpydtest.moc"
