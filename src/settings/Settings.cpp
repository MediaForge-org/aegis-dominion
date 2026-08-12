#include "Settings.hpp"

#include <algorithm>

namespace aegis::settings {

void Settings::sanitize() {
    uiScale = std::clamp(uiScale, .75f, 2.f);
    masterVolume = std::clamp(masterVolume, 0.f, 1.f);
    musicVolume = std::clamp(musicVolume, 0.f, 1.f);
    sfxVolume = std::clamp(sfxVolume, 0.f, 1.f);
    fpsLimit = std::clamp(fpsLimit, 30u, 360u);
    screenShake = std::clamp(screenShake, 0.f, 1.f);
}

} // namespace aegis::settings
