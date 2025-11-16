#include "mainwindow.h"
#include <KAboutData>
#include <KLazyLocalizedString>
#include <KMessageBox>
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char* argv[])
{
    using namespace Qt::Literals::StringLiterals;

    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("texteditor");

    KAboutData aboutData(
        u"texteditor"_s,
        i18n("text editor"),
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
    aboutData.processCommandLine(&parser); // now can use --author, --version etc as cmd line args.. interesting

    MainWindow* window = new MainWindow();
    window->show();

    return app.exec();
}
