#include "math/evaluate.h"

#include "math/function.h"

#include <charconv>
#include <cmath>
#include <limits>
#include <numbers>

namespace {

constexpr int kMaxDepth = 256;
constexpr qsizetype kMaxNumberLength = 64;
constexpr long double kPercent = 100.0L;
constexpr long double kMaxExactFactorial = 1754.0L;
constexpr long double kInfinity = std::numeric_limits<long double>::infinity();
constexpr long double kNaN = std::numeric_limits<long double>::quiet_NaN();
constexpr QStringView kPrimaryStart = u"0123456789.(πe√";

struct Operand {
    long double value = 0;
    bool percent = false;
};

class Descent
{
public:
    explicit Descent(int &depth) : depth_(depth) { ++depth_; }
    ~Descent() { --depth_; }
    Descent(const Descent &) = delete;
    Descent &operator=(const Descent &) = delete;

private:
    int &depth_;
};

long double factorial(long double x)
{
    if (x != std::floor(x))
        return std::tgamma(x + 1);
    if (x < 0)
        return kNaN;
    if (x > kMaxExactFactorial)
        return kInfinity;
    long double product = 1;
    for (long double factor = 2; factor <= x; ++factor)
        product *= factor;
    return product;
}

class Parser
{
public:
    Parser(QStringView text, Angle angle) : text_(text), angle_(angle) {}

    Evaluation run();

private:
    Operand sum();
    Operand product();
    Operand unary();
    Operand power();
    Operand postfix();
    long double primary();
    long double group();
    long double number();
    long double function();
    long double divide(long double dividend, long double divisor);
    long double raise(long double base, long double exponent);

    QChar peek() const { return pos_ < text_.size() ? text_[pos_] : QChar(); }
    bool accept(QChar c);
    bool startsPrimary() const;
    const Function *functionAt() const;
    void skipDigits();
    void fail(Fault fault);

    QStringView text_;
    Angle angle_;
    qsizetype pos_ = 0;
    int depth_ = 0;
    Fault fault_ = Fault::None;
};

Evaluation Parser::run()
{
    if (text_.isEmpty())
        return {0, Fault::Syntax};
    const long double value = sum().value;
    if (pos_ < text_.size())
        fail(Fault::Syntax);
    if (fault_ != Fault::None)
        return {0, fault_};
    if (std::isnan(value))
        return {0, Fault::Domain};
    if (std::isinf(value))
        return {0, Fault::Overflow};
    return {value, Fault::None};
}

Operand Parser::sum()
{
    Operand lhs = product();
    for (QChar op = peek(); op == u'+' || op == u'-'; op = peek()) {
        ++pos_;
        const Operand rhs = product();
        const long double amount = rhs.percent ? lhs.value * rhs.value : rhs.value;
        lhs = {op == u'+' ? lhs.value + amount : lhs.value - amount, false};
    }
    return lhs;
}

Operand Parser::product()
{
    Operand lhs = unary();
    for (;;) {
        if (accept(u'/')) {
            lhs = {divide(lhs.value, unary().value), false};
            continue;
        }
        if (!accept(u'*') && !startsPrimary())
            return lhs;
        lhs = {lhs.value * unary().value, false};
    }
}

Operand Parser::unary()
{
    const Descent descent(depth_);
    if (depth_ > kMaxDepth) {
        fail(Fault::Syntax);
        return {};
    }
    if (accept(u'+'))
        return unary();
    if (!accept(u'-'))
        return power();
    const Operand operand = unary();
    return {-operand.value, operand.percent};
}

Operand Parser::power()
{
    const Operand base = postfix();
    if (!accept(u'^'))
        return base;
    return {raise(base.value, unary().value), false};
}

Operand Parser::postfix()
{
    Operand operand{primary(), false};
    for (;; ++pos_) {
        switch (peek().unicode()) {
        case u'!':
            operand = {factorial(operand.value), false};
            break;
        case u'%':
            operand = {operand.value / kPercent, true};
            break;
        case u'²':
            operand = {operand.value * operand.value, false};
            break;
        default:
            return operand;
        }
    }
}

long double Parser::primary()
{
    const Descent descent(depth_);
    if (depth_ > kMaxDepth) {
        fail(Fault::Syntax);
        return 0;
    }
    const QChar c = peek();
    if (c.isDigit() || c == u'.')
        return number();
    switch (c.unicode()) {
    case u'(':
        ++pos_;
        return group();
    case u'π':
        ++pos_;
        return std::numbers::pi_v<long double>;
    case u'e':
        ++pos_;
        return std::numbers::e_v<long double>;
    case u'√':
        ++pos_;
        return std::sqrt(primary());
    default:
        return function();
    }
}

long double Parser::group()
{
    const long double value = sum().value;
    accept(u')');
    return value;
}

long double Parser::number()
{
    const qsizetype start = pos_;
    skipDigits();
    if (accept(u'E')) {
        accept(u'-');
        const qsizetype exponent = pos_;
        skipDigits();
        if (pos_ == exponent)
            fail(Fault::Syntax);
    }
    const QStringView digits = text_.sliced(start, pos_ - start);
    if (fault_ != Fault::None || digits.size() > kMaxNumberLength)
        return 0;

    char buffer[kMaxNumberLength];
    const qsizetype length = digits.size();
    for (qsizetype i = 0; i < length; ++i)
        buffer[i] = digits[i].toLatin1();
    long double value = 0;
    const auto [end, error] = std::from_chars(buffer, buffer + length, value);
    if (error != std::errc() || end != buffer + length)
        fail(Fault::Syntax);
    return value;
}

long double Parser::function()
{
    const Function *named = functionAt();
    if (!named) {
        fail(Fault::Syntax);
        return 0;
    }
    pos_ += named->name.size();
    return named->apply(primary(), angle_);
}

long double Parser::divide(long double dividend, long double divisor)
{
    if (divisor == 0)
        fail(Fault::DivideByZero);
    return dividend / divisor;
}

long double Parser::raise(long double base, long double exponent)
{
    if (base == 0 && exponent < 0)
        fail(Fault::DivideByZero);
    return std::pow(base, exponent);
}

bool Parser::accept(QChar c)
{
    if (peek() != c)
        return false;
    ++pos_;
    return true;
}

bool Parser::startsPrimary() const
{
    return kPrimaryStart.contains(peek()) || functionAt();
}

const Function *Parser::functionAt() const
{
    const QStringView rest = text_.sliced(pos_);
    for (const Function &named : functions()) {
        if (rest.startsWith(named.name))
            return &named;
    }
    return nullptr;
}

void Parser::skipDigits()
{
    while (peek().isDigit() || peek() == u'.')
        ++pos_;
}

void Parser::fail(Fault fault)
{
    if (fault_ == Fault::None)
        fault_ = fault;
    pos_ = text_.size();
}

} // namespace

Evaluation evaluate(QStringView expression, Angle angle)
{
    return Parser(expression, angle).run();
}
