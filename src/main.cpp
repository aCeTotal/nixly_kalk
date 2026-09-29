#include "text/phrase.h"
#include "window/window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("nixlykalk");
    app.setApplicationDisplayName(phrase(Phrase::Title).toString());
    app.setApplicationVersion("1.0");
    app.setDesktopFileName("nixlykalk");

    Window window;
    window.show();
    return app.exec();
}
