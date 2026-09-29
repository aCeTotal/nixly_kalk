#include "math/function.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>

namespace {

constexpr long double kPi = std::numbers::pi_v<long double>;
constexpr long double kFullTurn = 360.0L;
constexpr long double kHalfTurn = 180.0L;
constexpr long double kSnap = 4 * std::numeric_limits<long double>::epsilon();
constexpr long double kNaN = std::numeric_limits<long double>::quiet_NaN();

long double toRadians(long double x, Angle angle)
{
    if (angle == Angle::Radians)
        return x;
    return std::fmod(x, kFullTurn) * kPi / kHalfTurn;
}

long double fromRadians(long double x, Angle angle)
{
    if (angle == Angle::Radians)
        return x;
    return x * kHalfTurn / kPi;
}

bool negligible(long double value, long double radians)
{
    return std::fabs(value) < kSnap * std::max(1.0L, std::fabs(radians));
}

long double snapped(long double value, long double radians)
{
    return negligible(value, radians) ? 0 : value;
}

long double sine(long double x, Angle angle)
{
    const long double radians = toRadians(x, angle);
    return snapped(std::sin(radians), radians);
}

long double cosine(long double x, Angle angle)
{
    const long double radians = toRadians(x, angle);
    return snapped(std::cos(radians), radians);
}

long double tangent(long double x, Angle angle)
{
    const long double radians = toRadians(x, angle);
    if (negligible(std::cos(radians), radians))
        return kNaN;
    return snapped(std::tan(radians), radians);
}

long double arcsine(long double x, Angle angle)
{
    return fromRadians(std::asin(x), angle);
}

long double arccosine(long double x, Angle angle)
{
    return fromRadians(std::acos(x), angle);
}

long double arctangent(long double x, Angle angle)
{
    return fromRadians(std::atan(x), angle);
}

long double natural(long double x, Angle)
{
    return x > 0 ? std::log(x) : kNaN;
}

long double common(long double x, Angle)
{
    return x > 0 ? std::log10(x) : kNaN;
}

constexpr std::array<Function, 8> kFunctions{{
    {u"sin", sine},
    {u"cos", cosine},
    {u"tan", tangent},
    {u"asin", arcsine},
    {u"acos", arccosine},
    {u"atan", arctangent},
    {u"ln", natural},
    {u"log", common},
}};

} // namespace

std::span<const Function> functions()
{
    return kFunctions;
}
