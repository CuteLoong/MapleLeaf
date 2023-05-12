#pragma once

#include "Color.hpp"

namespace MapleLeaf {
constexpr bool Color::operator==(const Color& rhs) const
{
    return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
}

constexpr bool Color::operator!=(const Color& rhs) const
{
    return !operator==(rhs);
}

constexpr Color operator+(const Color& lhs, const Color& rhs)
{
    return {lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b, lhs.a + rhs.a};
}

constexpr Color operator-(const Color& lhs, const Color& rhs)
{
    return {lhs.r - rhs.r, lhs.g - rhs.g, lhs.b - rhs.b, lhs.a - rhs.a};
}

constexpr Color operator*(const Color& lhs, const Color& rhs)
{
    return {lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b, lhs.a * rhs.a};
}

constexpr Color operator/(const Color& lhs, const Color& rhs)
{
    return {lhs.r / rhs.r, lhs.g / rhs.g, lhs.b / rhs.b, lhs.a / rhs.a};
}

constexpr Color operator+(float lhs, const Color& rhs)
{
    return Color(lhs, lhs, lhs, 0.0f) + rhs;
}

constexpr Color operator-(float lhs, const Color& rhs)
{
    return Color(lhs, lhs, lhs, 0.0f) - rhs;
}

constexpr Color operator*(float lhs, const Color& rhs)
{
    return Color(lhs, lhs, lhs) * rhs;
}

constexpr Color operator/(float lhs, const Color& rhs)
{
    return Color(lhs, lhs, lhs) / rhs;
}

constexpr Color operator+(const Color& lhs, float rhs)
{
    return lhs + Color(rhs, rhs, rhs, 0.0f);
}

constexpr Color operator-(const Color& lhs, float rhs)
{
    return lhs - Color(rhs, rhs, rhs, 0.0f);
}

constexpr Color operator*(const Color& lhs, float rhs)
{
    return lhs * Color(rhs, rhs, rhs);
}

constexpr Color operator/(const Color& lhs, float rhs)
{
    return lhs / Color(rhs, rhs, rhs);
}

constexpr Color& Color::operator+=(const Color& rhs)
{
    return *this = *this + rhs;
}

constexpr Color& Color::operator-=(const Color& rhs)
{
    return *this = *this - rhs;
}

constexpr Color& Color::operator*=(const Color& rhs)
{
    return *this = *this * rhs;
}

constexpr Color& Color::operator/=(const Color& rhs)
{
    return *this = *this / rhs;
}

constexpr Color& Color::operator+=(float rhs)
{
    return *this = *this + rhs;
}

constexpr Color& Color::operator-=(float rhs)
{
    return *this = *this - rhs;
}

constexpr Color& Color::operator*=(float rhs)
{
    return *this = *this * rhs;
}

constexpr Color& Color::operator/=(float rhs)
{
    return *this = *this / rhs;
}
}   // namespace MapleLeaf