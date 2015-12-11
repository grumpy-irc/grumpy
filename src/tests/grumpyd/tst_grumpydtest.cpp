#include <QString>
#include <QtTest>

class GrumpydTest : public QObject
{
        Q_OBJECT

    public:
        GrumpydTest();

    private Q_SLOTS:
        void initTestCase();
        void cleanupTestCase();
        void testCase1_data();
        void testCase1();
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

void GrumpydTest::testCase1_data()
{
    QTest::addColumn<QString>("data");
    QTest::newRow("0") << QString();
}

void GrumpydTest::testCase1()
{
    QFETCH(QString, data);
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(GrumpydTest)

#include "tst_grumpydtest.moc"
