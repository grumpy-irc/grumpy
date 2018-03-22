#include <QString>
#include <QtTest>
#include "../../libirc/libircclient/channel.h"
#include "../../libirc/libircclient/mode.h"

class LibircclientTest : public QObject
{
        Q_OBJECT

    public:
        LibircclientTest();

    private Q_SLOTS:
        void initTestCase();
        void cleanupTestCase();
        void testCaseModeComparisons();
        void testCaseChannelModes();
};

LibircclientTest::LibircclientTest()
{
}

void LibircclientTest::initTestCase()
{
}

void LibircclientTest::cleanupTestCase()
{
}

void LibircclientTest::testCaseChannelModes()
{
    libircclient::Channel channel("#support");
    QVERIFY2(channel.GetBans().count() == 0, "Bans after init");
    channel.SetMode("+ntr");
    QVERIFY2(channel.GetMode().Includes('r'), "Check if modes +r was applied or not");
    channel.SetMode("-r");
    QVERIFY2(!channel.GetMode().Includes('r'), "Check if modes -r was applied OK");

    // Exceptions and bans
    libircclient::ChannelPMode pmode1("b"); // test ban 1
    pmode1.Parameter = "*!*@test";
    channel.SetPMode(pmode1);
    QVERIFY2(channel.GetBans().count() == 1, "Bans after set");
    QVERIFY2(channel.GetExceptions().count() == 0, "Exceptions after ban set");

    // Set another ban
    libircclient::ChannelPMode pmode2("b"); // test ban 2
    pmode2.Parameter = "*!*@test2";
    channel.SetPMode(pmode2);
    QVERIFY2(channel.GetBans().count() == 2, "Bans after set");

    // More
    libircclient::ChannelPMode pmode3("b"); // test ban 2
    pmode3.Parameter = "*!*@test2";
    channel.SetPMode(pmode3);
    QVERIFY2(channel.GetBans().count() == 2, "Bans after set of existing ban");
    libircclient::ChannelPMode pmode4("b"); // test ban 1
    pmode4.Parameter = "*!*@test";
    libircclient::ChannelPMode pmode5("b"); // test ban 2
    pmode5.Parameter = "*!*@test2";

    // Remove existing ban
    channel.RemovePMode(pmode4);
    QVERIFY2(channel.GetBans().count() == 1, "Bans after removal of existing ban");
    QVERIFY2(true, "Failure");
}

void LibircclientTest::testCaseModeComparisons()
{
    libircclient::ChannelPMode pmode1("b"); // test ban 1
    pmode1.Parameter = "*!*@test";
    libircclient::ChannelPMode pmode2("b"); // test ban 1
    pmode2.Parameter = "*!*@test";
    libircclient::ChannelPMode pmode3("e"); // test ban 1
    pmode3.Parameter = "*!*@test";
    libircclient::ChannelPMode pmode4("-e"); // test ban 1
    pmode4.Parameter = "*!*@test";
    QVERIFY2(pmode1 == pmode2, "pmode1 == pmode2");
    QVERIFY2(pmode1 != pmode3, "pmode1 != pmode3");
}

QTEST_APPLESS_MAIN(LibircclientTest)

#include "tst_libirctest.moc"
