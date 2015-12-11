#include <QString>
#include <QtTest>

class LibcoreTest : public QObject
{
        Q_OBJECT

    public:
        LibcoreTest();

    private Q_SLOTS:
        void initTestCase();
        void cleanupTestCase();
        void testCase1();
};

LibcoreTest::LibcoreTest()
{
}

void LibcoreTest::initTestCase()
{
}

void LibcoreTest::cleanupTestCase()
{
}

void LibcoreTest::testCase1()
{
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(LibcoreTest)

#include "tst_libcoretest.moc"
