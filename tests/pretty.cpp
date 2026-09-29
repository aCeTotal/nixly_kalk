#include "check.h"

#include "text/pretty.h"

#include <array>

namespace {

struct Case {
    QStringView expression;
    QStringView shown;
};

const Notation kNorwegian{QStringLiteral(","), QStringLiteral(" ")};
const Notation kEnglish{QStringLiteral("."), QStringLiteral(",")};

constexpr std::array kNorwegianCases{
    Case{u"1234567.891", u"1 234 567,891"},
    Case{u"123", u"123"},
    Case{u"1234", u"1 234"},
    Case{u"1+2", u"1 + 2"},
    Case{u"-5-3", u"−5 − 3"},
    Case{u"5*-3", u"5 × −3"},
    Case{u"8/(+2)", u"8 ÷ (+2)"},
    Case{u"2^-1", u"2^−1"},
    Case{u"1.5E20", u"1,5×10²⁰"},
    Case{u"2E-7-1", u"2×10⁻⁷ − 1"},
    Case{u"asin(1)+acos(atan(0", u"sin⁻¹(1) + cos⁻¹(tan⁻¹(0"},
    Case{u"√(2)π!%²e", u"√(2)π!%²e"},
    Case{u"", u""},
};

} // namespace

int main()
{
    for (const Case &c : kNorwegianCases)
        check(prettify(c.expression, kNorwegian) == c.shown, qPrintable(c.expression.toString()));
    check(prettify(u"12345.5", kEnglish) == u"12,345.5", "english grouping");
    return failures == 0 ? 0 : 1;
}
