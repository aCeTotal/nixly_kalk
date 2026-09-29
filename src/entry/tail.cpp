#include "entry/tail.h"

#include "math/function.h"

#include <algorithm>

namespace {

constexpr QChar kRoot = u'√';

bool isNumberChar(QStringView text, qsizetype i)
{
    const QChar c = text[i];
    if (c == u'-')
        return i > 0 && text[i - 1] == u'E';
    return c.isDigit() || c == u'.' || c == u'E';
}

} // namespace

qsizetype numberStart(QStringView text)
{
    qsizetype start = text.size();
    while (start > 0 && isNumberChar(text, start - 1))
        --start;
    return start;
}

qsizetype mantissaDigits(QStringView number)
{
    const QStringView mantissa = number.left(number.indexOf(u'E'));
    return std::ranges::count_if(mantissa, [](QChar c) { return c.isDigit(); });
}

qsizetype unitLength(QStringView text)
{
    if (!text.endsWith(u'('))
        return 1;
    const QStringView head = text.chopped(1);
    if (head.endsWith(kRoot))
        return 2;
    qsizetype longest = 0;
    for (const Function &named : functions()) {
        if (head.endsWith(named.name))
            longest = std::max(longest, named.name.size());
    }
    return longest + 1;
}
