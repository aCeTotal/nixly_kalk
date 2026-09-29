#include "math/number.h"

#include <algorithm>
#include <charconv>
#include <string_view>

namespace {

constexpr int kDigits = 16;
constexpr int kMinFixedExponent = -9;
constexpr int kBufferSize = 64;

std::string_view withoutTrailingZeros(std::string_view digits)
{
    if (digits.find('.') == std::string_view::npos)
        return digits;
    digits.remove_suffix(digits.size() - 1 - digits.find_last_not_of('0'));
    if (digits.back() == '.')
        digits.remove_suffix(1);
    return digits;
}

QString toQString(std::string_view text)
{
    return QString::fromLatin1(text.data(), static_cast<qsizetype>(text.size()));
}

} // namespace

QString formatNumber(long double value)
{
    if (value == 0)
        return QStringLiteral("0");

    char buffer[kBufferSize];
    const char *end = std::to_chars(buffer, buffer + kBufferSize, value,
                                    std::chars_format::scientific, kDigits - 1).ptr;
    const std::string_view scientific(buffer, end);
    const std::size_t mark = scientific.find('e');
    int exponent = 0;
    std::from_chars(scientific.data() + mark + 2, end, exponent);
    if (scientific[mark + 1] == '-')
        exponent = -exponent;

    if (exponent < kMinFixedExponent || exponent >= kDigits) {
        const QString mantissa = toQString(withoutTrailingZeros(scientific.substr(0, mark)));
        return mantissa + u'E' + QString::number(exponent);
    }

    char fixed[kBufferSize];
    const char *fixedEnd = std::to_chars(fixed, fixed + kBufferSize, value, std::chars_format::fixed,
                                         std::max(0, kDigits - 1 - exponent)).ptr;
    return toQString(withoutTrailingZeros(std::string_view(fixed, fixedEnd)));
}
