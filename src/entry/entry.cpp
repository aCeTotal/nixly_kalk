#include "entry/entry.h"

#include "entry/sanitize.h"
#include "entry/tail.h"
#include "math/number.h"

#include <algorithm>

namespace {

constexpr qsizetype kMaxDigits = 16;
constexpr qsizetype kMaxLength = 256;
constexpr QStringView kOperators = u"+-*/^";
constexpr QStringView kUnaryMinusAfter = u"*/^";
constexpr QStringView kOperandEnd = u"0123456789.)πe²!%";

bool isOperator(QChar c)
{
    return kOperators.contains(c);
}

} // namespace

bool Entry::apply(Keystroke stroke)
{
    const bool accepted = route(stroke);
    refresh();
    return accepted;
}

void Entry::paste(QStringView text, const Notation &notation)
{
    const QString clean = sanitize(text, notation);
    if (clean.isEmpty())
        return;
    if (phase_ != Phase::Editing)
        reset();
    text_ += clean;
    text_.truncate(kMaxLength);
    refresh();
}

void Entry::setAngle(Angle angle)
{
    angle_ = angle;
    refresh();
}

int Entry::openParens() const
{
    const qsizetype depth = text_.count(u'(') - text_.count(u')');
    return static_cast<int>(std::max<qsizetype>(0, depth));
}

bool Entry::route(Keystroke stroke)
{
    switch (phase_) {
    case Phase::Result:
        return resume(stroke);
    case Phase::Failed:
        return recover(stroke);
    case Phase::Editing:
        break;
    }
    return edit(stroke);
}

bool Entry::resume(Keystroke stroke)
{
    switch (stroke.command) {
    case Command::Digit:
    case Command::Point:
    case Command::Constant:
    case Command::Open:
    case Command::Raise:
        text_.clear();
        break;
    case Command::Function:
        text_.prepend(u'(');
        text_.prepend(stroke.token);
        text_ += u')';
        phase_ = Phase::Editing;
        return true;
    case Command::Close:
        return false;
    case Command::Equals:
        return true;
    case Command::Angle:
        return dispatch(stroke);
    default:
        break;
    }
    phase_ = Phase::Editing;
    return edit(stroke);
}

bool Entry::recover(Keystroke stroke)
{
    reset();
    if (stroke.command == Command::Clear || stroke.command == Command::Backspace)
        return true;
    return edit(stroke);
}

bool Entry::edit(Keystroke stroke)
{
    const QString before = text_;
    if (dispatch(stroke) && text_.size() <= kMaxLength)
        return true;
    text_ = before;
    return false;
}

bool Entry::dispatch(Keystroke stroke)
{
    switch (stroke.command) {
    case Command::Digit:
        return digit(stroke.token);
    case Command::Point:
        return point();
    case Command::Operator:
        return binary(stroke.token.front());
    case Command::Open:
        text_ += u'(';
        return true;
    case Command::Close:
        return close();
    case Command::Postfix:
        return postfix(stroke.token);
    case Command::Function:
        text_ += stroke.token;
        text_ += u'(';
        return true;
    case Command::Constant:
        text_ += stroke.token;
        return true;
    case Command::Raise:
        return raise(stroke.token);
    case Command::Negate:
        return negate();
    case Command::Backspace:
        text_.chop(unitLength(text_));
        return true;
    case Command::Clear:
        reset();
        return true;
    case Command::Equals:
        return equals();
    case Command::Angle:
        angle_ = angle_ == Angle::Degrees ? Angle::Radians : Angle::Degrees;
        return true;
    }
    return false;
}

bool Entry::digit(QStringView token)
{
    const QStringView number = QStringView(text_).sliced(numberStart(text_));
    if (mantissaDigits(number) >= kMaxDigits)
        return false;
    if (number == u"0")
        text_.chop(1);
    text_ += token;
    return true;
}

bool Entry::point()
{
    const QStringView number = QStringView(text_).sliced(numberStart(text_));
    if (number.contains(u'.') || number.contains(u'E'))
        return false;
    text_ += number.isEmpty() ? QStringView(u"0.") : QStringView(u".");
    return true;
}

bool Entry::binary(QChar op)
{
    if (text_.isEmpty())
        return begin(op);
    const QChar last = text_.back();
    if (last == u'(')
        return appendMinus(op);
    const bool unaryMinus = op == u'-' && kUnaryMinusAfter.contains(last);
    if (isOperator(last) && !unaryMinus)
        return replaceOperators(op);
    text_ += op;
    return true;
}

bool Entry::begin(QChar op)
{
    if (op != u'-')
        text_ += u'0';
    text_ += op;
    return true;
}

bool Entry::appendMinus(QChar op)
{
    if (op != u'-')
        return false;
    text_ += op;
    return true;
}

bool Entry::replaceOperators(QChar op)
{
    qsizetype base = text_.size();
    while (base > 0 && isOperator(text_[base - 1]))
        --base;
    text_.truncate(base);
    if (text_.isEmpty())
        return begin(op);
    if (text_.back() == u'(')
        return appendMinus(op);
    text_ += op;
    return true;
}

bool Entry::close()
{
    if (openParens() == 0 || !endsWithOperand())
        return false;
    text_ += u')';
    return true;
}

bool Entry::postfix(QStringView token)
{
    if (!endsWithOperand())
        return false;
    text_ += token;
    return true;
}

bool Entry::raise(QStringView base)
{
    if (endsWithOperand())
        text_ += u'*';
    text_ += base;
    text_ += u'^';
    return true;
}

bool Entry::negate()
{
    const qsizetype start = numberStart(text_);
    if (start == text_.size())
        return negateEmpty();
    const QStringView head = QStringView(text_).first(start);
    if (head.endsWith(u"(-")) {
        text_.remove(start - 2, 2);
        return true;
    }
    const bool unaryMinus = head.endsWith(u'-') && (start == 1 || !kOperandEnd.contains(text_[start - 2]));
    if (unaryMinus) {
        text_.remove(start - 1, 1);
        return true;
    }
    text_.insert(start, start == 0 ? QStringView(u"-") : QStringView(u"(-"));
    return true;
}

bool Entry::negateEmpty()
{
    const bool accepts = text_.isEmpty() || isOperator(text_.back()) || text_.back() == u'(';
    if (!accepts)
        return false;
    text_ += u'-';
    return true;
}

bool Entry::equals()
{
    if (text_.isEmpty())
        return true;
    const Evaluation outcome = evaluate(text_, angle_);
    if (outcome.fault == Fault::Syntax)
        return false;
    equation_ = text_ + QString(openParens(), u')');
    if (outcome.fault != Fault::None) {
        phase_ = Phase::Failed;
        fault_ = outcome.fault;
        return true;
    }
    text_ = formatNumber(outcome.value);
    phase_ = Phase::Result;
    return true;
}

bool Entry::endsWithOperand() const
{
    return !text_.isEmpty() && kOperandEnd.contains(text_.back());
}

void Entry::reset()
{
    text_.clear();
    equation_.clear();
    phase_ = Phase::Editing;
    fault_ = Fault::None;
}

void Entry::refresh()
{
    preview_.clear();
    if (phase_ != Phase::Editing || text_.isEmpty())
        return;
    const Evaluation outcome = evaluate(text_, angle_);
    if (outcome.fault != Fault::None)
        return;
    QString shown = formatNumber(outcome.value);
    if (shown != text_)
        preview_ = std::move(shown);
}
