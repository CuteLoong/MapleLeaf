#pragma once

#include "glm/common.hpp"
#include "glm/glm.hpp"
#include <cassert>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdint.h>
#include <string>

namespace MapleLeaf {
class Color
{
public:
    enum class Type
    {
        RGBA,
        ARGB,
        RGB
    };

    static const Color Clear;
    static const Color Black;
    static const Color Grey;
    static const Color Silver;
    static const Color White;
    static const Color Maroon;
    static const Color Red;
    static const Color Olive;
    static const Color Yellow;
    static const Color Green;
    static const Color Lime;
    static const Color Teal;
    static const Color Aqua;
    static const Color Navy;
    static const Color Blue;
    static const Color Purple;
    static const Color Fuchsia;

    float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;

    Color() = default;

    /**
     * Constructor for Color.
     * @param r The new R value.
     * @param g The new G value.
     * @param b The new B value.
     * @param a The new A value.
     */
    constexpr Color(float r, float g, float b, float a = 1.0f)
        : r(r)
        , g(g)
        , b(b)
        , a(a)
    {}

    /**
     * Constructor for Color.
     * @param i The integer value.
     * @param type The order components of Color are packed.
     */
    constexpr Color(uint32_t i, Type type = Type::RGB)
    {
        switch (type) {
        case Type::RGBA:
            r = static_cast<float>((uint8_t)(i >> 24 & 0xFF)) / 255.0f;
            g = static_cast<float>((uint8_t)(i >> 16 & 0xFF)) / 255.0f;
            b = static_cast<float>((uint8_t)(i >> 8 & 0xFF)) / 255.0f;
            a = static_cast<float>((uint8_t)(i & 0xFF)) / 255.0f;
            break;
        case Type::ARGB:
            r = static_cast<float>((uint8_t)(i >> 16)) / 255.0f;
            g = static_cast<float>((uint8_t)(i >> 8)) / 255.0f;
            b = static_cast<float>((uint8_t)(i & 0xFF)) / 255.0f;
            a = static_cast<float>((uint8_t)(i >> 24)) / 255.0f;
            break;
        case Type::RGB:
            r = static_cast<float>((uint8_t)(i >> 16)) / 255.0f;
            g = static_cast<float>((uint8_t)(i >> 8)) / 255.0f;
            b = static_cast<float>((uint8_t)(i & 0xFF)) / 255.0f;
            a = 1.0f;
            break;
        default: throw std::runtime_error("Unknown Color type");
        }
    }

    /**
     * Constructor for Color.
     * @param hex The new values from HEX.
     * @param a The new A value.
     */
    Color(std::string hex, float a = 1.0f)
        : a(a)
    {
        if (hex[0] == '#') {
            hex.erase(0, 1);
        }

        assert(hex.size() == 6);
        auto hexValue = std::stoul(hex, nullptr, 16);

        r = static_cast<float>((hexValue >> 16) & 0xff) / 255.0f;
        g = static_cast<float>((hexValue >> 8) & 0xff) / 255.0f;
        b = static_cast<float>((hexValue >> 0) & 0xff) / 255.0f;
    }

    /**
     * Calculates the linear interpolation between this Color and another Color.
     * @param other The other quaternion.
     * @param progression The progression.
     * @return Left lerp right.
     */
    constexpr Color Lerp(const Color& other, float progression) const
    {
        auto ta = *this * (1.0f - progression);
        auto tb = other * progression;
        return ta + tb;
    }

    /**
     * Normalizes this color.
     * @return The normalized color.
     */
    Color Normalize() const
    {
        auto l = Length();

        if (l == 0.0f) throw std::runtime_error("Can't normalize a zero length vector");

        return {r / l, g / l, b / l, a / l};
    }

    /**
     * Gets the length squared of this color.
     * @return The length squared.
     */
    constexpr float Length2() const { return r * r + g * g + b * b + a * a; }

    /**
     * Gets the length of this color.
     * @return The length.
     */
    float Length() const { return std::sqrt(Length2()); }

    /**
     * Gradually changes this color to a target.
     * @param target The target color.
     * @param rate The rate to go from current to the target.
     * @return The changed color.
     */
    Color SmoothStep(const Color& target, const Color& rate) const
    {
        float r = glm::smoothstep(this->r, target.r, rate.r);
        float g = glm::smoothstep(this->g, target.g, rate.g);
        float b = glm::smoothstep(this->b, target.b, rate.b);
        float a = glm::smoothstep(this->a, target.a, rate.a);
        return Color(r, g, b, a);
    }

    /**
     * Gets a colour representing the unit value of this color.
     * @return The unit color.
     */
    Color GetUnit() const
    {
        auto l = Length();
        return {r / l, g / l, b / l, a / l};
    }

    /**
     * Gets a packed integer representing this color.
     * @param type The order components of color are packed.
     * @return The packed integer.
     */
    constexpr uint32_t GetInt(Type type = Type::RGBA) const
    {
        switch (type) {
        case Type::RGBA:
            return (static_cast<uint8_t>(r * 255.0f) << 24) | (static_cast<uint8_t>(g * 255.0f) << 16) | (static_cast<uint8_t>(b * 255.0f) << 8) |
                   (static_cast<uint8_t>(a * 255.0f) & 0xFF);
        case Type::ARGB:
            return (static_cast<uint8_t>(a * 255.0f) << 24) | (static_cast<uint8_t>(r * 255.0f) << 16) | (static_cast<uint8_t>(g * 255.0f) << 8) |
                   (static_cast<uint8_t>(b * 255.0f) & 0xFF);
        case Type::RGB:
            return (static_cast<uint8_t>(r * 255.0f) << 16) | (static_cast<uint8_t>(g * 255.0f) << 8) | (static_cast<uint8_t>(b * 255.0f) & 0xFF);
        default: throw std::runtime_error("Unknown Color type");
        }
    }

    /**
     * Gets the hex code from this color.
     * @return The hex code.
     */
    std::string GetHex() const
    {
        std::stringstream stream;
        stream << "#";

        auto hexValue = ((static_cast<uint32_t>(r * 255.0f) & 0xff) << 16) + ((static_cast<uint32_t>(g * 255.0f) & 0xff) << 8) +
                        ((static_cast<uint32_t>(b * 255.0f) & 0xff) << 0);
        stream << std::hex << std::setfill('0') << std::setw(6) << hexValue;

        return stream.str();
    }

    constexpr float operator[](uint32_t i) const
    {
        assert(i < 4 && "Color subscript out of range");
        return i == 0 ? r : i == 1 ? g : i == 2 ? b : a;
    }
    constexpr float& operator[](uint32_t i)
    {
        assert(i < 4 && "Color subscript out of range");
        return i == 0 ? r : i == 1 ? g : i == 2 ? b : a;
    }

    constexpr bool operator==(const Color& rhs) const;
    constexpr bool operator!=(const Color& rhs) const;

    constexpr friend Color operator+(const Color& lhs, const Color& rhs);
    constexpr friend Color operator-(const Color& lhs, const Color& rhs);
    constexpr friend Color operator*(const Color& lhs, const Color& rhs);
    constexpr friend Color operator/(const Color& lhs, const Color& rhs);
    constexpr friend Color operator+(float lhs, const Color& rhs);
    constexpr friend Color operator-(float lhs, const Color& rhs);
    constexpr friend Color operator*(float lhs, const Color& rhs);
    constexpr friend Color operator/(float lhs, const Color& rhs);
    constexpr friend Color operator+(const Color& lhs, float rhs);
    constexpr friend Color operator-(const Color& lhs, float rhs);
    constexpr friend Color operator*(const Color& lhs, float rhs);
    constexpr friend Color operator/(const Color& lhs, float rhs);

    constexpr Color& operator+=(const Color& rhs);
    constexpr Color& operator-=(const Color& rhs);
    constexpr Color& operator*=(const Color& rhs);
    constexpr Color& operator/=(const Color& rhs);
    constexpr Color& operator+=(float rhs);
    constexpr Color& operator-=(float rhs);
    constexpr Color& operator*=(float rhs);
    constexpr Color& operator/=(float rhs);
};
}   // namespace MapleLeaf

#include "Color.inl"