#include "entry/sanitize.h"

namespace {

constexpr QStringView kAlphabet = u"0123456789.+-*/^()!%²π√eEsincotalg";

QChar canonical(QChar c, const Notation &notation)
{
    if (notation.decimal == c)
        return u'.';
    if (notation.group == c || c.isSpace())
        return {};
    switch (c.unicode()) {
    case u'×':
        return u'*';
    case u'÷':
        return u'/';
    case u'−':
        return u'-';
    default:
        return kAlphabet.contains(c) ? c : QChar();
    }
}

} // namespace

QString sanitize(QStringView text, const Notation &notation)
{
    QString clean;
    clean.reserve(text.size());
    for (const QChar c : text) {
        const QChar mapped = canonical(c, notation);
        if (!mapped.isNull())
            clean += mapped;
    }
    return clean;
}
