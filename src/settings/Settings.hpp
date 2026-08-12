#pragma once

namespace aegis::settings {

struct Settings {
    float uiScale = 1.f;
    float masterVolume = 1.f;
    float musicVolume = .75f;
    float sfxVolume = .9f;
    bool verticalSync = true;
    unsigned fpsLimit = 120;
    float screenShake = 1.f;

    void sanitize();
};

} // namespace aegis::settings
