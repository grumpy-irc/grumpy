#include <QString>
#include <QtTest>
#include "../../libirc2htmlcode/parser.h"

class Irc2HtmlCodeTest : public QObject
{
        Q_OBJECT

    private:
        QString render(const QString &text);

    private Q_SLOTS:
        void testCaseEscapesPlainText();
        void testCaseHyperlinks();
        void testCaseHyperlinkEscapedDelimiter();
        void testCaseChannels();
        void testCaseColors();
        void testCaseBrokenColorSyntax();
        void testCaseTextStyles();
        void testCaseNestedStyles();
};

QString Irc2HtmlCodeTest::render(const QString &text)
{
    irc2htmlcode::Parser parser;
    return parser.Process("$string", QDateTime(), QString(), text).source;
}

void Irc2HtmlCodeTest::testCaseEscapesPlainText()
{
    QCOMPARE(render("hello <&>\"'  world"),
             QString("<font color=\"#FFFFFF\">hello &lt;&amp;&gt;&quot;&#39;&nbsp;&nbsp;world</font>"));
}

void Irc2HtmlCodeTest::testCaseHyperlinks()
{
    QCOMPARE(render("visit http://example.org/path?q=1&v=2."),
             QString("<font color=\"#FFFFFF\">visit <a style=\"color:#94D7F2;\" href=\"http://example.org/path?q=1&amp;v=2\">http://example.org/path?q=1&amp;v=2</a>.</font>"));
}

void Irc2HtmlCodeTest::testCaseHyperlinkEscapedDelimiter()
{
    QCOMPARE(render("http://x/\" onclick=bad"),
             QString("<font color=\"#FFFFFF\"><a style=\"color:#94D7F2;\" href=\"http://x/\">http://x/</a>&quot; onclick=bad</font>"));
}

void Irc2HtmlCodeTest::testCaseChannels()
{
    QCOMPARE(render("join #grumpy, not foo#bar"),
             QString("<font color=\"#FFFFFF\">join <a style=\"color:#94D7F2;\" href=\"irc_join://#grumpy\">#grumpy</a>, not foo#bar</font>"));
}

void Irc2HtmlCodeTest::testCaseColors()
{
    QCOMPARE(render(QString("a ") + QChar(3) + "04red"),
             QString("<font color=\"#FFFFFF\">a <font color=\"#FFC4CE\">red</font></font>"));

    QCOMPARE(render(QString("a ") + QChar(3) + "04,02red"),
             QString("<font color=\"#FFFFFF\">a <font color=\"#FFC4CE\">red</font></font>"));
}

void Irc2HtmlCodeTest::testCaseBrokenColorSyntax()
{
    QCOMPARE(render(QString("a ") + QChar(3) + "x"),
             QString("<font color=\"#FFFFFF\">a x</font>"));
}

void Irc2HtmlCodeTest::testCaseTextStyles()
{
    QCOMPARE(render(QString("a ") + QChar(2) + "bold" + QChar(2) + " c"),
             QString("<font color=\"#FFFFFF\">a <b>bold</b> c</font>"));

    QCOMPARE(render(QString("a ") + QChar(31) + "under" + QChar(31)),
             QString("<font color=\"#FFFFFF\">a <u>under</u></font>"));

    QCOMPARE(render(QString("a ") + QChar(29) + "italic" + QChar(29)),
             QString("<font color=\"#FFFFFF\">a <i>italic</i></font>"));

    QCOMPARE(render(QString("a ") + QChar(2) + "dangling"),
             QString("<font color=\"#FFFFFF\">a <b>dangling</b></font>"));
}

void Irc2HtmlCodeTest::testCaseNestedStyles()
{
    QCOMPARE(render(QString("a ") + QChar(2) + "b" + QChar(3) + "04red" + QChar(15) + "x"),
             QString("<font color=\"#FFFFFF\">a <b>b</b><b><font color=\"#FFC4CE\">red</font></b>x</font>"));
}

QTEST_GUILESS_MAIN(Irc2HtmlCodeTest)

#include "tst_irc2htmlcodetest.moc"
