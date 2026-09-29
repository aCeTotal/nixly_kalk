#include "text/notation.h"

#include <QLocale>

const Notation &systemNotation()
{
    static const Notation notation = [] {
        const QLocale locale = QLocale::system();
        return Notation{locale.decimalPoint(), locale.groupSeparator()};
    }();
    return notation;
}
