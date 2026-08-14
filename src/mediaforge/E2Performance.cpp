#include "E2Performance.hpp"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <format>
#include <stdexcept>
#include <string_view>
#include <thread>

namespace aegis::mediaforge {
namespace {

template <typename Value>
Value parseNumber(std::string_view text, std::string_view option) {
    Value result{};
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size()) {
        throw std::invalid_argument("Invalid value for " + std::string(option));
    }
    return result;
}

std::string_view argumentValue(int& index, int argc, char** argv, std::string_view option) {
    if (index + 1 >= argc) throw std::invalid_argument("Missing value for " + std::string(option));
    return argv[++index];
}

} // namespace

E2RunConfiguration parseE2RunConfiguration(int argc, char** argv) {
    E2RunConfiguration result;
    for (int index = 1; index < argc; ++index) {
        const std::string_view option = argv[index];
        if (option == "--e2-showcase") {
            result.showcase = true;
        } else if (option == "--profile") {
            result.uncappedProfiling = true;
            result.verticalSync = false;
            result.fpsLimit = 0;
        } else if (option == "--vsync") {
            const auto value = argumentValue(index, argc, argv, option);
            if (value != "on" && value != "off") throw std::invalid_argument("--vsync expects on or off");
            result.verticalSync = value == "on";
        } else if (option == "--fps-limit") {
            result.fpsLimit = parseNumber<unsigned>(argumentValue(index, argc, argv, option), option);
            if (result.fpsLimit != 0) result.fpsLimit = std::clamp(result.fpsLimit, 15U, 360U);
        } else if (option == "--warmup-seconds") {
            result.warmupSeconds = parseNumber<float>(argumentValue(index, argc, argv, option), option);
        } else if (option == "--benchmark-seconds") {
            result.benchmarkSeconds = parseNumber<float>(argumentValue(index, argc, argv, option), option);
        } else if (option == "--stress-enemies") {
            result.stressEnemies = parseNumber<std::size_t>(argumentValue(index, argc, argv, option), option);
        } else if (option == "--stress-environment") {
            result.stressEnvironment = parseNumber<std::size_t>(argumentValue(index, argc, argv, option), option);
        } else if (option == "--stress-particles") {
            result.stressParticles = parseNumber<std::size_t>(argumentValue(index, argc, argv, option), option);
        } else if (option == "--quality") {
            const auto value = argumentValue(index, argc, argv, option);
            if (value == "low") result.quality = GraphicsQuality::low;
            else if (value == "medium") result.quality = GraphicsQuality::medium;
            else if (value == "high") result.quality = GraphicsQuality::high;
            else if (value == "ultra") result.quality = GraphicsQuality::ultra;
            else throw std::invalid_argument("--quality expects low, medium, high or ultra");
        } else if (option == "--help") {
            throw std::invalid_argument(
                "Options: --e2-showcase --profile --vsync on|off --fps-limit N --warmup-seconds N --benchmark-seconds N "
                "--stress-enemies N --stress-environment N --stress-particles N --quality low|medium|high|ultra");
        } else {
            throw std::invalid_argument("Unknown option: " + std::string(option));
        }
    }
    result.warmupSeconds = std::max(0.0F, result.warmupSeconds);
    result.benchmarkSeconds = std::max(0.0F, result.benchmarkSeconds);
    return result;
}

QualityConfiguration qualityConfiguration(GraphicsQuality quality) noexcept {
    switch (quality) {
        case GraphicsQuality::low: return {0.25F, 0.5F, 0.75F, false, true};
        case GraphicsQuality::medium: return {0.33F, 0.75F, 0.9F, true, true};
        case GraphicsQuality::high: return {0.5F, 1.0F, 1.0F, true, true};
        case GraphicsQuality::ultra: return {0.67F, 1.0F, 1.0F, true, true};
    }
    return {};
}

const char* qualityName(GraphicsQuality quality) noexcept {
    switch (quality) {
        case GraphicsQuality::low: return "low";
        case GraphicsQuality::medium: return "medium";
        case GraphicsQuality::high: return "high";
        case GraphicsQuality::ultra: return "ultra";
    }
    return "high";
}

const char* presentModeName(mf::PresentMode mode) noexcept {
    switch (mode) {
        case mf::PresentMode::vsync: return "vsync";
        case mf::PresentMode::immediate: return "immediate";
        case mf::PresentMode::mailbox: return "mailbox";
    }
    return "unknown";
}

std::uint64_t processResidentBytes() noexcept {
#if defined(__linux__)
    std::ifstream status("/proc/self/status");
    std::string key;
    while (status >> key) {
        if (key == "VmRSS:") {
            std::uint64_t kibibytes{};
            status >> kibibytes;
            return kibibytes * 1024U;
        }
        std::string remainder;
        std::getline(status, remainder);
    }
#endif
    return 0;
}

FramePacer::FramePacer(unsigned framesPerSecond) noexcept : framesPerSecond_(framesPerSecond) {}
void FramePacer::setLimit(unsigned framesPerSecond) noexcept { framesPerSecond_ = framesPerSecond; }
void FramePacer::beginFrame() noexcept { frameStart_ = std::chrono::steady_clock::now(); }

float FramePacer::wait(bool reducedActivity) const {
    const unsigned effectiveLimit = reducedActivity ? 15U : framesPerSecond_;
    if (effectiveLimit == 0 || frameStart_ == std::chrono::steady_clock::time_point{}) return 0.0F;
    const auto target = frameStart_ + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
        std::chrono::duration<double>(1.0 / static_cast<double>(effectiveLimit)));
    const auto before = std::chrono::steady_clock::now();
    if (before < target) std::this_thread::sleep_until(target);
    return std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - before).count();
}

void BenchmarkAccumulator::add(const mf::RendererStatistics& stats, float wallFrameMilliseconds) noexcept {
    ++frames;
    wallMilliseconds += wallFrameMilliseconds;
    cpuWorkMilliseconds += stats.cpuFrameMilliseconds;
    updateMilliseconds += stats.cpuUpdateMilliseconds;
    spriteSubmissionMilliseconds += stats.spriteSubmissionMilliseconds;
    particleUpdateMilliseconds += stats.particleUpdateMilliseconds;
    batchConstructionMilliseconds += stats.batchConstructionMilliseconds;
    commandSubmissionMilliseconds += stats.commandSubmissionMilliseconds;
    presentWaitMilliseconds += stats.presentWaitMilliseconds;
    frameLimitWaitMilliseconds += stats.frameLimitWaitMilliseconds;
    sprites += stats.submittedSprites + stats.cachedSprites;
    particles += stats.submittedParticles;
    batches += stats.batches;
    draws += stats.drawCalls;
    triangles += stats.triangles;
    culled += stats.culledSprites;
    renderTargetBytes = stats.renderTargetBytes;
    frameResourceBytes = stats.frameResourceBytes;
    frameStorageGrowths += stats.frameStorageGrowths;
}

std::string BenchmarkAccumulator::report(std::string_view scene, std::uint64_t rssBytes,
                                         std::size_t textures, std::uint64_t textureBytes,
                                         float assetLoadMilliseconds) const {
    const double divisor = frames == 0 ? 1.0 : static_cast<double>(frames);
    const double averageWall = wallMilliseconds / divisor;
    return std::format(
        "BENCHMARK scene={} frames={} fps={:.2f} frame_ms={:.3f} cpu_work_ms={:.3f} update_ms={:.3f} "
        "sprite_submit_ms={:.3f} particle_update_ms={:.3f} batch_ms={:.3f} command_submit_ms={:.3f} "
        "present_wait_ms={:.3f} limiter_wait_ms={:.3f} sprites={:.1f} particles={:.1f} batches={:.1f} "
        "draws={:.1f} triangles={:.1f} culled={:.1f} rss_bytes={} textures={} texture_bytes={} "
        "render_target_bytes={} frame_resource_bytes={} frame_storage_growths={} asset_load_ms={:.3f}",
        scene, frames, averageWall > 0.0 ? 1000.0 / averageWall : 0.0, averageWall,
        cpuWorkMilliseconds / divisor, updateMilliseconds / divisor, spriteSubmissionMilliseconds / divisor,
        particleUpdateMilliseconds / divisor, batchConstructionMilliseconds / divisor,
        commandSubmissionMilliseconds / divisor, presentWaitMilliseconds / divisor,
        frameLimitWaitMilliseconds / divisor, sprites / divisor, particles / divisor, batches / divisor,
        draws / divisor, triangles / divisor, culled / divisor, rssBytes, textures, textureBytes,
        renderTargetBytes, frameResourceBytes, frameStorageGrowths, assetLoadMilliseconds);
}

} // namespace aegis::mediaforge
