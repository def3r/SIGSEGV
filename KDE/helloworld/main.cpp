#include <KAboutData>
#include <KLazyLocalizedString>
#include <KMessageBox>
#include <QApplication>
#include <QCommandLineParser>
#include <cstdlib>
#include <kaboutdata.h>
#include <klazylocalizedstring.h>
#include <kmessagebox.h>
#include <kstandardguiitem.h>
#include <qcommandlineparser.h>
#include <qstringliteral.h>

int main(int argc, char* argv[])
{
    using namespace Qt::Literals::StringLiterals;

    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("tutorial1");

    KAboutData aboutData(
        u"helloworld"_s,
        i18n("Hello World tutorial"),
        u"1.0"_s,
        i18n("Displays a KMessageBox popup"),
        KAboutLicense::GPL,
        i18n("(c) 1699"),
        i18n("ohno"),
        u"def3r.in/"_s,
        u"contact@def3r.in"_s);

    aboutData.addAuthor(
        i18n("def3r"),
        i18n("KDE Moment"),
        u"contact.def3r.in"_s,
        u"def3r.in/"_s,
        u"def3r"_s);

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser); // now can use --author, --version etc as cmd line args

    KGuiItem primaryAction(
        QStringLiteral("HELLO"), QString(),
        QStringLiteral("This is a tooltip"),
        QStringLiteral("This is a WhatsThis help text."));

    auto messageBox = KMessageBox::questionTwoActions(
        nullptr,
        i18n("Hello World"),
        i18n("Hello Title"),
        primaryAction,
        KStandardGuiItem::cancel());

    if (messageBox == KMessageBox::PrimaryAction) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
