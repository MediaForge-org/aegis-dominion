#pragma once

#include <mediaforge/render/GPUDevice.hpp>
#include <mediaforge/render/Renderer2D.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace aegis::mediaforge {

enum class GraphicsQuality { low, medium, high, ultra };

struct E2RunConfiguration {
    bool showcase{};
    bool verticalSync{true};
    unsigned fpsLimit{120};
    bool uncappedProfiling{};
    float warmupSeconds{2.0F};
    float benchmarkSeconds{};
    std::size_t stressEnemies{};
    std::size_t stressEnvironment{};
    std::size_t stressParticles{};
    GraphicsQuality quality{GraphicsQuality::high};
};

struct QualityConfiguration {
    float bloomScale{0.5F};
    float particleDensity{1.0F};
    float renderScale{1.0F};
    bool hdrWorld{true};
    bool hdrEmissive{true};
};

[[nodiscard]] E2RunConfiguration parseE2RunConfiguration(int argc, char** argv);
[[nodiscard]] QualityConfiguration qualityConfiguration(GraphicsQuality quality) noexcept;
[[nodiscard]] const char* qualityName(GraphicsQuality quality) noexcept;
[[nodiscard]] const char* presentModeName(mf::PresentMode mode) noexcept;
[[nodiscard]] std::uint64_t processResidentBytes() noexcept;

class FramePacer {
public:
    explicit FramePacer(unsigned framesPerSecond = 0) noexcept;
    void setLimit(unsigned framesPerSecond) noexcept;
    void beginFrame() noexcept;
    [[nodiscard]] float wait(bool reducedActivity = false) const;

private:
    unsigned framesPerSecond_{};
    std::chrono::steady_clock::time_point frameStart_{};
};

struct BenchmarkAccumulator {
    std::uint64_t frames{};
    double wallMilliseconds{};
    double cpuWorkMilliseconds{};
    double updateMilliseconds{};
    double spriteSubmissionMilliseconds{};
    double particleUpdateMilliseconds{};
    double batchConstructionMilliseconds{};
    double commandSubmissionMilliseconds{};
    double presentWaitMilliseconds{};
    double frameLimitWaitMilliseconds{};
    std::uint64_t sprites{};
    std::uint64_t particles{};
    std::uint64_t batches{};
    std::uint64_t draws{};
    std::uint64_t triangles{};
    std::uint64_t culled{};
    std::uint64_t renderTargetBytes{};
    std::uint64_t frameResourceBytes{};
    std::uint64_t frameStorageGrowths{};

    void add(const mf::RendererStatistics& statistics, float wallFrameMilliseconds) noexcept;
    [[nodiscard]] std::string report(std::string_view scene, std::uint64_t rssBytes,
                                     std::size_t textures, std::uint64_t textureBytes,
                                     float assetLoadMilliseconds) const;
};

} // namespace aegis::mediaforge
