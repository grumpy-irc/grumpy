#include <QString>
#include <QCoreApplication>
#include <QtTest>
#include <QJSEngine>
#include <QJSValue>
#include "../../libcore/exception.h"
#include "../../libcore/configuration.h"
#include "../../libcore/commandprocessor.h"
#include "../../libcore/generic.h"
#include "../../libcore/resources.h"

class LibcoreTest : public QObject
{
        Q_OBJECT

    public:
        LibcoreTest();

    protected:
        void syntaxCheck(QString path);

    private Q_SLOTS:
        void initTestCase();
        void cleanupTestCase();
        void testCaseGenericBool();
        void testCaseEmbeddedScriptSyntaxCheck();
};

LibcoreTest::LibcoreTest()
{
    QCoreApplication::setApplicationName("grumpy_test");
}

void LibcoreTest::syntaxCheck(QString path)
{
    QJSEngine syntax_check;
    QJSValue result = syntax_check.evaluate(GrumpyIRC::Resources::GetSource(path), path);
    if (result.isError())
    {
        qWarning() << path + ": l" + result.property("lineNumber").toString() + " c" +
                      result.property("columnNumber").toString() + ": " + result.toString();
    }
    QVERIFY2(!result.isError(), "syntax check");
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

void LibcoreTest::testCaseEmbeddedScriptSyntaxCheck()
{
    syntaxCheck("/grumpy_core/ecma/libirc.js");
    syntaxCheck("/grumpy_core/ecma/grumpy.js");
}

QTEST_GUILESS_MAIN(LibcoreTest)

#include "tst_libcoretest.moc"
