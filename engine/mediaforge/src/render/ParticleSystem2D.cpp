#include <mediaforge/render/ParticleSystem2D.hpp>

#include <algorithm>
#include <cmath>

namespace mf {

ParticleSystem2D::ParticleSystem2D(std::size_t capacity) : capacity_(capacity) { particles_.reserve(capacity); }

bool ParticleSystem2D::emit(const Particle2D& particle) {
    if (particles_.size() >= capacity_ || particle.lifetime <= 0.0F) return false;
    particles_.push_back(particle);
    return true;
}

std::size_t ParticleSystem2D::emit(std::span<const Particle2D> particles) {
    std::size_t emitted{};
    for (const auto& particle : particles) {
        if (!emit(particle)) break;
        ++emitted;
    }
    return emitted;
}

void ParticleSystem2D::update(float deltaSeconds) {
    const float delta = std::max(deltaSeconds, 0.0F);
    for (auto& particle : particles_) {
        particle.age += delta;
        particle.velocity = particle.velocity + particle.acceleration * delta;
        particle.velocity = particle.velocity * std::exp(-std::max(0.0F, particle.drag) * delta);
        particle.position = particle.position + particle.velocity * delta;
        particle.rotationRadians += particle.angularVelocity * delta;
    }
    std::erase_if(particles_, [](const Particle2D& particle) { return particle.age >= particle.lifetime; });
}

float particleLifeRatio(const Particle2D& particle) noexcept {
    return particle.lifetime > 0.0F ? clamp(particle.age / particle.lifetime, 0.0F, 1.0F) : 1.0F;
}

float particleSize(const Particle2D& particle) noexcept {
    return lerp(particle.startSize, particle.endSize, particleLifeRatio(particle));
}

Vec4 particleColor(const Particle2D& particle) noexcept {
    return {lerp(particle.startColor.x, particle.endColor.x, particleLifeRatio(particle)),
            lerp(particle.startColor.y, particle.endColor.y, particleLifeRatio(particle)),
            lerp(particle.startColor.z, particle.endColor.z, particleLifeRatio(particle)),
            lerp(particle.startColor.w, particle.endColor.w, particleLifeRatio(particle))};
}

} // namespace mf
