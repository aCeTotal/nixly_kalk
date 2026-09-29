#include "text/pretty.h"

#include <algorithm>
#include <array>

namespace {

constexpr QChar kThinSpace = u' ';
constexpr QChar kSuperscriptMinus = u'⁻';
constexpr QStringView kSuperscripts = u"⁰¹²³⁴⁵⁶⁷⁸⁹";
constexpr QStringView kTimesTen = u"×10";
constexpr QStringView kSigns = u"+-*/";
constexpr QStringView kUnaryAfter = u"+-*/^(";
constexpr qsizetype kGroupSize = 3;
constexpr qsizetype kMinGrouped = 4;

struct Inverse {
    QStringView name;
    QStringView shown;
};

constexpr std::array kInverses{
    Inverse{u"asin", u"sin⁻¹"},
    Inverse{u"acos", u"cos⁻¹"},
    Inverse{u"atan", u"tan⁻¹"},
};

QChar glyphOf(QChar sign)
{
    switch (sign.unicode()) {
    case u'*':
        return u'×';
    case u'/':
        return u'÷';
    case u'-':
        return u'−';
    default:
        return sign;
    }
}

class Pretty
{
public:
    Pretty(QStringView expression, const Notation &notation) : in_(expression), notation_(notation) {}

    QString run();

private:
    void step();
    void number();
    void integer(QStringView digits);
    void exponent();
    void sign(QChar sign);
    void word();
    bool at(QChar c) const { return pos_ < in_.size() && in_[pos_] == c; }
    bool atDigit() const { return pos_ < in_.size() && in_[pos_].isDigit(); }

    QStringView in_;
    const Notation &notation_;
    QString out_;
    qsizetype pos_ = 0;
};

QString Pretty::run()
{
    out_.reserve(in_.size() * 2);
    while (pos_ < in_.size())
        step();
    return std::move(out_);
}

void Pretty::step()
{
    const QChar c = in_[pos_];
    if (c.isDigit() || c == u'.')
        number();
    else if (kSigns.contains(c))
        sign(c);
    else
        word();
}

void Pretty::number()
{
    const qsizetype start = pos_;
    while (atDigit())
        ++pos_;
    integer(in_.sliced(start, pos_ - start));
    for (; atDigit() || at(u'.'); ++pos_) {
        if (in_[pos_] == u'.')
            out_ += notation_.decimal;
        else
            out_ += in_[pos_];
    }
    if (at(u'E'))
        exponent();
}

void Pretty::integer(QStringView digits)
{
    const qsizetype count = digits.size();
    const bool grouped = count >= kMinGrouped;
    for (qsizetype i = 0; i < count; ++i) {
        if (grouped && i > 0 && (count - i) % kGroupSize == 0)
            out_ += notation_.group;
        out_ += digits[i];
    }
}

void Pretty::exponent()
{
    ++pos_;
    out_ += kTimesTen;
    if (at(u'-')) {
        out_ += kSuperscriptMinus;
        ++pos_;
    }
    for (; atDigit(); ++pos_)
        out_ += kSuperscripts[in_[pos_].digitValue()];
}

void Pretty::sign(QChar sign)
{
    const bool prefix = sign == u'+' || sign == u'-';
    const bool unary = prefix && (pos_ == 0 || kUnaryAfter.contains(in_[pos_ - 1]));
    ++pos_;
    if (unary) {
        out_ += glyphOf(sign);
        return;
    }
    out_ += kThinSpace;
    out_ += glyphOf(sign);
    out_ += kThinSpace;
}

void Pretty::word()
{
    const QStringView rest = in_.sliced(pos_);
    const auto inverse = std::ranges::find_if(kInverses, [rest](const Inverse &candidate) {
        return rest.startsWith(candidate.name);
    });
    if (inverse == kInverses.end()) {
        out_ += in_[pos_++];
        return;
    }
    out_ += inverse->shown;
    pos_ += inverse->name.size();
}

} // namespace

QString prettify(QStringView expression, const Notation &notation)
{
    return Pretty(expression, notation).run();
}
