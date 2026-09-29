#include "keypad/layout.h"

#include <array>

namespace {

constexpr int kStandardColumns = 4;
constexpr int kScientificColumns = 5;

constexpr KeySpec plain(QStringView label, Keystroke stroke, Kind kind)
{
    return {label, stroke, {}, stroke, kind};
}

constexpr KeySpec digit(QStringView token)
{
    return plain(token, {Command::Digit, token}, Kind::Digit);
}

constexpr KeySpec sign(QStringView label, QStringView token)
{
    return plain(label, {Command::Operator, token}, Kind::Operator);
}

constexpr KeySpec kOpen = plain(u"(", {Command::Open, u"("}, Kind::Function);
constexpr KeySpec kClose = plain(u")", {Command::Close, u")"}, Kind::Function);
constexpr KeySpec kRoot = plain(u"√", {Command::Function, u"√"}, Kind::Function);
constexpr KeySpec kSquare = plain(u"x²", {Command::Postfix, u"²"}, Kind::Function);
constexpr KeySpec kClear = plain(u"C", {Command::Clear, {}}, Kind::Clear);
constexpr KeySpec kBackspace = plain(u"⌫", {Command::Backspace, {}}, Kind::Function);
constexpr KeySpec kPercent = plain(u"%", {Command::Postfix, u"%"}, Kind::Function);
constexpr KeySpec kDivide = sign(u"÷", u"/");
constexpr KeySpec kTimes = sign(u"×", u"*");
constexpr KeySpec kMinus = sign(u"−", u"-");
constexpr KeySpec kPlus = sign(u"+", u"+");
constexpr KeySpec kNegate = plain(u"±", {Command::Negate, {}}, Kind::Digit);
constexpr KeySpec kPoint = plain(u".", {Command::Point, u"."}, Kind::Digit);
constexpr KeySpec kEquals = plain(u"=", {Command::Equals, {}}, Kind::Equals);

constexpr KeySpec kShift = plain(u"2nd", {Command::Clear, {}}, Kind::Shift);
constexpr KeySpec kAngle = plain(u"DEG", {Command::Angle, {}}, Kind::Function);
constexpr KeySpec kPower = plain(u"xʸ", {Command::Operator, u"^"}, Kind::Function);
constexpr KeySpec kPi = plain(u"π", {Command::Constant, u"π"}, Kind::Function);
constexpr KeySpec kEuler = plain(u"e", {Command::Constant, u"e"}, Kind::Function);
constexpr KeySpec kFactorial = plain(u"n!", {Command::Postfix, u"!"}, Kind::Function);
constexpr KeySpec kSin{u"sin", {Command::Function, u"sin"}, u"sin⁻¹", {Command::Function, u"asin"}, Kind::Function};
constexpr KeySpec kCos{u"cos", {Command::Function, u"cos"}, u"cos⁻¹", {Command::Function, u"acos"}, Kind::Function};
constexpr KeySpec kTan{u"tan", {Command::Function, u"tan"}, u"tan⁻¹", {Command::Function, u"atan"}, Kind::Function};
constexpr KeySpec kLn{u"ln", {Command::Function, u"ln"}, u"eˣ", {Command::Raise, u"e"}, Kind::Function};
constexpr KeySpec kLog{u"log", {Command::Function, u"log"}, u"10ˣ", {Command::Raise, u"10"}, Kind::Function};

constexpr std::array kDigit{
    digit(u"0"), digit(u"1"), digit(u"2"), digit(u"3"), digit(u"4"),
    digit(u"5"), digit(u"6"), digit(u"7"), digit(u"8"), digit(u"9"),
};

constexpr std::array kStandard{
    kOpen,     kClose,     kRoot,     kSquare,
    kClear,    kBackspace, kPercent,  kDivide,
    kDigit[7], kDigit[8],  kDigit[9], kTimes,
    kDigit[4], kDigit[5],  kDigit[6], kMinus,
    kDigit[1], kDigit[2],  kDigit[3], kPlus,
    kNegate,   kDigit[0],  kPoint,    kEquals,
};

constexpr std::array kScientific{
    kShift,     kSin,      kCos,       kTan,      kAngle,
    kPower,     kOpen,     kClose,     kRoot,     kSquare,
    kLn,        kClear,    kBackspace, kPercent,  kDivide,
    kLog,       kDigit[7], kDigit[8],  kDigit[9], kTimes,
    kPi,        kDigit[4], kDigit[5],  kDigit[6], kMinus,
    kEuler,     kDigit[1], kDigit[2],  kDigit[3], kPlus,
    kFactorial, kNegate,   kDigit[0],  kPoint,    kEquals,
};

} // namespace

std::span<const KeySpec> keysOf(Mode mode)
{
    if (mode == Mode::Standard)
        return kStandard;
    return kScientific;
}

int columnsOf(Mode mode)
{
    return mode == Mode::Standard ? kStandardColumns : kScientificColumns;
}
