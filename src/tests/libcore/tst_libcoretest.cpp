#include <QString>
#include <QtTest>
#include "../../libcore/exception.h"
#include "../../libcore/configuration.h"
#include "../../libcore/commandprocessor.h"
#include "../../libcore/generic.h"

class LibcoreTest : public QObject
{
        Q_OBJECT

    public:
        LibcoreTest();

    private Q_SLOTS:
        void initTestCase();
        void cleanupTestCase();
        void testCaseGenericBool();
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

void LibcoreTest::testCaseGenericBool()
{
    QVERIFY2(GrumpyIRC::Generic::Bool2String(true).toLower() == QString("true"), "Check results of bool 2 string");
    QVERIFY2(GrumpyIRC::Generic::Bool2String(false).toLower() == QString("false"), "Check results of bool 2 string");
    QVERIFY2(GrumpyIRC::Generic::String2Bool("TRUE") == true, "Check results of string 2 bool");
    QVERIFY2(GrumpyIRC::Generic::String2Bool("fucking string") == false, "Check results of string 2 bool");
    QVERIFY2(GrumpyIRC::Generic::String2Bool("fucking string", true) == true, "Check results of string 2 bool");
    QVERIFY2(GrumpyIRC::Generic::String2Bool("fucking string") == false, "Check results of string 2 bool");
    QVERIFY2(GrumpyIRC::Generic::String2Bool("true") == true, "Check results of string 2 bool");
    QVERIFY2(GrumpyIRC::Generic::String2Bool("false") == false, "Check results of string 2 bool");
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(LibcoreTest)

#include "tst_libcoretest.moc"
