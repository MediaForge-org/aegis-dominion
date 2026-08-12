#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace mf {

struct Vec2 {
    float x{};
    float y{};
    friend constexpr bool operator==(Vec2, Vec2) noexcept = default;
};

struct Vec3 {
    float x{};
    float y{};
    float z{};
    friend constexpr bool operator==(Vec3, Vec3) noexcept = default;
};

struct Vec4 {
    float x{};
    float y{};
    float z{};
    float w{};
    friend constexpr bool operator==(Vec4, Vec4) noexcept = default;
};

constexpr Vec2 operator+(Vec2 left, Vec2 right) noexcept { return {left.x + right.x, left.y + right.y}; }
constexpr Vec2 operator-(Vec2 left, Vec2 right) noexcept { return {left.x - right.x, left.y - right.y}; }
constexpr Vec2 operator*(Vec2 value, float scale) noexcept { return {value.x * scale, value.y * scale}; }
constexpr Vec3 operator+(Vec3 left, Vec3 right) noexcept { return {left.x + right.x, left.y + right.y, left.z + right.z}; }
constexpr Vec3 operator-(Vec3 left, Vec3 right) noexcept { return {left.x - right.x, left.y - right.y, left.z - right.z}; }
constexpr Vec3 operator*(Vec3 value, float scale) noexcept { return {value.x * scale, value.y * scale, value.z * scale}; }
constexpr float dot(Vec2 left, Vec2 right) noexcept { return left.x * right.x + left.y * right.y; }
constexpr float dot(Vec3 left, Vec3 right) noexcept { return left.x * right.x + left.y * right.y + left.z * right.z; }
inline float length(Vec2 value) noexcept { return std::sqrt(dot(value, value)); }
inline float length(Vec3 value) noexcept { return std::sqrt(dot(value, value)); }

struct Mat4 {
    std::array<float, 16> values{};

    [[nodiscard]] static constexpr Mat4 identity() noexcept {
        return {{1.0F, 0.0F, 0.0F, 0.0F,
                 0.0F, 1.0F, 0.0F, 0.0F,
                 0.0F, 0.0F, 1.0F, 0.0F,
                 0.0F, 0.0F, 0.0F, 1.0F}};
    }

    constexpr float& at(int row, int column) noexcept { return values[static_cast<std::size_t>(column * 4 + row)]; }
    constexpr float at(int row, int column) const noexcept { return values[static_cast<std::size_t>(column * 4 + row)]; }
    friend constexpr bool operator==(const Mat4&, const Mat4&) noexcept = default;
};

constexpr Mat4 operator*(const Mat4& left, const Mat4& right) noexcept {
    Mat4 result{};
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            for (int inner = 0; inner < 4; ++inner) {
                result.at(row, column) += left.at(row, inner) * right.at(inner, column);
            }
        }
    }
    return result;
}

constexpr Mat4 translation(Vec3 value) noexcept {
    Mat4 result = Mat4::identity();
    result.at(0, 3) = value.x;
    result.at(1, 3) = value.y;
    result.at(2, 3) = value.z;
    return result;
}

constexpr Mat4 scale(Vec3 value) noexcept {
    Mat4 result{};
    result.at(0, 0) = value.x;
    result.at(1, 1) = value.y;
    result.at(2, 2) = value.z;
    result.at(3, 3) = 1.0F;
    return result;
}

constexpr float radians(float degreesValue) noexcept {
    return degreesValue * std::numbers::pi_v<float> / 180.0F;
}

constexpr float degrees(float radiansValue) noexcept {
    return radiansValue * 180.0F / std::numbers::pi_v<float>;
}

inline Mat4 rotationZ(float angleRadians) noexcept {
    Mat4 result = Mat4::identity();
    const float cosine = std::cos(angleRadians);
    const float sine = std::sin(angleRadians);
    result.at(0, 0) = cosine;
    result.at(1, 0) = sine;
    result.at(0, 1) = -sine;
    result.at(1, 1) = cosine;
    return result;
}

inline Mat4 rotationX(float angleRadians) noexcept {
    Mat4 result = Mat4::identity();
    const float cosine = std::cos(angleRadians);
    const float sine = std::sin(angleRadians);
    result.at(1, 1) = cosine;
    result.at(2, 1) = sine;
    result.at(1, 2) = -sine;
    result.at(2, 2) = cosine;
    return result;
}

inline Mat4 rotationY(float angleRadians) noexcept {
    Mat4 result = Mat4::identity();
    const float cosine = std::cos(angleRadians);
    const float sine = std::sin(angleRadians);
    result.at(0, 0) = cosine;
    result.at(2, 0) = -sine;
    result.at(0, 2) = sine;
    result.at(2, 2) = cosine;
    return result;
}

template <typename T>
constexpr T clamp(T value, T low, T high) noexcept { return std::clamp(value, low, high); }

template <typename T, typename Factor>
constexpr T lerp(const T& from, const T& to, Factor factor) noexcept {
    return from + (to - from) * factor;
}

struct Transform2D {
    Vec2 position{};
    float rotationRadians{};
    Vec2 scale{1.0F, 1.0F};

    [[nodiscard]] Mat4 matrix() const noexcept {
        return translation({position.x, position.y, 0.0F}) * rotationZ(rotationRadians) *
               mf::scale({scale.x, scale.y, 1.0F});
    }
};

struct Transform3D {
    Vec3 position{};
    Vec3 rotationRadians{};
    Vec3 scale{1.0F, 1.0F, 1.0F};

    [[nodiscard]] Mat4 matrix() const noexcept {
        return translation(position) * rotationZ(rotationRadians.z) * rotationY(rotationRadians.y) *
               rotationX(rotationRadians.x) * mf::scale(scale);
    }
};

} // namespace mf
