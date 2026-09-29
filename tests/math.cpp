#include "check.h"

#include "math/evaluate.h"
#include "math/number.h"

#include <array>

namespace {

struct Case {
    QStringView expression;
    QStringView shown;
};

struct FaultCase {
    QStringView expression;
    Fault fault;
};

constexpr std::array kDegrees{
    Case{u"1+2*3", u"7"},
    Case{u"(1+2)*3", u"9"},
    Case{u"2^3^2", u"512"},
    Case{u"2^-1", u"0.5"},
    Case{u"-2^2", u"-4"},
    Case{u"2π", u"6.283185307179586"},
    Case{u"3(4+5)", u"27"},
    Case{u"(1+2)(3+4)", u"21"},
    Case{u"2sin(30)", u"1"},
    Case{u"2√(9)", u"6"},
    Case{u"√9", u"3"},
    Case{u"sin(30)²", u"0.25"},
    Case{u"sin(30", u"0.5"},
    Case{u"2*(3+4", u"14"},
    Case{u"50+10%", u"55"},
    Case{u"50-10%", u"45"},
    Case{u"200*10%", u"20"},
    Case{u"10%", u"0.1"},
    Case{u".5+5.", u"5.5"},
    Case{u"1.5E20", u"1.5E20"},
    Case{u"2E-7*1", u"0.0000002"},
    Case{u"5!", u"120"},
    Case{u"0.5!", u"0.886226925452758"},
    Case{u"sin(180)", u"0"},
    Case{u"cos(90)", u"0"},
    Case{u"tan(45)", u"1"},
    Case{u"asin(1)", u"90"},
    Case{u"ln(e)", u"1"},
    Case{u"log(1000)", u"3"},
    Case{u"0.1+0.2", u"0.3"},
    Case{u"1/3", u"0.3333333333333333"},
    Case{u"2/3", u"0.6666666666666667"},
    Case{u"1000000000000000", u"1000000000000000"},
    Case{u"10000000000000000", u"1E16"},
    Case{u"0.000000001", u"0.000000001"},
    Case{u"0.00000000012", u"1.2E-10"},
    Case{u"-3*-3", u"9"},
};

constexpr std::array kRadians{
    Case{u"sin(π)", u"0"},
    Case{u"cos(π)", u"-1"},
    Case{u"asin(1)", u"1.570796326794897"},
};

constexpr std::array kFaults{
    FaultCase{u"", Fault::Syntax},
    FaultCase{u"2)", Fault::Syntax},
    FaultCase{u"1.2.3", Fault::Syntax},
    FaultCase{u".", Fault::Syntax},
    FaultCase{u"2+", Fault::Syntax},
    FaultCase{u"sin", Fault::Syntax},
    FaultCase{u"1E", Fault::Syntax},
    FaultCase{u"5/0", Fault::DivideByZero},
    FaultCase{u"0/0", Fault::DivideByZero},
    FaultCase{u"0^-1", Fault::DivideByZero},
    FaultCase{u"(-3)!", Fault::Domain},
    FaultCase{u"√(-1)", Fault::Domain},
    FaultCase{u"ln(0)", Fault::Domain},
    FaultCase{u"log(-1)", Fault::Domain},
    FaultCase{u"asin(2)", Fault::Domain},
    FaultCase{u"tan(90)", Fault::Domain},
    FaultCase{u"1800!", Fault::Overflow},
    FaultCase{u"10^5000", Fault::Overflow},
};

void expectShown(const Case &c, Angle angle)
{
    const Evaluation outcome = evaluate(c.expression, angle);
    const bool ok = outcome.fault == Fault::None && formatNumber(outcome.value) == c.shown;
    check(ok, qPrintable(c.expression.toString()));
}

} // namespace

int main()
{
    for (const Case &c : kDegrees)
        expectShown(c, Angle::Degrees);
    for (const Case &c : kRadians)
        expectShown(c, Angle::Radians);
    for (const FaultCase &c : kFaults)
        check(evaluate(c.expression, Angle::Degrees).fault == c.fault, qPrintable(c.expression.toString()));

    check(evaluate(QString(300, u'('), Angle::Degrees).fault == Fault::Syntax, "deep nesting");
    check(formatNumber(-0.0L) == u"0", "negative zero");
    check(formatNumber(-1.5e-12L) == u"-1.5E-12", "negative tiny");
    return failures == 0 ? 0 : 1;
}
