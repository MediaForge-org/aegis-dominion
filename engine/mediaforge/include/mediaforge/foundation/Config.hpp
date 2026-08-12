#pragma once

#include <string>

namespace mf {

enum class GpuBackendPreference {
    automatic,
    preferVulkan,
    requireVulkan,
};

struct EngineConfig {
    std::string applicationName{"MediaForge Application"};
    GpuBackendPreference gpuBackend{GpuBackendPreference::automatic};
    bool gpuDebug{false};
};

[[nodiscard]] constexpr const char* preferredBackendName(GpuBackendPreference preference) noexcept {
    return preference == GpuBackendPreference::automatic ? nullptr : "vulkan";
}

} // namespace mf
