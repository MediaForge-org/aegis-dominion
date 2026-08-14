#pragma once

#include <mediaforge/math/Math.hpp>

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace mf {

struct Particle2D {
    Vec2 position{};
    Vec2 velocity{};
    Vec2 acceleration{};
    float lifetime{1.0F};
    float age{};
    float startSize{8.0F};
    float endSize{};
    float rotationRadians{};
    float angularVelocity{};
    Vec4 startColor{1.0F, 1.0F, 1.0F, 1.0F};
    Vec4 endColor{1.0F, 1.0F, 1.0F, 0.0F};
    Vec4 uvRegion{0.0F, 0.0F, 1.0F, 1.0F};
    float drag{};
    std::uint32_t textureKey{};
};

enum class EmitterType : std::uint8_t { burst, trail, stream };

class ParticleSystem2D {
public:
    explicit ParticleSystem2D(std::size_t capacity = 4096);
    [[nodiscard]] bool emit(const Particle2D& particle);
    std::size_t emit(std::span<const Particle2D> particles);
    void update(float deltaSeconds);
    void clear() noexcept { particles_.clear(); }
    [[nodiscard]] std::span<const Particle2D> particles() const noexcept { return particles_; }
    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }

private:
    std::size_t capacity_{};
    std::vector<Particle2D> particles_;
};

[[nodiscard]] float particleLifeRatio(const Particle2D& particle) noexcept;
[[nodiscard]] float particleSize(const Particle2D& particle) noexcept;
[[nodiscard]] Vec4 particleColor(const Particle2D& particle) noexcept;

} // namespace mf
