#include "text/phrase.h"
#include "window/window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    /* Custom painted, styles unused. */
    qunsetenv("QT_STYLE_OVERRIDE");
    /* Uses no portals. */
    qputenv("QT_NO_XDG_DESKTOP_PORTAL", "1");
    QApplication app(argc, argv);
    app.setApplicationName("nixlykalk");
    app.setApplicationDisplayName(phrase(Phrase::Title).toString());
    app.setApplicationVersion("1.0");
    app.setDesktopFileName("nixlykalk");

    Window window;
    window.show();
    return app.exec();
}
