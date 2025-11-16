#include "mainwindow.h"
#include <KActionCollection>
#include <KLocalizedString>
#include <KStandardAction>
#include <KTextEdit>
#include <QAction>
#include <QApplication>
#include <kstandardaction.h>
#include <ktextedit.h>
#include <qcoreapplication.h>
#include <qnamespace.h>

MainWindow::MainWindow(QWidget* parent)
    : KXmlGuiWindow(parent)
{
    textArea = new KTextEdit();
    setCentralWidget(textArea);
    setupActions();
}

void MainWindow::setupActions()
{
    using namespace Qt::Literals::StringLiterals;

    QAction* clearAction = new QAction(this);
    clearAction->setText(i18n("&Clear"));
    clearAction->setIcon(QIcon::fromTheme(u"document-new-symbolic"_s));

    actionCollection()->addAction(u"clear"_s, clearAction);
    actionCollection()->setDefaultShortcut(clearAction, Qt::CTRL | Qt::Key_L);
    connect(clearAction, &QAction::triggered, textArea, &KTextEdit::clear);

    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());

    setupGUI(Default, u"texteditorrui.rc"_s);
}
